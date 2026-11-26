/*
    Implementace překladače imperativního jazyka IFJ26.
    autor: Tobiáš Ondra(xondrat00)
*/

#include "parser.h"
#include "scanner.h"
#include "symtable.h"
#include <stdio.h>

int main() {
    int result;
    result = parse();

    if (result != SYNTAX_OK && result != SEM_OK && result != OK) {
        fprintf(stderr, "Program error: %d\n", result);
    }

    return result;
}