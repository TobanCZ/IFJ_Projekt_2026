/*
    Implementace překladače imperativního jazyka IFJ26.
    autor: Michal Böhm(xbohmmi00)
*/

#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <stdbool.h>

typedef enum {
    TYPE_INT,    // integer type
    TYPE_DOUBLE, // double type
    TYPE_STRING, // string type
    TYPE_BOOL,   // boolean type
    TYPE_UNIT,   // unit type (void)
    TYPE_ANY,    // any type
    TYPE_UNKNOWN, // unknown type (used for functions returns)
    TYPE_NEVER    //used for if elses with defined return type
} DataType;

typedef struct Symbol {
    char *name;               // Název symbolu
    DataType symbolType;      // Datový typ symbolu
    int scopeLevel;           // Úroveň rozsahu (scope level) symbolu
    bool isFunction;          // Vyžaduje stdbool.h
    DataType *parameterTypes; // Dynamické pole typů
    int paramCount;           // Počet parametrů ve výše uvedeném poli
    bool isMutable;           // rozlišení var a val
    char *irName;
} Symbol;

// Deklarace uzlu zásobníku
struct SymtableStackNode;

// Deklarace zásobníku tabulek symbolů
typedef struct {
    struct SymtableStackNode *top; // Ukazatel na vrchol zásobníku
} SymtableStack;

// Inicializace zásobníku tabulek symbolů
void symtable_stack_init(SymtableStack *stack);

// Vstup do nového bloku: alokuje novou prázdnou Symtable a vloží ji na vrchol
bool symtable_stack_push(SymtableStack *stack);

// Výstup z bloku: uvolní vrcholovou Symtable (symtable_destroy) a odstraní uzel
void symtable_stack_pop(SymtableStack *stack);

// Vloží symbol pouze do aktuálního (vrcholového) bloku
bool symtable_stack_insert(SymtableStack *stack, const char *key, Symbol data);

// Hledá symbol jen v aktuálním bloku
Symbol *symtable_stack_find_current(SymtableStack *stack, const char *key);

// Hledá symbol od vrcholu směrem dolů přes nadřazené bloky
Symbol *symtable_stack_find(SymtableStack *stack, const char *key);

// Uvolní celý zásobník včetně všech zbývajících tabulek
void symtable_stack_destroy(SymtableStack *stack);

struct BSTNode;
typedef struct BSTNode *Symtable;

void symtable_init(Symtable *table); // Inicializace tabulky symbolů

bool symtable_insert(Symtable *table, const char *key,
                     Symbol data); // Vložení symbolu do tabulky symbolů

Symbol *symtable_find(Symtable *table,
                      const char *key); // Vyhledání symbolu v tabulce symbolů podle klíče

void symtable_destroy(Symtable *table); // Uvolnění paměti alokované pro tabulku symbolů a její uzly

#endif
