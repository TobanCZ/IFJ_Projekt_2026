#include "symtable.h"
#include "bst.h"

void symtable_init(Symtable *table) {
    bst_init((BSTNode **)table);
}

bool symtable_insert(Symtable *table, const char *key, Symbol data) {
    return bst_insert((BSTNode **)table, key, data);
}

Symbol *symtable_find(Symtable *table, const char *key) {
    return bst_find((BSTNode *)*table, key); // U vyhledávání posíláme hodnotu kořene, ne adresu
}

void symtable_destroy(Symtable *table) {
    bst_dispose((BSTNode **)table);
}