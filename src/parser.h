/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#define OK                                  0 
#define SYNTAX_OK                           0
#define SEM_OK                              0
#define SYNTAX_ERROR                        2
#define SEM_ERROR_UNDEFINED                 3  //nedefinovana promenna, funkce nebo @main
#define SEM_ERROR_REDEFINITION              4  //predefinovana promenna, funkce
#define SEM_ERROR_INVALID_ARGUMENT_COUNT    5  //nespravny pocet argumentu, nepovelen parametry pro @main
#define SEM_ERROR_TYPE_MISMATCH             6  //nespravny typova kompatibilita
#define SEM_ERROR_INVALID_WRITE             7  //nepovolen zapis do promenne, parametru
#define SEM_ERROR_OTHER                     10 //nepovolen return
#define INTERNAL_ERROR                      99 //nepovolen return

// hlavicka pro syntakticky analyzator
#include "token.h"
#include "symtable.h"

extern SymtableStack stack;

typedef enum {
    PASS_HEADERS, //hlavicky funkci do tabulky
    PASS_INFER,   //funkce bez typu otypuje
    PASS_FULL     //celej pruchod
} ParsePass;

int parse();

extern Token current_token;
void next_token(void);
Token peek_token(void);
int token_type_to_data_type(TokenType token_type);
