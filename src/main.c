/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#include <stdio.h>
#include "parser.h"
#include "symtable.h"
#include "scanner.h"


int main(int argc, char** argv) { 

    int result;
    result = parse();

    if(result == SEM_OK)
    {
        printf("Program je syntakticky a semanticky spravny\n");
    }
    else
    {
        printf("Program obsahuje chybu, kod chyby: %d\n", result);
    }

    return 0;
}