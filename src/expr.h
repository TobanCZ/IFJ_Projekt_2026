/*
    Implementace překladače imperativního jazyka IFJ26.
*/
#include "symtable.h"


typedef enum 
{
    PSA_MINUS_UNAR,
    PSA_MULTIPLY,
    PSA_DIVIDE,
    PSA_PLUS,
    PSA_MINUS,
    PSA_LESS,
    PSA_LESS_EQUAL,
    PSA_GREATER,
    PSA_GREATER_EQUAL,
    PSA_EQUAL,
    PSA_NOT_EQUAL,
    PSA_LEFT_PAREN,
    PSA_RIGHT_PAREN,
    PSA_OPERAND,
    PSA_DOLLAR, //konec vyrazu

    PSA_EXPR, //neterminal
    PSA_HANDLE //zarazka
} PSA_Token;

// precedencni tabulka [vrchol zasobniku][vstupni token]
//E-error; <-posun; >-vyhodnotit; =- zavorky
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

typedef struct {
    int token;
    int data_type;
} PsaItem;

int parse_expr(DataType *out_type);