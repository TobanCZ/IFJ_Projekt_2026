/*
    Implementace překladače imperativního jazyka IFJ26.
    autor: Michal Böhm(xbohmmi00)
*/

#include "symtable.h"
#include "bst.h"
#include "help.h"
#include <stdlib.h>

// Pomocná funkce pro uvolnění jednoho Symbolu (callback pro bst_dispose)
static void free_symbol(void *data) {
    Symbol *sym = (Symbol *)data;
    if (sym->parameterTypes != NULL) {
        free(sym->parameterTypes);
    }
    free(sym->name);
    free(sym->irName);
    free(sym);
}

void symtable_init(Symtable *table) {
    if (table == NULL) {
        return; // Neplatný vstup
    }
    bst_init((BSTNode **)table);
}

bool symtable_insert(Symtable *table, const char *key, Symbol data) {
    if (table == NULL || key == NULL) {
        return false; // Neplatný vstup
    }

    Symbol *copy = (Symbol *)malloc(sizeof(Symbol));
    if (copy == NULL) {
        return false;
    }
    *copy = data;
    // Hluboka kopie
    copy->name   = data.name ? copy_str(data.name) : NULL;
    copy->irName = data.irName ? copy_str(data.irName) : NULL;
    if (data.paramCount > 0 && data.parameterTypes != NULL) {
        copy->parameterTypes = malloc(sizeof(DataType) * data.paramCount);
        if (copy->parameterTypes == NULL) {
            free_symbol(copy);
            return false;
        }
        memcpy(copy->parameterTypes, data.parameterTypes, sizeof(DataType) * data.paramCount);
    } else {
        copy->parameterTypes = NULL;
    }

    if (!bst_insert((BSTNode **)table, key, copy)) {
        free_symbol(copy); // Klíč už v tabulce existuje nebo selhala alokace v BST
        return false;
    }
    return true;
}

Symbol *symtable_find(Symtable *table, const char *key) {
    if (table == NULL) {
        return NULL; // Neplatný vstup
    }
    return bst_find((BSTNode *)*table, key); // U vyhledávání posíláme hodnotu kořene, ne adresu
}

void symtable_destroy(Symtable *table) {
    bst_dispose((BSTNode **)table, free_symbol);
}

typedef struct SymtableStackNode {
    Symtable table;                 // Tabulka symbolů pro tento konkrétní blok
    struct SymtableStackNode *next; // Odkaz na nadřazený (vnější) blok
} SymtableStackNode;

// Inicializuje prázdný zásobník tabulek
void symtable_stack_init(SymtableStack *stack) {
    if (stack == NULL) {
        return;
    }
    stack->top = NULL;
}

// Vloží novou prázdnou tabulku na vrchol zásobníku (vstup do bloku)
bool symtable_stack_push(SymtableStack *stack) {
    if (stack == NULL) {
        return false;
    }
    SymtableStackNode *node = (SymtableStackNode *)malloc(sizeof(SymtableStackNode));
    if (node == NULL) {
        return false;
    }
    symtable_init(&node->table);
    node->next = stack->top;
    stack->top = node;
    return true;
}

// Odstraní a uvolní vrcholovou tabulku (výstup z bloku)
void symtable_stack_pop(SymtableStack *stack) {
    if (stack == NULL || stack->top == NULL) {
        return;
    }
    SymtableStackNode *node = stack->top;
    stack->top              = node->next;
    symtable_destroy(&node->table);
    free(node);
}

// Vloží symbol do aktuálního (vrcholového) bloku
bool symtable_stack_insert(SymtableStack *stack, const char *key, Symbol data) {
    if (stack == NULL || stack->top == NULL) {
        return false;
    }
    return symtable_insert(&stack->top->table, key, data);
}

// Vyhledá symbol pouze v aktuálním bloku (kontrola redefinice)
Symbol *symtable_stack_find_current(SymtableStack *stack, const char *key) {
    if (stack == NULL || stack->top == NULL) {
        return NULL;
    }
    return symtable_find(&stack->top->table, key);
}

// Vyhledá symbol od nejvnitřnějšího bloku směrem ven
Symbol *symtable_stack_find(SymtableStack *stack, const char *key) {
    if (stack == NULL) {
        return NULL;
    }
    for (SymtableStackNode *curr = stack->top; curr != NULL; curr = curr->next) {
        Symbol *found = symtable_find(&curr->table, key);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

// Uvolní celý zásobník i se všemi zbývajícími tabulkami
void symtable_stack_destroy(SymtableStack *stack) {
    if (stack == NULL) {
        return;
    }
    while (stack->top != NULL) {
        symtable_stack_pop(stack);
    }
}