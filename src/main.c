/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#include <stdio.h>
#include "parser.h"
#include "symtable.h"
#include "scanner.h"


int main() { 
    int result;
    result = parse();

    if(result != SYNTAX_OK && result != SEM_OK && result != OK)
    {
        fprintf(stderr,"Program error: %d\n", result);
    }

    return result;
}