/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#include <stdbool.h>
#include "expr.h"
#include "token.h"
#include "parser.h"

// precedencni tabulka [vrchol zasobniku][vstupni token]
//E-error; <-posun; >-vyhodnotit; =-zarazka 
static const char precedent_table[15][15] = {
// Vstup:    u-   *    /    +    -    <    <=   >    >=   ==   !=   (    )    i    $
/* u- */  { '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' },
/* *  */  { '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' },
/* /  */  { '<', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' },
/* +  */  { '<', '<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' },
/* -  */  { '<', '<', '<', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' },
/* <  */  { '<', '<', '<', '<', '<', 'E', 'E', 'E', 'E', '>', '>', '<', '>', '<', '>' },
/* <= */  { '<', '<', '<', '<', '<', 'E', 'E', 'E', 'E', '>', '>', '<', '>', '<', '>' },
/* >  */  { '<', '<', '<', '<', '<', 'E', 'E', 'E', 'E', '>', '>', '<', '>', '<', '>' },
/* >= */  { '<', '<', '<', '<', '<', 'E', 'E', 'E', 'E', '>', '>', '<', '>', '<', '>' },
/* == */  { '<', '<', '<', '<', '<', '<', '<', '<', '<', 'E', 'E', '<', '>', '<', '>' },
/* != */  { '<', '<', '<', '<', '<', '<', '<', '<', '<', 'E', 'E', '<', '>', '<', '>' },
/* (  */  { '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '=', '<', 'E' },
/* )  */  { 'E', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'E', '>', 'E', '>' },
/* i  */  { 'E', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', 'E', '>', 'E', '>' },
/* $  */  { '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', 'E', '<', 'E' }
};

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
        case TOKEN_PUNCTUATION_LEFT_PAREN:
            return PSA_LEFT_PAREN;
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

int parse_expr(){
        while (current_token.type != TOKEN_KW_THEN && current_token.type != TOKEN_KW_DO &&
           current_token.type != TOKEN_EOL && current_token.type != TOKEN_PUNCTUATION_SEMICOLON &&
           current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        next_token();
    }
    return SYNTAX_OK;
}