/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#include "symtable.h"
#include <stdlib.h>
#include <string.h>

// BVS implementace tabulky symbolů s vyvažováním (AVL strom)
// Pomocná funkce pro porovnání klíčů
static int max_int(int a, int b) {
    return (a > b) ? a : b;
}

// Pomocná funkce pro získání výšky uzlu
static int get_height(SymtableNode *node) {
    return (node == NULL) ? 0 : node->height;
}

// Výpočet faktoru vyvážení
static int get_balance(SymtableNode *node) {
    if (node == NULL) {
        return 0;
    }
    return get_height(node->left) - get_height(node->right);
}

// Pravá rotace
static SymtableNode *rotate_right(SymtableNode *y) {
    SymtableNode *x  = y->left;
    SymtableNode *T2 = x->right;

    // Přepojení ukazatelů
    x->right = y;
    y->left  = T2;

    // Aktualizace výšek (nejprve uzel y, který klesl níž, pak nový kořen x)
    y->height = 1 + max_int(get_height(y->left), get_height(y->right));
    x->height = 1 + max_int(get_height(x->left), get_height(x->right));

    return x;
}

// Levá rotace
static SymtableNode *rotate_left(SymtableNode *x) {
    SymtableNode *y  = x->right;
    SymtableNode *T2 = y->left;

    y->left  = x;
    x->right = T2;

    // Aktualizace výšek (nejprve uzel x, který klesl níž, pak nový kořen y)
    x->height = 1 + max_int(get_height(x->left), get_height(x->right));
    y->height = 1 + max_int(get_height(y->left), get_height(y->right));

    return y;
}

static SymtableNode *insert_recursive(SymtableNode *node, const char *key, Symbol data,
                                      bool *inserted) {
    // Základní případ: pokud je uzel NULL, vytvoříme nový uzel
    if (node == NULL) {
        SymtableNode *newNode = (SymtableNode *)malloc(sizeof(SymtableNode));
        if (newNode == NULL) {
            return NULL; // Alokace paměti selhala
        }
        newNode->key = (char *)malloc(strlen(key) + 1); // Duplikace klíče
        if (newNode->key == NULL) {
            free(newNode);
            return NULL;
        }
        strcpy(newNode->key, key);

        newNode->data   = data; // Kopírování dat symbolu
        newNode->height = 1;    // Nový uzel má výšku 1
        newNode->left   = NULL;
        newNode->right  = NULL;

        *inserted = true; // Je to nový vložený prvek
        return newNode;
    }

    // Rekurzivní vložení do levého nebo pravého podstromu
    int cmp = strcmp(key, node->key);
    if (cmp < 0) {
        node->left = insert_recursive(node->left, key, data, inserted);
    } else if (cmp > 0) {
        node->right = insert_recursive(node->right, key, data, inserted);
    } else {
        *inserted = false; // Není to nový vložený prvek
        return node;
    }

    if (!(*inserted)) {
        return node;
    }
    // Aktualizace výšky uzlu
    node->height = 1 + max_int(get_height(node->left), get_height(node->right));

    // Získání faktoru vyvážení
    int balance = get_balance(node);

    // Rotace pro vyvážení stromu
    if (balance > 1 && strcmp(key, node->left->key) < 0) { // Levá-levá situace
        return rotate_right(node);
    }
    if (balance < -1 && strcmp(key, node->right->key) > 0) { // Pravá-pravá situace
        return rotate_left(node);
    }
    if (balance > 1 && strcmp(key, node->left->key) > 0) { // Levá-pravá situace
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (balance < -1 && strcmp(key, node->right->key) < 0) { // Pravá-levá situace
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node; // Vracíme nevyvážený uzel (pokud nebyla provedena žádná rotace)
}
// Konec implementace AVL stromu pro tabulku symbolů

// Pomocná funkce pro rekurzivní uvolnění paměti
static void destroy_recursive(SymtableNode *node) {
    if (node == NULL) {
        return;
    }
    destroy_recursive(node->left);
    destroy_recursive(node->right);

    if (node->data.parameterTypes != NULL) {
        free(node->data.parameterTypes);
    }

    free(node->key); // Uvolnění paměti pro klíč
    free(node);      // Uvolnění paměti pro uzel
}

// Pomocná funkce pro vyhledání symbolu v tabulce symbolů
static Symbol *find_recursive(SymtableNode *node, const char *key) {
    if (node == NULL) {
        return NULL; // Symbol nebyl nalezen
    }

    int cmp = strcmp(key, node->key);
    if (cmp == 0) {
        return &(node->data); // Symbol nalezen, vracíme ukazatel na data symbolu
    } else if (cmp < 0) {
        return find_recursive(node->left, key); // Hledáme v levém podstromu
    } else {
        return find_recursive(node->right, key); // Hledáme v pravém podstromu
    }
}

void symtable_init(Symtable *table) {
    *table = NULL; // Inicializace tabulky symbolů jako prázdného stromu
}

bool symtable_insert(Symtable *table, const char *key, Symbol data) {
    if (table == NULL || key == NULL) {
        return false; // Neplatný vstup
    }

    bool inserted = false;
    *table        = insert_recursive(*table, key, data, &inserted);
    return inserted;
}

Symbol *symtable_find(Symtable *table, const char *key) {
    if (table == NULL || key == NULL) {
        return NULL; // Neplatný vstup
    }
    return find_recursive(*table, key);
}

void symtable_destroy(Symtable *table) {
    destroy_recursive(*table);
    *table = NULL;
}