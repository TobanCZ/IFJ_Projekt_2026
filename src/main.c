/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#include <stdio.h>
#include "parser.h"
#include "symtable.h"
#include "scanner.h"


int main(int argc, char** argv) { 
    Symtable symtable;
    symtable_init(&symtable);

    int result;
    result = parse(&symtable);

    if(result == SEM_OK)
    {
        printf("Program je syntakticky a semanticky spravny\n");
    }
    else
    {
        printf("Program obsahuje chybu, kod chyby: %d\n", result);
    }

    symtable_destroy(&symtable);

    return 0;
}