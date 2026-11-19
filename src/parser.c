/*
    Implementace překladače imperativního jazyka IFJ26.
*/
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "symtable.h"
#include "scanner.h"

Symtable *table;
int token;          // globalni promenna, ve ktere bude ulozen aktualni token
char *attr;        // globalni promenna, ve ktere bude ulozen atribut tokenu

int parse(Symtable *symtable)
{
    table = symtable;
    Token test = get_token();
    token = test.type;
    attr = test.value;
    return SEM_OK;
}