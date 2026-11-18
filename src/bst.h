#ifndef BST_H
#define BST_H

#include "symtable.h" // Abychom znali strukturu Symbol

typedef struct BSTNode {
    char *key;
    Symbol data;
    int height;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

void bst_init(BSTNode **tree);
bool bst_insert(BSTNode **tree, const char *key, Symbol data);
Symbol *bst_find(BSTNode *tree, const char *key);
void bst_dispose(BSTNode **tree);

#endif