/*
    Implementace překladače imperativního jazyka IFJ26.
*/



#include <stddef.h>
#include "scanner.h"



Token get_token(void)
{
    Token token = { TOKEN_EOF, NULL, 1 };
    return token;
}