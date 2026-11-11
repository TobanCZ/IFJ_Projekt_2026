/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#include "symtable.h"
#include <stdlib.h>
#include <string.h>

void symtable_init(Symtable *table) {
    // Zatim prazdne
    (void)table;
}

bool symtable_insert(Symtable *table, const char *key, Symbol data) {
    // Zatim jen aby to proslo prekladem
    (void)table;
    (void)key;
    (void)data;
    return true;
}

Symbol *symtable_find(Symtable *table, const char *key) {
    // Zatim nic nenajde
    (void)table;
    (void)key;
    return NULL;
}

void symtable_destroy(Symtable *table) {
    (void)table;
    // Zatim prazdne
}