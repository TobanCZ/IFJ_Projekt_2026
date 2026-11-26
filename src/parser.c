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

#define MAX_PARAM_COUNT 1024

int add_builtins_functions(Symtable *table);

int parse_program(bool first_pass);
int parse_func_def(bool first_pass);

int parse_expr_or_call();
int parse_block();

int parse_statement();

int parse_def();
int parse_assign();
int parse_call();
int parse_return();
int parse_if();
int parse_while();
int parse_expr();

static Symtable table; //funkce
static SymtableStack stack;  //promenne a parametry

Token current_token;
bool has_main = false;
int current_scope = 0;

//Pro moznost 2 pruchodu 
Token* tokens = NULL;
static int token_count = 0;
static int token_index = 0;

void free_all_tokens(void) {
    for (size_t i = 0; i < token_count; i++) {
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
            return TYPE_INT;
        case TOKEN_KW_DOUBLE:
            return TYPE_DOUBLE;
        case TOKEN_KW_STRING:
            return TYPE_STRING;
        case TOKEN_KW_UNIT:
            return TYPE_UNIT;
        default:
            return TYPE_UNKNOWN;
    }
}


int parse() {
    has_main = false;
    current_scope = 0;
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
    token_index = 0;
    //prvni pruchod, nacteni funkci
    res = parse_program(true);
    if(res != SYNTAX_OK) {
        free_all_tokens();
        symtable_destroy(&table);
        symtable_stack_destroy(&stack);
        return res;
    }

    //druhy pruchod
    token_index = 0;
    has_main = false;
    res = parse_program(false);

    free_all_tokens();
    symtable_destroy(&table);
    symtable_stack_destroy(&stack);
    return res;
}

//pridani vestavenych funkci TODO: mozna to dat do funkce protoze se toho dost opakuje
int add_builtins_functions(Symtable *table) {
    Symbol function;

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

int parse_program(bool first_pass){
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

    //Do konce souboru
    while(current_token.type != TOKEN_EOF) {
        int res = parse_func_def(first_pass);
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
        
        Symbol param;
        param.name = param_name;
        param.symbolType = token_type_to_data_type(current_token.type);
        param.scopeLevel = current_scope;
        param.isFunction = false;
        param.isMutable = false;
        param.paramCount = 0;
        param.parameterTypes = NULL;
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
    //blok nebo jednoratkovej vyraz;
    if (current_token.type == TOKEN_EOL) {
        next_token();
        return parse_block();
    }

    int res = parse_expr_or_call();
    if (res != SYNTAX_OK) return res;

    if (current_token.type == TOKEN_EOL) {
        next_token();
    } else if (current_token.type != TOKEN_EOF) {
        return SYNTAX_ERROR;
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
        return (depth == 0) ? SYNTAX_OK : SYNTAX_ERROR;
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

int parse_func_def(bool first_pass) {
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
    if (first_pass && symtable_find(&table, current_token.value) != NULL) 
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
        if(first_pass){
            Symbol function;
            function.name = func_name;
            function.symbolType = token_type_to_data_type(ret_type);
            function.scopeLevel = 0;
            function.isFunction = true;
            function.paramCount = param_count;
            function.isMutable = false;
            if(param_count > 0)
                function.parameterTypes = param_types;
            else
                function.parameterTypes = NULL;

            symtable_insert(&table, function.name, function);

            res = skip_func_body();
        }
        else 
            res = parse_func_body();
    }

    free(func_name); 
    symtable_stack_pop(&stack);
    current_scope--;
    return res;
}

int parse_block() {
    //Zacatek bloku
    if (expect_token(TOKEN_BLOCK_START) != SYNTAX_OK) 
        return SYNTAX_ERROR;

    // Blok nesmi byt prazdny
    if (current_token.type == TOKEN_BLOCK_END || current_token.type == TOKEN_EOF)
        return SYNTAX_ERROR;

    //Do konce bloku nebo souboru
    while (current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        int res = parse_statement();
        if (res != SYNTAX_OK) {
            return res;
        }
    }

    //konec bloku
    if (expect_token(TOKEN_BLOCK_END) != SYNTAX_OK)
        return SYNTAX_ERROR;

    return SYNTAX_OK;
}

int parse_expr_or_call() {
    if (current_token.type == TOKEN_IDENT) {
        Token next = peek_token();
        if (next.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            return parse_call();
        }
    }
    return parse_expr();
}

int parse_statement() {
    //Blokove prikazy
    if (current_token.type == TOKEN_KW_IF)
        return parse_if();

    if (current_token.type == TOKEN_KW_WHILE)
        return parse_while();

    //Jednoradkove
    int res = SYNTAX_ERROR;

    if (current_token.type == TOKEN_KW_VAL || current_token.type == TOKEN_KW_VAR) {
        res = parse_def();
    } 
    else if (current_token.type == TOKEN_KW_RETURN) {
        res = parse_return();
    } 
    else if (current_token.type == TOKEN_IDENT) {
        // prirazeni nebo funkce
        Token next = peek_token();
        if (next.type == TOKEN_OPERATOR_ASSIGN) {
            res = parse_assign();
        } 
        else if (next.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            res = parse_call();
        } 
        else {
            res = parse_expr();
        }
    } 
    else if (current_token.type == TOKEN_PUNCTUATION_LEFT_PAREN || current_token.type == TOKEN_OPERATOR_MINUS || current_token.type == TOKEN_INT || current_token.type == TOKEN_DOUBLE || current_token.type == TOKEN_STRING ||current_token.type == TOKEN_KW_UNIT) {
        res = parse_expr();
    } 
    else {
        return SYNTAX_ERROR;
    }

    if (res != SYNTAX_OK) 
        return res;

    //Oddelovac za jednoradkovym prikazem
    if (current_token.type == TOKEN_EOL || current_token.type == TOKEN_PUNCTUATION_SEMICOLON) {
        next_token();
    } else if (current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        return SYNTAX_ERROR;
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

    int res = parse_expr_or_call();

    if(res == SYNTAX_OK) {
        Symbol symbol;
        symbol.name = name;
        symbol.symbolType = data_type;
        symbol.scopeLevel = current_scope;
        symbol.isFunction = false;
        symbol.isMutable = is_mutable;
        symbol.paramCount = 0;
        symbol.parameterTypes = NULL;
        symtable_stack_insert(&stack, symbol.name, symbol);
    }
   
    free(name);
    return res;
}

int parse_assign() {
    //jmeno promenne

    if(current_token.type != TOKEN_IDENT) 
        return SYNTAX_ERROR;
    Symbol *symbol = symtable_stack_find(&stack, current_token.value);
    if (symbol == NULL) {
        return SEM_ERROR_UNDEFINED;
    }
    if(!symbol->isMutable) {
        return SEM_ERROR_REDEFINITION;
    }
    next_token();

    //=
    if(expect_token(TOKEN_OPERATOR_ASSIGN) != SYNTAX_OK)
        return SYNTAX_ERROR;

    return parse_expr_or_call();
}

int parse_call() {
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
    bool undefined_param = false; //syntax chyby maji prednost

    //parametry
    while (current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN && current_token.type != TOKEN_EOF) {
        //vyraz nebo volani
        if (current_token.type == TOKEN_IDENT) {
            Symbol *arg = symtable_stack_find(&stack, current_token.value);
            if (arg == NULL) {
                undefined_param = true;
                param_types[param_count++] = TYPE_UNKNOWN;
            } else {
                param_types[param_count++] = arg->symbolType;
            }
            next_token();
        } 
        else if (current_token.type == TOKEN_INT) {
            param_types[param_count++] = TYPE_INT;
            next_token();
        } 
        else if (current_token.type == TOKEN_DOUBLE) {
            param_types[param_count++] = TYPE_DOUBLE;
            next_token();
        } 
        else if (current_token.type == TOKEN_STRING) {
            param_types[param_count++] = TYPE_STRING;
            next_token();
        } 
        else if (current_token.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            next_token();
            if (expect_token(TOKEN_PUNCTUATION_RIGHT_PAREN) != SYNTAX_OK) return SYNTAX_ERROR;
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

    return SYNTAX_OK;
}

int parse_return() {
    //return
    if(expect_token(TOKEN_KW_RETURN) != SYNTAX_OK)
        return SYNTAX_ERROR;

    if (current_token.type != TOKEN_EOL && current_token.type != TOKEN_PUNCTUATION_SEMICOLON &&
        current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        if (parse_expr_or_call() != SYNTAX_OK) {
            return SYNTAX_ERROR;
        }
    }
    return SYNTAX_OK;
}

int parse_if() {
    //if
    if(expect_token(TOKEN_KW_IF) != SYNTAX_OK)
        return SYNTAX_ERROR;

    //vyraz
    int res = parse_expr();
    if (res != SYNTAX_OK)
        return res;

    //then
    if(expect_token(TOKEN_KW_THEN) != SYNTAX_OK) 
        return SYNTAX_ERROR;
    if(expect_token(TOKEN_EOL) != SYNTAX_OK)
        return SYNTAX_ERROR;

    //blok 
    if (!symtable_stack_push(&stack)) {
        return INTERNAL_ERROR;
    }
    current_scope++;

    res = parse_block();

    symtable_stack_pop(&stack);
    current_scope--;
    if(res != SYNTAX_OK) {
        return res;
    }

    //else
    if(expect_token(TOKEN_KW_ELSE) != SYNTAX_OK)
        return SYNTAX_ERROR;
    if(expect_token(TOKEN_EOL) != SYNTAX_OK)
        return SYNTAX_ERROR;

    //else blok
    if (!symtable_stack_push(&stack)) {
        return INTERNAL_ERROR;
    }
    current_scope++;

    res = parse_block();

    symtable_stack_pop(&stack);
    current_scope--;
    return res;
}

int parse_while() {
    //while
    if(expect_token(TOKEN_KW_WHILE) != SYNTAX_OK)
        return SYNTAX_ERROR;

    //vyraz
    int res = parse_expr();
    if (res != SYNTAX_OK)
        return res;
    //do
    if(expect_token(TOKEN_KW_DO) != SYNTAX_OK)
        return SYNTAX_ERROR;
    if(expect_token(TOKEN_EOL) != SYNTAX_OK)
        return SYNTAX_ERROR;

    if (!symtable_stack_push(&stack)) {
        return INTERNAL_ERROR;
    }
    current_scope++;

    res = parse_block();

    symtable_stack_pop(&stack);
    current_scope--;
    return res;
}