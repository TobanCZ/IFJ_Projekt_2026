/*
    Implementace překladače imperativního jazyka IFJ26.
    autor: Michal Böhm(xbohmmi00)
*/

#ifndef BST_H
#define BST_H

#include <stdbool.h>

typedef struct BSTNode {
    char *key;
    void *data;
    int height;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

// Inicializace prázdného stromu
void bst_init(BSTNode **tree);

// Vloží data do stromu, vrací true, pokud byl vložen, false pokud již existuje
bool bst_insert(BSTNode **tree, const char *key, void *data);
// Vyhledá data podle klíče, vrací ukazatel na data nebo NULL, pokud nebyl nalezen
void *bst_find(BSTNode *tree, const char *key);
// Uvolní celý strom a všechny jeho uzly
void bst_dispose(BSTNode **tree, void (*free_data)(void *data));

#endif