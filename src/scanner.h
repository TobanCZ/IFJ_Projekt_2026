/*
    Implementace překladače imperativního jazyka IFJ26.
*/



#ifndef SCANNER_H
#define SCANNER_H
#include "token.h"


// Returning the next token.
Token get_token(void);
Token peek_token(void);


#endif
