/*
    Implementace překladače imperativního jazyka IFJ26.

*/
#include <stdbool.h>

typedef enum {
    TYPE_INT,    // integer type
    TYPE_DOUBLE, // double type
    TYPE_STRING, // string type
    TYPE_BOOL,   // boolean type
    TYPE_UNIT,   // unit type (void)
} DataType;

typedef struct Symbol {
    char *name;               // Název symbolu
    DataType symbolType;      // Datový typ symbolu
    int scopeLevel;           // Úroveň rozsahu (scope level) symbolu
    bool isFunction;          // Vyžaduje stdbool.h
    DataType *parameterTypes; // Dynamické pole typů
    int paramCount;           // Počet parametrů ve výše uvedeném poli
} Symbol;

typedef struct SymtableNode {
    char *key;                  // Klíč (název symbolu)
    Symbol data;                // Data symbolu
    int height;                 // Výška uzlu (pro AVL strom)
    struct SymtableNode *left;  // Levý potomek
    struct SymtableNode *right; // Pravý potomek
} SymtableNode;

typedef SymtableNode *Symtable;

void symtable_init(Symtable *table); // Inicializace tabulky symbolů

bool symtable_insert(Symtable *table, const char *key,
                     Symbol data); // Vložení symbolu do tabulky symbolů

Symbol *symtable_find(Symtable *table,
                      const char *key); // Vyhledání symbolu v tabulce symbolů podle klíče

void symtable_destroy(Symtable *table); // Uvolnění paměti alokované pro tabulku symbolů a její uzly