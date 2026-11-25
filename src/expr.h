/*
    Implementace překladače imperativního jazyka IFJ26.
*/

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

int parse_expr();