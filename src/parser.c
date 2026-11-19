/*
    Implementace překladače imperativního jazyka IFJ26.
*/
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "symtable.h"
#include "scanner.h"

int parse_program();
int parse_func_def();

Symtable *table;
int token;          // globalni promenna, ve ktere bude ulozen aktualni token
char *attr;        // globalni promenna, ve ktere bude ulozen atribut tokenu
bool has_main = false;

void next_token() {
    Token test = get_token();
    token = test.type;
    attr = test.value;
}

int expect_token(int expected_token) {
    if (token != expected_token) {
        return SYNTAX_ERROR;
    }
    next_token();
    return SYNTAX_OK;
}

int expect_token_and_attr(int expected_token, char* expected_attr) {
    if (token != expected_token || attr == NULL || strcmp(attr, expected_attr) != 0) {
        return SYNTAX_ERROR;
    }
    next_token();
    return SYNTAX_OK;
}


int parse(Symtable *symtable)
{
    table = symtable;

    int res =parse_program();

    return res;
}

int parse_program(){
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

    while(token != TOKEN_EOF)
    {
        int res = parse_func_def();
        if (res != SYNTAX_OK)
            return res;
    }

    return SYNTAX_OK;
}

int parse_func_def(){
    next_token();
    fprintf(stderr, "Token: %d, Attr: %s\n", token, attr);
}
