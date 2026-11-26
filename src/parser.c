/*
    Implementace překladače imperativního jazyka IFJ26.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "symtable.h"
#include "scanner.h"
#include "expr.h"
#include "help.h"
#include "ir_builder.h"
#include "ir.h"

#define MAX_PARAM_COUNT 1024

int add_builtins_functions(Symtable *table);

int parse_program(ParsePass pass);
int parse_func_def(ParsePass pass);

int parse_expr_or_call(DataType *out_type, char **out_addr);
int parse_block(DataType *out_type, char **out_addr);

int parse_statement(DataType *out_type, char **out_addr);

int parse_def();
int parse_assign();
int parse_call();
int parse_return();
int parse_if();
int parse_while();
int parse_expr();

static Symtable table; //funkce
SymtableStack stack;  //promenne a parametry

Token current_token;
 bool on_full_pass;
bool has_main = false;
int current_scope = 0;
Symbol *current_function = NULL;
char *main_func_name = NULL;

//Pro moznost 2 pruchodu 
Token* tokens = NULL;
static int token_count = 0;
static int token_index = 0;

void free_all_tokens(void) {
    for (int i = 0; i < token_count; i++) {
        free(tokens[i].value);
    }
    free(tokens);
    tokens = NULL;
    token_count = 0;
    token_index = 0;
    current_token.value = NULL;
}

int load_all_tokens(){
    //pomoci dynamickeho pole
    int capacity = 128;
    tokens = malloc(capacity * sizeof(Token));
    if (tokens == NULL) 
        return INTERNAL_ERROR;
    while(true) {
        if(token_count >= capacity) {
            capacity *= 2;
            Token *new_tokens = realloc(tokens, capacity * sizeof(Token));
            if (new_tokens == NULL) {
                free_all_tokens();
                return INTERNAL_ERROR;
            }
            tokens = new_tokens;
        }
        Token current = get_token();
        tokens[token_count++] = current;
        if (current.type == TOKEN_EOF) 
            break;
        if (current.type == TOKEN_LEX_ERROR) {
            free_all_tokens();
            return LEX_ERROR;
        }
        if(current.type == TOKEN_INTERNAL_ERROR) {
            free_all_tokens();
            return INTERNAL_ERROR;
        }
    }
    token_index = 0;
    return OK;
}

void next_token() {
    if (token_index < token_count) {
        current_token = tokens[token_index++];
    } else {
        current_token = tokens[token_count - 1]; 
    }
}

Token peek_token(void) {
    if (token_index < token_count) {
        return tokens[token_index];
    }
    return tokens[token_count - 1];
}

int expect_token(TokenType expected_token) {
    if (current_token.type != expected_token) {
        return SYNTAX_ERROR;
    }
    next_token();
    return SYNTAX_OK;
}

int expect_token_and_attr(TokenType expected_token, char* expected_attr) {
    if (current_token.type != expected_token || current_token.value == NULL || strcmp(current_token.value, expected_attr) != 0) {
        return SYNTAX_ERROR;
    }
    next_token();
    return SYNTAX_OK;
}

int token_type_to_data_type(TokenType token_type) {
    switch (token_type) {
        case TOKEN_KW_INT:
        case TOKEN_INT:
            return TYPE_INT;
        case TOKEN_KW_DOUBLE:
        case TOKEN_DOUBLE:
            return TYPE_DOUBLE;
        case TOKEN_KW_STRING:
        case TOKEN_STRING:
            return TYPE_STRING;
        case TOKEN_KW_UNIT:
        case TOKEN_PUNCTUATION_LEFT_PAREN:
            return TYPE_UNIT;
        default:
            return TYPE_UNKNOWN;
    }
}


int parse() {
    has_main = false;
    on_full_pass = false;
    current_scope = 0;
    ir_builder_init();
    scanner_reset();
    symtable_init(&table);
    symtable_stack_init(&stack);
    if (add_builtins_functions(&table) != OK) {
        symtable_destroy(&table);
        symtable_stack_destroy(&stack);
        return INTERNAL_ERROR;
    }

    int res = load_all_tokens();
    if (res != OK) {
        symtable_destroy(&table);
        symtable_stack_destroy(&stack);
        return res;
    }
    //prvni
    token_index = 0;
    has_main = false;
    res = parse_program(PASS_HEADERS);
    if(res != SYNTAX_OK) {
        free_all_tokens();
        symtable_destroy(&table);
        symtable_stack_destroy(&stack);
        free(main_func_name);
        main_func_name = NULL;
        return res;
    }

    //druhy pruchod
    token_index = 0;
    has_main = false;
    res = parse_program(PASS_INFER);
    if(res != SYNTAX_OK) {
        free_all_tokens();
        symtable_destroy(&table);
        symtable_stack_destroy(&stack);
        free(main_func_name);
        main_func_name = NULL;
        return res;
    }

    //treti pruchod
    token_index = 0;
    has_main = false;
    on_full_pass = true;
    res = parse_program(PASS_FULL);
    if(res != SYNTAX_OK) {
        free_all_tokens();
        symtable_destroy(&table);
        symtable_stack_destroy(&stack);
        free(main_func_name);
        main_func_name = NULL;
        return res;
    }

    IRNode *head = get_ir();
    optimizer(head);
    generate_target_code(head);
    free_ir_list(head);

    free_all_tokens();
    symtable_destroy(&table);
    symtable_stack_destroy(&stack);
    free(main_func_name);
    main_func_name = NULL;

    return res;
}

//pridani vestavenych funkci TODO: mozna to dat do funkce protoze se toho dost opakuje
int add_builtins_functions(Symtable *table) {
    Symbol function = {0};

    function.name = "print";
    function.symbolType = TYPE_UNIT;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = malloc(sizeof(DataType));
    if(function.parameterTypes == NULL) {
        return INTERNAL_ERROR;
    }
    function.parameterTypes[0] = TYPE_ANY;
    function.paramCount = 1;
    function.isMutable = false;
    symtable_insert(table, "print", function);
    free(function.parameterTypes);

    function.name = "readLine";
    function.symbolType = TYPE_STRING;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = NULL;
    function.paramCount = 0;
    function.isMutable = false;
    symtable_insert(table, "readLine", function);

    function.name = "readInt";
    function.symbolType = TYPE_INT;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = NULL;
    function.paramCount = 0;
    function.isMutable = false;
    symtable_insert(table, "readInt", function);

    function.name = "readDouble";
    function.symbolType = TYPE_DOUBLE;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = NULL;
    function.paramCount = 0;
    function.isMutable = false;
    symtable_insert(table, "readDouble", function);

    function.name = "toStr";
    function.symbolType = TYPE_STRING;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = malloc(sizeof(DataType));
    if(function.parameterTypes == NULL) {
        return INTERNAL_ERROR;
    }
    function.parameterTypes[0] = TYPE_ANY;
    function.paramCount = 1;
    function.isMutable = false;
    symtable_insert(table, "toStr", function);
    free(function.parameterTypes);

    function.name = "toInt";
    function.symbolType = TYPE_INT;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = malloc(sizeof(DataType));
    if(function.parameterTypes == NULL) {
        return INTERNAL_ERROR;
    }
    function.parameterTypes[0] = TYPE_DOUBLE;
    function.paramCount = 1;
    function.isMutable = false;
    symtable_insert(table, "toInt", function);
    free(function.parameterTypes);

    function.name = "strLength";
    function.symbolType = TYPE_INT;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = malloc(sizeof(DataType));
    if(function.parameterTypes == NULL) {
        return INTERNAL_ERROR;
    }
    function.parameterTypes[0] = TYPE_STRING;
    function.paramCount = 1;
    function.isMutable = false;
    symtable_insert(table, "strLength", function);
    free(function.parameterTypes);

    function.name = "subStr";
    function.symbolType = TYPE_STRING;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = malloc(sizeof(DataType)*3);
    if(function.parameterTypes == NULL) {
        return INTERNAL_ERROR;
    }
    function.parameterTypes[0] = TYPE_STRING;
    function.parameterTypes[1] = TYPE_INT;
    function.parameterTypes[2] = TYPE_INT;
    function.paramCount = 3;
    function.isMutable = false;
    symtable_insert(table, "subStr", function);
    free(function.parameterTypes);

    function.name = "ord";
    function.symbolType = TYPE_INT;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = malloc(sizeof(DataType)*2);
    if(function.parameterTypes == NULL) {
        return INTERNAL_ERROR;
    }
    function.parameterTypes[0] = TYPE_STRING;
    function.parameterTypes[1] = TYPE_INT;
    function.paramCount = 2;
    function.isMutable = false;
    symtable_insert(table, "ord", function);
    free(function.parameterTypes);

    function.name = "chr";
    function.symbolType = TYPE_STRING;
    function.scopeLevel = 0;
    function.isFunction = true;
    function.parameterTypes = malloc(sizeof(DataType));
    if(function.parameterTypes == NULL) {
        return INTERNAL_ERROR;
    }
    function.parameterTypes[0] = TYPE_INT;
    function.paramCount = 1;
    function.isMutable = false;
    symtable_insert(table, "chr", function);
    free(function.parameterTypes);

    return OK;
}

int parse_program(ParsePass pass){
    // Kontrola importu ifj26 na zacatku programu
    next_token();
    if (expect_token(TOKEN_KW_IMPORT) != SYNTAX_OK) 
        return SYNTAX_ERROR;

    if (expect_token_and_attr(TOKEN_IDENT, "ifj26") != SYNTAX_OK) 
        return SYNTAX_ERROR;

    if (expect_token(TOKEN_PUNCTUATION_DOT) != SYNTAX_OK) 
        return SYNTAX_ERROR;

    if (expect_token(TOKEN_OPERATOR_MULTIPLY) != SYNTAX_OK)
        return SYNTAX_ERROR;

    if (expect_token(TOKEN_EOL) != SYNTAX_OK)
        return SYNTAX_ERROR;

    if (pass == PASS_FULL && main_func_name != NULL) {
        char main_label[128];
        snprintf(main_label, sizeof(main_label), "$%s", main_func_name);
        ir_add(OP_CREATEFRAME, NULL, NULL, NULL);
        ir_add(OP_CALL, main_label, NULL, NULL);
        ir_add(OP_EXIT, "int@0", NULL, NULL);
    }

    //Do konce souboru
    while(current_token.type != TOKEN_EOF) {
        if (current_token.type == TOKEN_EOL) {
            next_token();
            continue;
        }
        int res = parse_func_def(pass);
        if (res != SYNTAX_OK)
            return res;
    }

    if (!has_main) 
        return SEM_ERROR_UNDEFINED; // chyba 3

    return SYNTAX_OK;
}

int parse_func_params(DataType *parameterTypes, int *param_counter) {
    while(current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN && current_token.type != TOKEN_EOF) {
        //Nazev parametru
        if (current_token.type != TOKEN_IDENT) 
            return SYNTAX_ERROR;
        
        // kontrola redefinice parametru
        if(symtable_stack_find_current(&stack, current_token.value) != NULL) 
            return SEM_ERROR_REDEFINITION;
        
        char *param_name = copy_str(current_token.value);
        if (!param_name) return INTERNAL_ERROR;
        next_token();
        int res = SYNTAX_OK;
        //:
        if (expect_token(TOKEN_PUNCTUATION_COLON) != SYNTAX_OK) 
            res = SYNTAX_ERROR;
        
        //Typ parametru
        if(res == SYNTAX_OK && current_token.type != TOKEN_KW_INT && current_token.type != TOKEN_KW_DOUBLE && current_token.type != TOKEN_KW_STRING && current_token.type != TOKEN_KW_UNIT) 
            res = SYNTAX_ERROR;
        
        if(res != SYNTAX_OK) {
            free(param_name);
            return res;
        }

        if (*param_counter >= MAX_PARAM_COUNT) {
            free(param_name);
            return INTERNAL_ERROR;
        }

        //ulozeni parametru do tabulek
        parameterTypes[*param_counter] = token_type_to_data_type(current_token.type);
        (*param_counter)++;
        
        //vygenerovani ir nazvu parametru
        char ir_buf[32];
        snprintf(ir_buf, sizeof(ir_buf), "LF@%%%d", *param_counter - 1);

        Symbol param = {0};
        param.name = param_name;
        param.symbolType = token_type_to_data_type(current_token.type);
        param.scopeLevel = current_scope;
        param.isFunction = false;
        param.isMutable = false;
        param.paramCount = 0;
        param.parameterTypes = NULL;
        param.irName = ir_buf;
        symtable_stack_insert(&stack, param.name, param);
        free(param_name);
        next_token();

        //bud , mezi parametry a nebo musi byt zavorka
        if(current_token.type == TOKEN_PUNCTUATION_COMMA) {
            next_token();
            // za , musi byt identifikator
            if (current_token.type == TOKEN_PUNCTUATION_RIGHT_PAREN) 
                return SYNTAX_ERROR;
            
        } else if(current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN) 
            return SYNTAX_ERROR;
        
    }
    return SYNTAX_OK;
}

int parse_func_body(){
    DataType ret_type = TYPE_UNKNOWN;
    char *addr = NULL;

    //blok nebo jednoratkovej vyraz;
    if (current_token.type == TOKEN_EOL) {
        next_token();
        int res = parse_block(&ret_type, &addr);
        if (res != SYNTAX_OK) {
            free(addr);
            return res;
        }
    } 
    else {
        int res = parse_expr_or_call(&ret_type, &addr);
        if (res != SYNTAX_OK){
            free(addr);
            return res;
        }

        if (current_token.type == TOKEN_EOL) {
            next_token();
        } 
        else if (current_token.type != TOKEN_EOF) {
            free(addr);
            return SYNTAX_ERROR;
        }
    }

    //ulozeni navratu vyrazu (u Unit funkce se vysledek zahodi a vrati se nil@nil)
    if (on_full_pass && current_function != NULL) {
        if (current_function->symbolType == TYPE_UNIT) {
            ir_add(OP_MOVE, "LF@%retval", "nil@nil", NULL);
        } else if (addr != NULL) {
            if (current_function->symbolType == TYPE_DOUBLE && ret_type == TYPE_INT) {
                ir_add(OP_INT2FLOAT, "LF@%retval", addr, NULL);
            } else {
                ir_add(OP_MOVE, "LF@%retval", addr, NULL);
            }
        }
    }
    free(addr);


    if(ret_type == TYPE_BOOL) 
        return SEM_ERROR_TYPE_MISMATCH;

    //kontrola navrat typu
    if (current_function != NULL) {
        if (current_function->symbolType == TYPE_UNKNOWN || current_function->symbolType == TYPE_ANY) {
            if (ret_type == TYPE_UNKNOWN) {
                return SEM_ERROR_TYPE_MISMATCH;
            }
            current_function->symbolType = ret_type;
        } 
        else if (current_function->symbolType != TYPE_UNIT && ret_type != TYPE_NEVER) {
            if (ret_type == TYPE_UNKNOWN) {
                return SEM_ERROR_TYPE_MISMATCH;
            }
            DataType expected = current_function->symbolType;
            bool valid = (expected == ret_type) || (expected == TYPE_DOUBLE && ret_type == TYPE_INT);
            if (!valid) {
                return SEM_ERROR_TYPE_MISMATCH;
            }
        }
    }

    return SYNTAX_OK;
}

//preskoci parsofani body
int skip_func_body(){
    if (current_token.type == TOKEN_EOL) {
        next_token();
        if (current_token.type != TOKEN_BLOCK_START) {
            return SYNTAX_ERROR;
        }
        //projede vsechny bloky a preskoci je
        int depth = 0;
        do {
            if (current_token.type == TOKEN_BLOCK_START) depth++;
            else if (current_token.type == TOKEN_BLOCK_END) depth--;
            next_token();
        } while (depth > 0 && current_token.type != TOKEN_EOF);
        if(depth != 0) 
            return SYNTAX_ERROR;
        else
            return SYNTAX_OK;
    }

    //jednoradkove 
    while (current_token.type != TOKEN_EOL && current_token.type != TOKEN_EOF) {
        next_token();
    }
    if (current_token.type == TOKEN_EOL) {
        next_token();
    }
    return SYNTAX_OK;
}

int parse_func_def(ParsePass pass) {
    //Kontrola main funkce
    bool is_main = false;
    if (current_token.type == TOKEN_PUNCTUATION_AT) {
        next_token();
        if(expect_token_and_attr(TOKEN_IDENT, "main") != SYNTAX_OK) 
            return SYNTAX_ERROR;
        
        if (has_main) return SEM_ERROR_REDEFINITION;
        has_main = true;
        is_main = true;

        if (current_token.type == TOKEN_EOL) {
            next_token();
        }
    }

    //Zacatek funkce
    if (expect_token(TOKEN_KW_DEF) != SYNTAX_OK)  
        return SYNTAX_ERROR;
    //jmeno funkce
    if (current_token.type != TOKEN_IDENT) return 
        SYNTAX_ERROR;

    //duplikat jmena
    if (pass == PASS_HEADERS && symtable_find(&table, current_token.value) != NULL) 
        return SEM_ERROR_REDEFINITION;
    char *func_name = copy_str(current_token.value);
    if (!func_name) return INTERNAL_ERROR;
    next_token();

    //(
    if(expect_token(TOKEN_PUNCTUATION_LEFT_PAREN) != SYNTAX_OK) { 
        free(func_name);
        return SYNTAX_ERROR;
    }

    //vlozeni parametru
    symtable_stack_push(&stack);
    current_scope++;
    DataType param_types[MAX_PARAM_COUNT];
    int param_count = 0;
    int res = SYNTAX_OK; //kvuli uvolnovani at se nemusi v kazdem ifu uvolnovat

    //Parametry funkce
    if(!is_main) {
        res =parse_func_params(param_types, &param_count);
    }

    //)
    if(res == SYNTAX_OK && expect_token(TOKEN_PUNCTUATION_RIGHT_PAREN) != SYNTAX_OK) {
        if(is_main)
            res = SEM_ERROR_INVALID_ARGUMENT_COUNT;
        else
            res = SYNTAX_ERROR;
    }
    

    //navratovy typ
    int ret_type = -1;
    if (current_token.type == TOKEN_PUNCTUATION_COLON) {
        next_token();
        if (res == SYNTAX_OK && current_token.type != TOKEN_KW_INT && current_token.type != TOKEN_KW_DOUBLE &&
            current_token.type != TOKEN_KW_STRING && current_token.type != TOKEN_KW_UNIT) {
            res = SYNTAX_ERROR;
        }
        ret_type = current_token.type;
        next_token();
    }
    if (res == SYNTAX_OK && is_main && ret_type != TOKEN_KW_UNIT) 
        res = SEM_ERROR_TYPE_MISMATCH;
    
    
    
    // =
    if(res == SYNTAX_OK && expect_token(TOKEN_OPERATOR_ASSIGN) != SYNTAX_OK) 
        res = SYNTAX_ERROR;
    
    //ulozeni funkce, volani tela
    if (res == SYNTAX_OK) {
        //prvnvi pruchod
        if (pass == PASS_HEADERS) {
            Symbol function = {0};
            function.name = func_name;
            function.symbolType = token_type_to_data_type(ret_type);
            function.scopeLevel = 0;
            function.isFunction = true;
            function.paramCount = param_count;
            function.isMutable = false;
            function.parameterTypes = (param_count > 0) ? param_types : NULL;

            symtable_insert(&table, function.name, function);
            res = skip_func_body();

            if (is_main) {
                free(main_func_name);
                main_func_name = copy_str(func_name);
            }
        }//druhy pruchod
        else if (pass == PASS_INFER) {
            if (ret_type == -1) {
                current_function = symtable_find(&table, func_name);
                res = parse_func_body();
                current_function = NULL;
            } else {
                res = skip_func_body();
            }
        } 
        //treti pruchod
        else {
            char func_label[128];
            char func_end_label[128];
            snprintf(func_label, sizeof(func_label), "$%s", func_name);
            snprintf(func_end_label, sizeof(func_end_label), "$%s_end", func_name);

            ir_add(OP_LABEL, func_label, NULL, NULL);
            ir_add(OP_PUSHFRAME, NULL, NULL, NULL);
            ir_add(OP_DEFVAR, "LF@%retval", NULL, NULL);
            ir_add(OP_MOVE, "LF@%retval", "nil@nil", NULL);
            ir_set_func_anchor(); // vsechny DEFVAR v tele funkce se vlozi sem!

            current_function = symtable_find(&table, func_name);
            res = parse_func_body();
            current_function = NULL;

            ir_add(OP_LABEL, func_end_label, NULL, NULL);
            ir_add(OP_POPFRAME, NULL, NULL, NULL);
            ir_add(OP_RETURN, NULL, NULL, NULL);
        }
    }
    free(func_name); 
    symtable_stack_pop(&stack);
    current_scope--;
    return res;
}

int parse_block(DataType *out_type, char **out_addr) {
    //Zacatek bloku
    if (expect_token(TOKEN_BLOCK_START) != SYNTAX_OK) 
        return SYNTAX_ERROR;

    // Blok nesmi byt prazdny
    if (current_token.type == TOKEN_BLOCK_END || current_token.type == TOKEN_EOF)
        return SYNTAX_ERROR;


    //navratova hodnota pro posledni prikaz v bloku
    DataType last_type = TYPE_UNIT;
    char *last_addr = NULL;
    bool has_statement = false;
    //Do konce bloku nebo souboru
    while (current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        if (current_token.type == TOKEN_EOL) {
            next_token();
            continue;
        }
        has_statement = true;
        free(last_addr);
        last_addr = NULL;
        int res = parse_statement(&last_type, (out_addr != NULL) ? &last_addr : NULL);
        if (res != SYNTAX_OK) {
            free(last_addr);
            return res;
        }
    }

    if (!has_statement) {
        free(last_addr);
        return SYNTAX_ERROR;
    }

    //konec bloku
    if (expect_token(TOKEN_BLOCK_END) != SYNTAX_OK) {
        free(last_addr);
        return SYNTAX_ERROR;
    }

    if (out_type != NULL) {
        *out_type = last_type;
    }
    if (out_addr != NULL) {
        *out_addr = last_addr;
    } else {
        free(last_addr);
    }
    return SYNTAX_OK;
}

int parse_expr_or_call(DataType *out_type, char **out_addr) {
    if (current_token.type == TOKEN_IDENT) {
        Token next = peek_token();
        if (next.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            return parse_call(out_type, out_addr);
        }
    }
    return parse_expr(out_type, out_addr);
}

int parse_statement(DataType *out_type, char **out_addr) {
    DataType statement_type = TYPE_UNIT;
    if (out_addr != NULL) {
        *out_addr = NULL;
    }

    //Blokove prikazy
    if (current_token.type == TOKEN_KW_IF) {
        int res = parse_if(&statement_type);
        if (res == SYNTAX_OK && out_type != NULL) {
            *out_type = statement_type;
        }
        return res;
    }

    if (current_token.type == TOKEN_KW_WHILE) {
        int res = parse_while();
        if (res == SYNTAX_OK && out_type != NULL) {
            *out_type = TYPE_UNIT; // while vzdu uint
        }
        return res;
    }

    //Jednoradkove
    int res = SYNTAX_ERROR;

    if (current_token.type == TOKEN_KW_VAL || current_token.type == TOKEN_KW_VAR) {
        res = parse_def();
        statement_type = TYPE_UNIT;
    } 
    else if (current_token.type == TOKEN_KW_RETURN) {
        res = parse_return();
        statement_type = TYPE_NEVER;
    } 
    else if (current_token.type == TOKEN_IDENT) {
        // prirazeni nebo funkce
        Token next = peek_token();
        if (next.type == TOKEN_OPERATOR_ASSIGN) {
            res = parse_assign();
            statement_type = TYPE_UNIT;
        } 
        else if (next.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            res = parse_call(&statement_type, out_addr);
        } 
        else {
            res = parse_expr(&statement_type, out_addr);
        }
    } 
    else if (current_token.type == TOKEN_PUNCTUATION_LEFT_PAREN || current_token.type == TOKEN_OPERATOR_MINUS || current_token.type == TOKEN_INT || current_token.type == TOKEN_DOUBLE || current_token.type == TOKEN_STRING ||current_token.type == TOKEN_KW_UNIT) {
        res = parse_expr(&statement_type, out_addr);
    } 
    else {
        return SYNTAX_ERROR;
    }

    if (res != SYNTAX_OK) {
        if (out_addr != NULL) {
            free(*out_addr);
            *out_addr = NULL;
        }
        return res;
    }

    //Oddelovac za jednoradkovym prikazem
    if (current_token.type == TOKEN_EOL || current_token.type == TOKEN_PUNCTUATION_SEMICOLON) {
        next_token();
    } else if (current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        if (out_addr != NULL) {
            free(*out_addr);
            *out_addr = NULL;
        }
        return SYNTAX_ERROR;
    }

    if (out_type != NULL) {
        *out_type = statement_type;
    }

    return SYNTAX_OK;
}

int parse_def() {
    bool is_mutable = (current_token.type == TOKEN_KW_VAR);
    //var nebo val
    if(current_token.type != TOKEN_KW_VAL && current_token.type != TOKEN_KW_VAR) {
        return SYNTAX_ERROR;
    }
    next_token();

    //jmeno
    if(current_token.type != TOKEN_IDENT)
        return SYNTAX_ERROR;
    
    //kontrola duplicity promenne
    if (symtable_stack_find_current(&stack, current_token.value) != NULL) 
        return SEM_ERROR_REDEFINITION;
    char *name = copy_str(current_token.value);
    if (!name) return INTERNAL_ERROR;
    next_token();

    int data_type = TYPE_ANY;
    //typ
    if(current_token.type == TOKEN_PUNCTUATION_COLON) {
        next_token();
        if(current_token.type != TOKEN_KW_INT && current_token.type != TOKEN_KW_DOUBLE && current_token.type != TOKEN_KW_STRING && current_token.type != TOKEN_KW_UNIT) {
            free(name);
            return SYNTAX_ERROR;
        }
        data_type = token_type_to_data_type(current_token.type);
        next_token();
    }

    //=
    if(expect_token(TOKEN_OPERATOR_ASSIGN) != SYNTAX_OK){
        free(name);
        return SYNTAX_ERROR;
    }

    DataType expr_type = TYPE_UNKNOWN;
    char *expr_addr = NULL; 
    int res = parse_expr_or_call(&expr_type, &expr_addr);

    if(res == SYNTAX_OK) {
        //Nemuze byt vyraz
        if (expr_type == TYPE_BOOL) {
            free(name);
            return SEM_ERROR_TYPE_MISMATCH;
        }

        if (data_type == TYPE_ANY) {
            if (expr_type != TYPE_UNKNOWN) {
                data_type = expr_type;
            }
        } else if (expr_type != TYPE_UNKNOWN) {;
            bool valid = (data_type == (int)expr_type) || (data_type == TYPE_DOUBLE && expr_type == TYPE_INT);
            if (!valid) {
                free(name);
                return SEM_ERROR_TYPE_MISMATCH;
            }
        }

        Symbol symbol = {0};
        symbol.name = name;
        symbol.symbolType = data_type;
        symbol.scopeLevel = current_scope;
        symbol.isFunction = false;
        symbol.isMutable = is_mutable;
        symbol.paramCount = 0;
        symbol.parameterTypes = NULL;

        char *var_ir_name = NULL;
        if (on_full_pass) {
            var_ir_name = ir_create_local_var(name);
            symbol.irName = var_ir_name;

            if (expr_addr != NULL) {
                if (data_type == TYPE_DOUBLE && expr_type == TYPE_INT) {
                    ir_add(OP_INT2FLOAT, var_ir_name, expr_addr, NULL);
                } else {
                    ir_add(OP_MOVE, var_ir_name, expr_addr, NULL);
                }
            }
        }

        symtable_stack_insert(&stack, symbol.name, symbol);
        free(var_ir_name);
    }
   
    free(expr_addr);
    free(name);
    return res;
}

int parse_assign() {
    //jmeno promenne

    if(current_token.type != TOKEN_IDENT) 
        return SYNTAX_ERROR;
    Symbol *symbol = symtable_stack_find(&stack, current_token.value);
    if (symbol == NULL) {
        symbol = symtable_find(&table, current_token.value);
    }

    next_token();

    //=
    if(expect_token(TOKEN_OPERATOR_ASSIGN) != SYNTAX_OK)
        return SYNTAX_ERROR;

    DataType expr_type = TYPE_UNKNOWN;
    char *expr_addr = NULL;
    int res = parse_expr_or_call(&expr_type, &expr_addr);

    if(res != SYNTAX_OK) {
        free(expr_addr);
        return res;
    }

    if (symbol == NULL) {
        free(expr_addr);
        return SEM_ERROR_UNDEFINED;
    }
    if(!symbol->isMutable) {
        free(expr_addr);
        return SEM_ERROR_INVALID_WRITE;
    }
    //Nemuze byt vyraz
    if (expr_type == TYPE_BOOL) 
    {
        free(expr_addr);
        return SEM_ERROR_TYPE_MISMATCH;
    }

    if (symbol->symbolType == TYPE_ANY) {
        if (expr_type != TYPE_UNKNOWN) {
            symbol->symbolType = expr_type;
        }
    } 
    else if (expr_type != TYPE_UNKNOWN) {
        bool valid = (symbol->symbolType == expr_type) || (symbol->symbolType == TYPE_DOUBLE && expr_type == TYPE_INT);
        if (!valid){
            free(expr_addr);
            return SEM_ERROR_TYPE_MISMATCH;
        }
    }

    //pokud cislo, tak pretypovat, jinak jen presunout hodnotu
    if (on_full_pass && symbol->irName != NULL && expr_addr != NULL) {
    if (symbol->symbolType == TYPE_DOUBLE && expr_type == TYPE_INT) {
        ir_add(OP_INT2FLOAT, symbol->irName, expr_addr, NULL);
    } 
    else {
        ir_add(OP_MOVE, symbol->irName, expr_addr, NULL);
    }
    }

    free(expr_addr);
    return res;
}

int parse_call(DataType *out_type, char **out_addr) {
    //jmeno
    if(current_token.type != TOKEN_IDENT)
        return SYNTAX_ERROR;
    //kontrola existence
    Symbol* function = symtable_find(&table, current_token.value);

    next_token();
    //(
    if(expect_token(TOKEN_PUNCTUATION_LEFT_PAREN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }
    int param_count = 0;
    DataType param_types[MAX_PARAM_COUNT];
    char *arg_addrs[MAX_PARAM_COUNT] = {0};
    bool undefined_param = false; //syntax chyby maji prednost

    //parametry
    while (current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN && current_token.type != TOKEN_EOF) {
        //vyraz nebo volani
        if (current_token.type == TOKEN_IDENT) {
            Symbol *arg = symtable_stack_find(&stack, current_token.value);
            if (arg == NULL) {
                undefined_param = true;
                arg_addrs[param_count] = NULL;
                param_types[param_count++] = TYPE_UNKNOWN;
            } else {
                arg_addrs[param_count] = (arg && arg->irName) ? copy_str(arg->irName) : NULL;
                param_types[param_count++] = arg->symbolType;
            }
            next_token();
        } 
        else if (current_token.type == TOKEN_INT) {
            arg_addrs[param_count] = ir_from_token_to_operand(current_token);
            param_types[param_count++] = TYPE_INT;
            next_token();
        } 
        else if (current_token.type == TOKEN_DOUBLE) {
            arg_addrs[param_count] = ir_from_token_to_operand(current_token);
            param_types[param_count++] = TYPE_DOUBLE;
            next_token();
        } 
        else if (current_token.type == TOKEN_STRING) {
            arg_addrs[param_count] = ir_from_token_to_operand(current_token);
            param_types[param_count++] = TYPE_STRING;
            next_token();
        } 
        else if (current_token.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            next_token();
            if (expect_token(TOKEN_PUNCTUATION_RIGHT_PAREN) != SYNTAX_OK) 
                return SYNTAX_ERROR;
            arg_addrs[param_count] = copy_str("nil@nil");
            param_types[param_count++] = TYPE_UNIT;
        } 
        else {
            return SYNTAX_ERROR;
        }

        if (current_token.type == TOKEN_PUNCTUATION_COMMA) {
            next_token();
            //osetreni odradkovani mezi parametry
            if (current_token.type == TOKEN_EOL) {
                next_token();
                if (current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN) return SYNTAX_ERROR;
            } 
            else if (current_token.type == TOKEN_PUNCTUATION_RIGHT_PAREN) {
                return SYNTAX_ERROR;
            }
        } 
        else if (current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN) {
            return SYNTAX_ERROR;
        }
    }

    //)
    if(expect_token(TOKEN_PUNCTUATION_RIGHT_PAREN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    if (current_token.type != TOKEN_EOL && current_token.type != TOKEN_PUNCTUATION_SEMICOLON && current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        return SYNTAX_ERROR;
    }

    //rekurze bez typu
    if (function != NULL && function->symbolType == TYPE_UNKNOWN) {
        return SEM_ERROR_TYPE_MISMATCH;
    }

    if(function == NULL || undefined_param) 
        return SEM_ERROR_UNDEFINED;
    if(param_count != function->paramCount) 
        return SEM_ERROR_INVALID_ARGUMENT_COUNT;
    

    for (int i = 0; i < param_count; i++) {
        DataType expected = function->parameterTypes[i];
        DataType actual = param_types[i];

        bool valid = false;
        if (expected == TYPE_ANY || actual == TYPE_ANY)
            valid = true;
        if (expected == actual)
            valid = true;
        if (expected == TYPE_DOUBLE && actual == TYPE_INT)
            valid = true;

        if (!valid) 
            return SEM_ERROR_INVALID_ARGUMENT_COUNT;
    }
    if (out_type != NULL) {
        *out_type = function->symbolType;
    }

    if (on_full_pass) {
        ir_add(OP_CREATEFRAME, NULL, NULL, NULL);

        for (int i = 0; i < param_count; i++) {
            char tf_param[32];
            snprintf(tf_param, sizeof(tf_param), "TF@%%%d", i);
            ir_add(OP_DEFVAR, tf_param, NULL, NULL);

            if (function->parameterTypes[i] == TYPE_DOUBLE && param_types[i] == TYPE_INT) {
                ir_add(OP_INT2FLOAT, tf_param, arg_addrs[i], NULL);
            } else {
                ir_add(OP_MOVE, tf_param, arg_addrs[i], NULL);
            }
        }

        char call_label[128];
        // Pozor: v codegen.c mas LABEL $substr s malym 's'
        if (strcmp(function->name, "subStr") == 0) {
            snprintf(call_label, sizeof(call_label), "$substr");
        } else {
            snprintf(call_label, sizeof(call_label), "$%s", function->name);
        }
        ir_add(OP_CALL, call_label, NULL, NULL);

        if (out_addr != NULL) {
            char *res_tmp = ir_create_tmp();
            ir_add(OP_MOVE, res_tmp, "TF@%retval", NULL);
            *out_addr = res_tmp;
        }
    }

    for (int i = 0; i < param_count; i++) {
        free(arg_addrs[i]);
    }
    return SYNTAX_OK;
}

int parse_return() {
    // return
    if (expect_token(TOKEN_KW_RETURN) != SYNTAX_OK)
        return SYNTAX_ERROR;    

    DataType ret_type = TYPE_UNIT;
    char *return_addr = NULL;

    if (current_token.type != TOKEN_EOL &&
        current_token.type != TOKEN_PUNCTUATION_SEMICOLON &&
        current_token.type != TOKEN_BLOCK_END &&
        current_token.type != TOKEN_EOF) {
        int res = parse_expr_or_call(&ret_type, &return_addr);
        if (res != SYNTAX_OK) {
            free(return_addr);
            return res;
        }
    }

    //funkce bez typu nemuze mit return
    if (current_function != NULL && current_function->symbolType == TYPE_UNKNOWN) {
        free(return_addr);
        return SEM_ERROR_TYPE_MISMATCH;
    }

    //bool nejde vratit
    if (ret_type == TYPE_BOOL) {
        free(return_addr);
        return SEM_ERROR_TYPE_MISMATCH;
    }

    if (ret_type == TYPE_UNIT && current_function != NULL && current_function->symbolType != TYPE_UNIT) {
        free(return_addr);
        return SEM_ERROR_OTHER;
    }

    //navratovy typ funkce
    if (current_function != NULL && current_function->symbolType != TYPE_UNKNOWN &&
        current_function->symbolType != TYPE_UNIT && ret_type != TYPE_UNKNOWN) {
        DataType expected = current_function->symbolType;
        bool valid = (expected == ret_type) || (expected == TYPE_DOUBLE && ret_type == TYPE_INT);
        if (!valid) {
            free(return_addr);
            return SEM_ERROR_TYPE_MISMATCH;
        }
    }

    //prirazeni navratove hodnoty a skok na konec funkce
    if (on_full_pass && current_function != NULL) {
        if (current_function->symbolType == TYPE_UNIT) {
            ir_add(OP_MOVE, "LF@%retval", "nil@nil", NULL);
        } else if (return_addr != NULL) {
            if (current_function->symbolType == TYPE_DOUBLE && ret_type == TYPE_INT) {
                ir_add(OP_INT2FLOAT, "LF@%retval", return_addr, NULL);
            } else {
                ir_add(OP_MOVE, "LF@%retval", return_addr, NULL);
            }
        }
        char func_end_label[128];
        snprintf(func_end_label, sizeof(func_end_label), "$%s_end", current_function->name);
        ir_add(OP_JUMP, func_end_label, NULL, NULL);
    }

    free(return_addr);

    return SYNTAX_OK;
}

int parse_if(DataType *out_type) {
    //if
    if(expect_token(TOKEN_KW_IF) != SYNTAX_OK)
        return SYNTAX_ERROR;

    //vyraz
    DataType condition_type = TYPE_UNKNOWN;
    char *addr = NULL;
    int res = parse_expr(&condition_type, &addr);
    if (res != SYNTAX_OK){
        free(addr);
        return res;
    }

    //then
    if(expect_token(TOKEN_KW_THEN) != SYNTAX_OK) { 
        free(addr);
        return SYNTAX_ERROR;
    }
    if(expect_token(TOKEN_EOL) != SYNTAX_OK){ 
        free(addr);
        return SYNTAX_ERROR;
    }

    //vygenerovani podminky
    char *label_else = NULL;
    char *label_end = NULL;
    if (on_full_pass && addr != NULL) {
        label_else = ir_create_label("if_else");
        label_end = ir_create_label("if_end");
        ir_add(OP_JUMPIFEQ, label_else, addr, "bool@false");
    }
    free(addr);

    //blok 
    if (!symtable_stack_push(&stack)) {
        free(label_else);
        free(label_end);
        return INTERNAL_ERROR;
    }
    current_scope++;

    DataType then_type = TYPE_UNIT;
    res = parse_block(&then_type, NULL);

    symtable_stack_pop(&stack);
    current_scope--;
    if(res != SYNTAX_OK) {
        free(label_else);
        free(label_end);
        return res;
    }

    //vygenerovani skoku na konec
    if (on_full_pass) {
        ir_add(OP_JUMP, label_end, NULL, NULL);
        ir_add(OP_LABEL, label_else, NULL, NULL);
    }

    //else
    if(expect_token(TOKEN_KW_ELSE) != SYNTAX_OK){
        free(label_else);
        free(label_end);
        return SYNTAX_ERROR;
    }
    if(expect_token(TOKEN_EOL) != SYNTAX_OK){
        free(label_else);
        free(label_end);
        return SYNTAX_ERROR;
    }

    //else blok
    if (!symtable_stack_push(&stack)) {
        free(label_else);
        free(label_end);
        return INTERNAL_ERROR;
    }
    current_scope++;

    DataType else_type = TYPE_UNIT;
    res = parse_block(&else_type, NULL);

    symtable_stack_pop(&stack);
    current_scope--;

    if (on_full_pass) {
        ir_add(OP_LABEL, label_end, NULL, NULL);
    }
    free(label_else);
    free(label_end);

    if (res != SYNTAX_OK) {
        return res;
    }
    if(condition_type != TYPE_BOOL) {
        return SEM_ERROR_TYPE_MISMATCH;
    }

    if (out_type != NULL) {
        if (then_type == TYPE_NEVER && else_type == TYPE_NEVER) {
            *out_type = TYPE_NEVER;
        } 
        else if (then_type == TYPE_UNIT && else_type == TYPE_UNIT) {
            *out_type = TYPE_UNIT;
        } 
        else if ((then_type == TYPE_NEVER && else_type == TYPE_UNIT) || (then_type == TYPE_UNIT && else_type == TYPE_NEVER)) {
            *out_type = TYPE_UNIT;
        } 
        else {
            *out_type = TYPE_UNKNOWN;
        }
    }

    return SYNTAX_OK;
}

int parse_while() {
    //while
    if(expect_token(TOKEN_KW_WHILE) != SYNTAX_OK)
        return SYNTAX_ERROR;

    //vygenerovani labelu pro start
    char *label_start = NULL;
    char *label_end = NULL;
    if (on_full_pass) {
        label_start = ir_create_label("while_start");
        label_end = ir_create_label("while_end");
        ir_add(OP_LABEL, label_start, NULL, NULL);
    }

    //vyraz
    DataType condition_type = TYPE_UNKNOWN;
    char *addr = NULL;
    int res = parse_expr(&condition_type, &addr);
    if (res != SYNTAX_OK){
        free(label_start);
        free(label_end);
        free(addr);
        return res;
    }

    if (on_full_pass && addr != NULL) {
        ir_add(OP_JUMPIFEQ, label_end, addr, "bool@false");
    }
    free(addr);

    //do
    if(expect_token(TOKEN_KW_DO) != SYNTAX_OK){
        free(label_start);
        free(label_end);
        return SYNTAX_ERROR;
    }
    if(expect_token(TOKEN_EOL) != SYNTAX_OK){
        free(label_start);
        free(label_end);
        return SYNTAX_ERROR;
    }

    if (!symtable_stack_push(&stack)) {
        free(label_start);
        free(label_end);
        return INTERNAL_ERROR;
    }
    current_scope++;

    res = parse_block(NULL, NULL);

    symtable_stack_pop(&stack);
    current_scope--;

    //pridani start a labelu pro konec while cyklu
    if (on_full_pass) {
        ir_add(OP_JUMP, label_start, NULL, NULL);
        ir_add(OP_LABEL, label_end, NULL, NULL);
    }
    free(label_start);
    free(label_end);

    if(res != SYNTAX_OK) {
        return res;
    }
    if(condition_type != TYPE_BOOL) {
        return SEM_ERROR_TYPE_MISMATCH;
    }

    return SYNTAX_OK;
}