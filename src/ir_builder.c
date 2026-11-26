/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#include "ir_builder.h"
#include "help.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static IRNode *ir_first           = NULL; // vrchol
static IRNode *ir_last            = NULL; // posledni
static IRNode *func_defvar_anchor = NULL;

static int tmp_counter   = 0;
static int var_counter   = 0;
static int label_counter = 0;

void ir_builder_init(void) {
    ir_first           = NULL;
    ir_last            = NULL;
    func_defvar_anchor = NULL;
    tmp_counter        = 0;
    var_counter        = 0;
    label_counter      = 0;
}

IRNode *ir_add(Opcode op, const char *dest, const char *op1, const char *op2) {
    IRNode *node = malloc(sizeof(IRNode));
    if (!node)
        return NULL;

    node->opcode   = op;
    node->dest     = dest ? copy_str(dest) : NULL;
    node->operand1 = op1 ? copy_str(op1) : NULL;
    node->operand2 = op2 ? copy_str(op2) : NULL;
    node->next     = NULL;

    // pridani na konec seznamu
    if (ir_first == NULL) {
        ir_first = ir_last = node;
    } else {
        ir_last->next = node;
        ir_last       = node;
    }
    return node;
}

IRNode *get_ir(void) {
    return ir_first;
}

void ir_set_func_anchor(void) {
    func_defvar_anchor = ir_last;
}

void ir_add_defvar(const char *var_name) {
    // bez kotvy nakonec
    if (func_defvar_anchor == NULL) {
        ir_add(OP_DEFVAR, var_name, NULL, NULL);
        return;
    }

    IRNode *node = malloc(sizeof(IRNode));
    if (!node)
        return;

    node->opcode   = OP_DEFVAR;
    node->dest     = copy_str(var_name);
    node->operand1 = NULL;
    node->operand2 = NULL;

    // vlozi se za kotvu na zacatku funkce
    node->next               = func_defvar_anchor->next;
    func_defvar_anchor->next = node;

    if (ir_last == func_defvar_anchor) {
        ir_last = node;
    }
    func_defvar_anchor = node;
}

char *ir_create_tmp(void) {
    char buf[64];
    snprintf(buf, sizeof(buf), "LF@$tmp%d", tmp_counter++);
    ir_add_defvar(buf);
    return copy_str(buf);
}

char *ir_create_local_var(const char *src_name) {
    char buf[128];
    snprintf(buf, sizeof(buf), "LF@%s$%d", src_name, var_counter++);
    ir_add_defvar(buf);
    return copy_str(buf);
}

char *ir_create_label(const char *prefix) {
    char buf[64];
    snprintf(buf, sizeof(buf), "$%s_%d", prefix, label_counter++);
    return copy_str(buf);
}

// Odstrani podtrzitka z ciselnych literalu (napr. "1_000" -> "1000")
static char *strip_underscores(const char *str) {
    size_t len  = strlen(str);
    char *clean = malloc(len + 1);
    if (!clean)
        return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] != '_') {
            clean[j++] = str[i];
        }
    }
    clean[j] = '\0';
    return clean;
}

char *ir_from_token_to_operand(Token token) {
    if (token.type == TOKEN_INT) {
        char *clean = strip_underscores(token.value);
        if (!clean)
            return NULL;
        size_t len = strlen(clean) + 5;
        char *res  = malloc(len);
        if (res)
            snprintf(res, len, "int@%s", clean);
        free(clean);
        return res;
    }
    if (token.type == TOKEN_DOUBLE) {
        char *clean = strip_underscores(token.value);
        if (!clean)
            return NULL;
        // Odstranime pripadnou koncovou priponu 'd' nebo 'D'
        size_t clen = strlen(clean);
        if (clen > 0 && (clean[clen - 1] == 'd' || clean[clen - 1] == 'D')) {
            clean[clen - 1] = '\0';
        }
        size_t len = strlen(clean) + 7;
        char *res  = malloc(len);
        // codegen.c v print_operand() sam prevede "float@..." pres %a
        if (res)
            snprintf(res, len, "float@%s", clean);
        free(clean);
        return res;
    }
    if (token.type == TOKEN_STRING) {
        size_t len = strlen(token.value) + 8;
        char *res  = malloc(len);
        // codegen.c v print_operand() sam escapuje znaky <= 32, '#' a '\'
        if (res)
            snprintf(res, len, "string@%s", token.value);
        return res;
    }
    // Literal Unit ()
    return copy_str("nil@nil");
}