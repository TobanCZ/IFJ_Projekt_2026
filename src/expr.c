/*
    Implementace překladače imperativního jazyka IFJ26.
*/
#define MAX_STACK_SIZE 1024

#include <stdbool.h>
#include <stddef.h>
#include "expr.h"
#include "token.h"
#include "parser.h"
#include "scanner.h"
#include "symtable.h"
#include "ir_builder.h"
#include "help.h"


//stack
PsaItem psa_stack[MAX_STACK_SIZE];
int stack_top = -1;

void stack_init(void) {
    stack_top = -1;
}

int stack_push(PsaItem item) {
    if (stack_top >= MAX_STACK_SIZE - 1) {
        return INTERNAL_ERROR;
    }
    psa_stack[++stack_top] = item;
    return SEM_OK;
}

bool stack_pop(PsaItem *out) {
    if (stack_top < 0) {
        return false; 
    }
    *out = psa_stack[stack_top--];
    return true;
}

void stack_cleanup(void) {
    while (stack_top >= 0) {
        free(psa_stack[stack_top].addr);
        psa_stack[stack_top].addr = NULL;
        stack_top--;
    }
}

static void free_items(PsaItem *items, int count) {
    for (int i = 0; i < count; i++) {
        free(items[i].addr);
        items[i].addr = NULL;
    }
}

//vrati prvni terminal
bool stack_top_terminal(PsaItem *out) {
    if (stack_top < 0) {
        return false; 
    }

    for(int i = stack_top; i >= 0; i--) {
        if (psa_stack[i].token != PSA_EXPR && psa_stack[i].token != PSA_HANDLE) {
            *out = psa_stack[i];
            return true;
        }
    }
    return false;
}

//pridani zarazky nad prvni terminal
int stack_insert_handle() {
    if (stack_top < 0 || stack_top >= MAX_STACK_SIZE - 1) {
        return INTERNAL_ERROR;
    }

    for(int i = stack_top; i >= 0; i--) {
        if (psa_stack[i].token != PSA_EXPR && psa_stack[i].token != PSA_HANDLE) {
            for(int j = stack_top + 1; j > i + 1; j--) {
                psa_stack[j] = psa_stack[j - 1];
            }
            psa_stack[i + 1].token = PSA_HANDLE;
            psa_stack[i + 1].data_type = -1; 
            psa_stack[i + 1].is_expression = false;
            psa_stack[i + 1].addr = NULL;
            stack_top++;
            return SEM_OK;
        }
    }
    return INTERNAL_ERROR;
}

//mapovani tokenu na PSA tokeny (pro prehlednost)
int to_psa_token(int tokenType, bool expect_operand) {
    switch (tokenType) {
        case TOKEN_OPERATOR_MINUS:
           return expect_operand ? PSA_MINUS_UNAR : PSA_MINUS;
        case TOKEN_OPERATOR_MULTIPLY:
            return PSA_MULTIPLY;
        case TOKEN_OPERATOR_DIVIDE:
            return PSA_DIVIDE;
        case TOKEN_OPERATOR_PLUS:
            return PSA_PLUS;
        case TOKEN_OPERATOR_LESS:
            return PSA_LESS;
        case TOKEN_OPERATOR_LESS_EQUAL:
            return PSA_LESS_EQUAL;
        case TOKEN_OPERATOR_GREATER:
            return PSA_GREATER;
        case TOKEN_OPERATOR_GREATER_EQUAL:
            return PSA_GREATER_EQUAL;
        case TOKEN_OPERATOR_EQUAL:
            return PSA_EQUAL;
        case TOKEN_OPERATOR_NOT_EQUAL:
            return PSA_NOT_EQUAL;
        case TOKEN_PUNCTUATION_LEFT_PAREN:{
            if(peek_token().type == TOKEN_PUNCTUATION_RIGHT_PAREN) {
                return PSA_OPERAND; //unit ()
            }
            return PSA_LEFT_PAREN;
        }
        case TOKEN_PUNCTUATION_RIGHT_PAREN:
            return PSA_RIGHT_PAREN;
        case TOKEN_IDENT:
        case TOKEN_INT:
        case TOKEN_DOUBLE:
        case TOKEN_STRING:
            return PSA_OPERAND;
        default:
            return PSA_DOLLAR;
    }
}

bool is_operator(int psa_token) {
    bool is_operand = false;
    switch (psa_token) {
        case PSA_MULTIPLY:
        case PSA_DIVIDE:
        case PSA_PLUS:
        case PSA_MINUS:
        case PSA_LESS:
        case PSA_LESS_EQUAL:
        case PSA_GREATER:
        case PSA_GREATER_EQUAL:
        case PSA_EQUAL:
        case PSA_NOT_EQUAL:
           is_operand = true;
           break;
        default:
            break;
    }
    return is_operand;
}

bool is_numeric(DataType t) {
    if(t == TYPE_INT || t == TYPE_DOUBLE)
        return true;
    else
        return false;
}

int reduce(int *sem_error) {
    PsaItem items[3];
    int count = 0;
    PsaItem popped;

    //odebere az po zarazku, max 3 protoze neni delsi pravidlo
    while (stack_pop(&popped)) {
        if (popped.token == PSA_HANDLE) {
            break;
        }
        if (count >= 3) {
            return SYNTAX_ERROR;
        }
        items[count++] = popped;
    }

    if (popped.token != PSA_HANDLE || count == 0) {
        return SYNTAX_ERROR;
    }

    DataType result_type = TYPE_UNKNOWN;
    char *result_addr = NULL;

    //pravidla 
    if(count == 1) {
        //operand E
        if(items[0].token != PSA_OPERAND) {
            return SYNTAX_ERROR;
        }
        result_type = items[0].data_type;
        result_addr = items[0].addr;
        items[0].addr = NULL;
    }
    else if(count == 2) {
        //unarni minus -E
        if (items[1].token != PSA_MINUS_UNAR || items[0].token != PSA_EXPR) {
            return SYNTAX_ERROR;
        }
        if(!is_numeric(items[0].data_type)) {
            if (*sem_error == SEM_OK) //aby se neprebila s jiz nalezenou chybou
                *sem_error = SEM_ERROR_TYPE_MISMATCH;
        }
        //prida 0 a odecte
        else if (on_full_pass && *sem_error == SEM_OK) {
            char *tmp = ir_create_tmp();
            const char *zero = (items[0].data_type == TYPE_DOUBLE) ? "float@0.0" : "int@0";
            ir_add(OP_SUB, tmp, zero, items[0].addr);
            result_addr = tmp;
        }
        result_type = items[0].data_type;
    }
    else if(count == 3) 
    {   //vyraz v zavorkach ( E)
       if (items[2].token == PSA_LEFT_PAREN && items[1].token == PSA_EXPR && items[0].token == PSA_RIGHT_PAREN) {
            result_type = items[1].data_type;
            result_addr = items[1].addr;
            items[1].addr = NULL;
        }
        // vyrazi s operatorem E operand E
       else if (items[2].token == PSA_EXPR && is_operator(items[1].token) && items[0].token == PSA_EXPR) {
            DataType left = items[2].data_type;
            DataType right = items[0].data_type;
            int op = items[1].token;

            //semantika
            switch (op) {
                case PSA_PLUS:
                    if (left == TYPE_STRING && right == TYPE_STRING) {
                        result_type = TYPE_STRING;
                    } else if (is_numeric(left) && is_numeric(right)) {
                        if(left == TYPE_DOUBLE || right == TYPE_DOUBLE)
                            result_type = TYPE_DOUBLE;
                        else 
                            result_type = TYPE_INT;
                    } else {
                         if (*sem_error == SEM_OK)
                            *sem_error = SEM_ERROR_TYPE_MISMATCH;
                    }
                    break;
                case PSA_MINUS:
                case PSA_MULTIPLY:
                case PSA_DIVIDE:
                    if (is_numeric(left) && is_numeric(right)) {
                        if(left == TYPE_DOUBLE || right == TYPE_DOUBLE)
                            result_type = TYPE_DOUBLE;
                        else
                            result_type = TYPE_INT;
                    } else {
                        if (*sem_error == SEM_OK)
                            *sem_error = SEM_ERROR_TYPE_MISMATCH;
                    }
                    break;
                case PSA_LESS:
                case PSA_LESS_EQUAL:
                case PSA_GREATER:
                case PSA_GREATER_EQUAL:
                    if ((is_numeric(left) && is_numeric(right)) || (left == TYPE_STRING && right == TYPE_STRING)) {
                        result_type = TYPE_BOOL;
                    } else {
                        if (*sem_error == SEM_OK)
                            *sem_error = SEM_ERROR_TYPE_MISMATCH;
                    }
                    break;
                case PSA_EQUAL:
                case PSA_NOT_EQUAL:
                    if ((is_numeric(left) && is_numeric(right)) || (left == TYPE_STRING && right == TYPE_STRING) || (left == TYPE_UNIT && right == TYPE_UNIT))
                        result_type = TYPE_BOOL;
                    else
                        if (*sem_error == SEM_OK)
                            *sem_error = SEM_ERROR_TYPE_MISMATCH;
                    
                    break;
                default:
                    return SYNTAX_ERROR;
            }

            //generace ir
            if (on_full_pass && *sem_error == SEM_OK) {
                char *left_addr = items[2].addr;
                char *right_addr = items[0].addr;
                items[2].addr = NULL;
                items[0].addr = NULL;

                //konverze int na double
                if (left == TYPE_INT && right == TYPE_DOUBLE) {
                    char *conv = ir_create_tmp();
                    ir_add(OP_INT2FLOAT, conv, left_addr, NULL);
                    free(left_addr);
                    left_addr = conv;
                } else if (left == TYPE_DOUBLE && right == TYPE_INT) {
                    char *conv = ir_create_tmp();
                    ir_add(OP_INT2FLOAT, conv, right_addr, NULL);
                    free(right_addr);
                    right_addr = conv;
                }

                result_addr = ir_create_tmp();

                switch (op) {
                    case PSA_PLUS:
                        ir_add((left == TYPE_STRING) ? OP_CONCAT : OP_ADD, result_addr, left_addr, right_addr);
                        break;
                    case PSA_MINUS:
                        ir_add(OP_SUB, result_addr, left_addr, right_addr);
                        break;
                    case PSA_MULTIPLY:
                        ir_add(OP_MUL, result_addr, left_addr, right_addr);
                        break;
                    case PSA_DIVIDE:
                        ir_add((result_type == TYPE_INT) ? OP_IDIV : OP_DIV, result_addr, left_addr, right_addr);
                        break;
                    case PSA_LESS:
                        ir_add(OP_LT, result_addr, left_addr, right_addr);
                        break;
                    case PSA_GREATER:
                        ir_add(OP_GT, result_addr, left_addr, right_addr);
                        break;
                    case PSA_EQUAL:
                        ir_add(OP_EQ, result_addr, left_addr, right_addr);
                        break;
                    case PSA_NOT_EQUAL: {
                        char *eq_tmp = ir_create_tmp();
                        ir_add(OP_EQ, eq_tmp, left_addr, right_addr);
                        ir_add(OP_NOT, result_addr, eq_tmp, NULL);
                        free(eq_tmp);
                        break;
                    }
                    case PSA_LESS_EQUAL:
                    case PSA_GREATER_EQUAL: {
                        char *cmp_tmp = ir_create_tmp();
                        char *eq_tmp = ir_create_tmp();
                        ir_add((op == PSA_LESS_EQUAL) ? OP_LT : OP_GT, cmp_tmp, left_addr, right_addr);
                        ir_add(OP_EQ, eq_tmp, left_addr, right_addr);
                        ir_add(OP_OR, result_addr, cmp_tmp, eq_tmp);
                        free(cmp_tmp);
                        free(eq_tmp);
                        break;
                    }
                    default:
                        break;
                }

                free(left_addr);
                free(right_addr);
            }
        } 
        else {
            return SYNTAX_ERROR;
        }
    }

    free_items(items, count);
    PsaItem expr_item;
    expr_item.token = PSA_EXPR;
    expr_item.data_type = result_type;
    expr_item.addr = result_addr;
    if (stack_push(expr_item) != SEM_OK)
        return INTERNAL_ERROR;

    return SYNTAX_OK;
}

int parse_expr(DataType *out_type, char **out_addr){
    stack_init();

    PsaItem dollar_item = { .token = PSA_DOLLAR, .data_type = -1 };
    if (stack_push(dollar_item) != SEM_OK) {
        return INTERNAL_ERROR;
    }

    bool expect_operand = true;
    int sem_error = SEM_OK;

    while (true) {
        PsaItem top_item;

        if(!stack_top_terminal(&top_item)) {
            return INTERNAL_ERROR;
        }

        PSA_Token input_psa_token = to_psa_token(current_token.type, expect_operand);

        //konec pokud jsou oba $ a na zasobniku je jen E a $
        if (top_item.token == PSA_DOLLAR && input_psa_token == PSA_DOLLAR) {
            if (current_token.type == TOKEN_OPERATOR_ASSIGN) 
                return SYNTAX_ERROR;
                
            if (stack_top == 1 && psa_stack[1].token == PSA_EXPR) {
                if (sem_error != SEM_OK) {
                    return sem_error;
                }
                //vraci typ
                if (out_type != NULL) {
                    *out_type = psa_stack[1].data_type;
                }
                //vraci adresu
                if (out_addr != NULL) {
                    *out_addr = psa_stack[1].addr;
                    psa_stack[1].addr = NULL;
                } else {
                    free(psa_stack[1].addr);
                    psa_stack[1].addr = NULL;
                }
                return SYNTAX_OK;
            }
            return SYNTAX_ERROR;
        }

        char prec_result = precedent_table[top_item.token][input_psa_token];

        switch (prec_result) {
            case '<': {
                //vlozi zarazku a token
                if (stack_insert_handle() != SEM_OK) {
                    return INTERNAL_ERROR;
                }

                char *addr = NULL;
                DataType type = TYPE_UNKNOWN;

                // E operand
                if (input_psa_token == PSA_OPERAND) {
                    if (current_token.type == TOKEN_IDENT) {
                        Symbol *sym = symtable_stack_find(&stack, current_token.value);
                        if (sym == NULL) {
                            if (sem_error == SEM_OK) 
                                sem_error = SEM_ERROR_UNDEFINED;
                        } 
                        else {
                            type = sym->symbolType;
                            if (on_full_pass && sym->irName != NULL) {
                                addr = copy_str(sym->irName);
                            }
                        }           
                    } else if (current_token.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
                        type = TYPE_UNIT;
                        if (on_full_pass) 
                            addr = copy_str("nil@nil");
                    } else {
                        type = token_type_to_data_type(current_token.type);
                        if (on_full_pass) 
                            addr = ir_from_token_to_operand(current_token);
                    }
                }


                PsaItem item;
                item.data_type = type;
                item.token = input_psa_token;
                item.addr = addr;

                if (stack_push(item) != SEM_OK) {
                    stack_cleanup();
                    return INTERNAL_ERROR;
                }

                // za ) a E nemuze byt unarni minus
                if(input_psa_token == PSA_OPERAND || input_psa_token == PSA_RIGHT_PAREN) {
                    expect_operand = false;
                } else {
                    expect_operand = true;
                }

                //pokud je () uint tak musi jit o jeden token dal
                if (current_token.type == TOKEN_PUNCTUATION_LEFT_PAREN && input_psa_token == PSA_OPERAND) {
                    next_token();
                }

                next_token();
                break;
            }
            case '>': {
                //redukuje zasobnik
                int res = reduce(&sem_error);
                if (res != SYNTAX_OK) {
                    return res;
                }
                break;
            }
            case '=': {
                // konec )
                PsaItem item = { .token = input_psa_token, .data_type = -1 };
                if (stack_push(item) != SEM_OK) {
                    return INTERNAL_ERROR;
                }

                expect_operand = false; // za ) nemuze byt unarni minus
                next_token();
                break;
            }
            default:
            // E - chyba
                return SYNTAX_ERROR;
        }

    }

    return SYNTAX_OK;
}