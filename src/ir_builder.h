/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#include <stdbool.h>
#include "ir.h"
#include "scanner.h"
#include "symtable.h"

void ir_builder_init(void);
IRNode *get_ir(void);

IRNode *ir_add(Opcode op, const char *dest, const char *op1, const char *op2); //pridani instrukce
void ir_set_func_anchor(void);  //nastaveni kotvi na zacatek tela
void ir_add_defvar(const char *var_name); //zacatek funkce

char *ir_create_tmp(void); //unikatni docasna promenna
char *ir_create_local_var(const char *src_name); //unikatni lokalni prom.
char *ir_create_label(const char *prefix); //unikatni label
char *ir_from_token_to_operand(Token tok); //token -> ifjcode operand