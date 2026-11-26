/*
    Implementace překladače imperativního jazyka IFJ26.
    autor: Jan Drcmánek(xdrcmaj00)
*/

#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gen_header(void) {
    printf(".IFJcode26\n");
}

static void print_escaped_string(const char *str) {
    printf("string@");
    if (str == NULL) {
        return;
    }

    for (size_t i = 0; str[i] != '\0'; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c <= 32 || c == 35 || c == 92) {
            printf("\\%03u", c);
        } else {
            putchar(c);
        }
    }
}

static void print_operand(const char *op) {
    if (op == NULL) {
        return;
    }

    if (strncmp(op, "float@", 6) == 0) {
        double val = strtod(op + 6, NULL);
        printf("float@%a", val);
        return;
    }

    if (strncmp(op, "string@", 7) == 0) {
        print_escaped_string(op + 7);
        return;
    }

    printf("%s", op);
}

static void emit_1(const char *instr, const char *op1) {
    printf("%s ", instr);
    print_operand(op1);
    putchar('\n');
}

static void emit_2(const char *instr, const char *op1, const char *op2) {
    printf("%s ", instr);
    print_operand(op1);
    putchar(' ');
    print_operand(op2);
    putchar('\n');
}

static void emit_3(const char *instr, const char *op1, const char *op2, const char *op3) {
    printf("%s ", instr);
    print_operand(op1);
    putchar(' ');
    print_operand(op2);
    putchar(' ');
    print_operand(op3);
    putchar('\n');
}

static void print_label(const char *label) {
    if (label == NULL) {
        return;
    }
    printf("%s", label);
}

void generate_builtins(void) {
    printf("\n# --- VESTAVENE FUNKCE IFJ26 ---\n");

    // --- 1. print(value: Any): Unit ---
    printf("\nLABEL $print\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval nil@nil\n");
    printf("JUMPIFEQ $print_unit LF@%%0 nil@nil\n");
    printf("WRITE LF@%%0\n");
    printf("JUMP $print_end\n");
    printf("LABEL $print_unit\n");
    printf("WRITE string@()\n");
    printf("LABEL $print_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 2. readLine(): String ---
    printf("\nLABEL $readLine\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("READ LF@%%retval string\n");
    printf("JUMPIFNEQ $readLine_end LF@%%retval nil@nil\n");
    printf("MOVE LF@%%retval string@\n");
    printf("LABEL $readLine_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 3. readInt(): Int ---
    printf("\nLABEL $readInt\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("READ LF@%%retval int\n");
    printf("JUMPIFNEQ $readInt_end LF@%%retval nil@nil\n");
    printf("MOVE LF@%%retval int@0\n");
    printf("LABEL $readInt_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 4. readDouble(): Double ---
    printf("\nLABEL $readDouble\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("READ LF@%%retval float\n");
    printf("JUMPIFNEQ $readDouble_end LF@%%retval nil@nil\n");
    printf("MOVE LF@%%retval float@0x0p+0\n");
    printf("LABEL $readDouble_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 5. toStr(value: Any): String ---
    printf("\nLABEL $toStr\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("DEFVAR LF@type\n");
    printf("TYPE LF@type LF@%%0\n");
    printf("JUMPIFEQ $toStr_int LF@type string@int\n");
    printf("JUMPIFEQ $toStr_float LF@type string@float\n");
    printf("JUMPIFEQ $toStr_str LF@type string@string\n");
    printf("JUMPIFEQ $toStr_bool LF@type string@bool\n");
    // Hodnota typu Unit (nil@nil)
    printf("MOVE LF@%%retval string@()\n");
    printf("JUMP $toStr_end\n");
    printf("LABEL $toStr_int\n");
    printf("INT2STR LF@%%retval LF@%%0\n");
    printf("JUMP $toStr_end\n");
    printf("LABEL $toStr_float\n");
    printf("FLOAT2STR LF@%%retval LF@%%0\n");
    printf("JUMP $toStr_end\n");
    printf("LABEL $toStr_str\n");
    printf("MOVE LF@%%retval LF@%%0\n");
    printf("JUMP $toStr_end\n");
    printf("LABEL $toStr_bool\n");
    printf("JUMPIFEQ $toStr_true LF@%%0 bool@true\n");
    printf("MOVE LF@%%retval string@false\n");
    printf("JUMP $toStr_end\n");
    printf("LABEL $toStr_true\n");
    printf("MOVE LF@%%retval string@true\n");
    printf("LABEL $toStr_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 6. toInt(value: Double): Int ---
    printf("\nLABEL $toInt\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("FLOAT2INT LF@%%retval LF@%%0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 7. strLength(value: String): Int ---
    printf("\nLABEL $strLength\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("STRLEN LF@%%retval LF@%%0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 8. substr(value: String, start: Int, end: Int): String ---
    printf("\nLABEL $substr\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("DEFVAR LF@len\n");
    printf("DEFVAR LF@cond\n");
    printf("DEFVAR LF@char\n");
    printf("STRLEN LF@len LF@%%0\n");
    // Kontrola: start < 0
    printf("LT LF@cond LF@%%1 int@0\n");
    printf("JUMPIFEQ $substr_err LF@cond bool@true\n");
    // Kontrola: end < 0
    printf("LT LF@cond LF@%%2 int@0\n");
    printf("JUMPIFEQ $substr_err LF@cond bool@true\n");
    // Kontrola: start > end
    printf("GT LF@cond LF@%%1 LF@%%2\n");
    printf("JUMPIFEQ $substr_err LF@cond bool@true\n");
    // Kontrola: start >= len (tj. !(start < len))
    printf("LT LF@cond LF@%%1 LF@len\n");
    printf("JUMPIFEQ $substr_err LF@cond bool@false\n");
    // Kontrola: end > len
    printf("GT LF@cond LF@%%2 LF@len\n");
    printf("JUMPIFEQ $substr_err LF@cond bool@true\n");

    printf("MOVE LF@%%retval string@\n");
    printf("LABEL $substr_loop\n");
    printf("EQ LF@cond LF@%%1 LF@%%2\n");
    printf("JUMPIFEQ $substr_end LF@cond bool@true\n");
    printf("GETCHAR LF@char LF@%%0 LF@%%1\n");
    printf("CONCAT LF@%%retval LF@%%retval LF@char\n");
    printf("ADD LF@%%1 LF@%%1 int@1\n");
    printf("JUMP $substr_loop\n");

    printf("LABEL $substr_err\n");
    printf("EXIT int@40\n");
    printf("LABEL $substr_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 9. ord(value: String, index: Int): Int ---
    printf("\nLABEL $ord\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("DEFVAR LF@len\n");
    printf("DEFVAR LF@cond\n");
    printf("STRLEN LF@len LF@%%0\n");
    // Kontrola: index < 0
    printf("LT LF@cond LF@%%1 int@0\n");
    printf("JUMPIFEQ $ord_err LF@cond bool@true\n");
    // Kontrola: index >= len
    printf("LT LF@cond LF@%%1 LF@len\n");
    printf("JUMPIFEQ $ord_err LF@cond bool@false\n");
    printf("STRI2INT LF@%%retval LF@%%0 LF@%%1\n");
    printf("JUMP $ord_end\n");
    printf("LABEL $ord_err\n");
    printf("MOVE LF@%%retval int@-1\n");
    printf("LABEL $ord_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // --- 10. chr(code: Int): String ---
    printf("\nLABEL $chr\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("INT2CHAR LF@%%retval LF@%%0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
}

void generate_target_code(IRNode *ir_head) {
    gen_header();

    IRNode *current = ir_head;

    while (current != NULL) {
        switch (current->opcode) {
        case OP_DEFVAR:
            emit_1("DEFVAR", current->dest);
            break;
        case OP_MOVE:
            emit_2("MOVE", current->dest, current->operand1);
            break;
        case OP_ADD:
            emit_3("ADD", current->dest, current->operand1, current->operand2);
            break;
        case OP_SUB:
            emit_3("SUB", current->dest, current->operand1, current->operand2);
            break;
        case OP_MUL:
            emit_3("MUL", current->dest, current->operand1, current->operand2);
            break;
        case OP_IDIV:
            emit_3("IDIV", current->dest, current->operand1, current->operand2);
            break;
        case OP_DIV:
            emit_3("DIV", current->dest, current->operand1, current->operand2);
            break;
        case OP_AND:
            emit_3("AND", current->dest, current->operand1, current->operand2);
            break;
        case OP_OR:
            emit_3("OR", current->dest, current->operand1, current->operand2);
            break;
        case OP_NOT:
            emit_2("NOT", current->dest, current->operand1);
            break;
        case OP_INT2FLOAT:
            emit_2("INT2FLOAT", current->dest, current->operand1);
            break;
        case OP_FLOAT2INT:
            emit_2("FLOAT2INT", current->dest, current->operand1);
            break;
        case OP_EQ:
            emit_3("EQ", current->dest, current->operand1, current->operand2);
            break;
        case OP_GT:
            emit_3("GT", current->dest, current->operand1, current->operand2);
            break;
        case OP_LT:
            emit_3("LT", current->dest, current->operand1, current->operand2);
            break;
        case OP_WRITE:
            emit_1("WRITE", current->dest);
            break;
        case OP_LABEL:
            printf("LABEL ");
            print_label(current->dest);
            putchar('\n');
            break;
        case OP_JUMP:
            printf("JUMP ");
            print_label(current->dest);
            putchar('\n');
            break;
        case OP_JUMPIFEQ:
            printf("JUMPIFEQ ");
            print_label(current->dest);
            putchar(' ');
            print_operand(current->operand1);
            putchar(' ');
            print_operand(current->operand2);
            putchar('\n');
            break;
        case OP_JUMPIFNEQ:
            printf("JUMPIFNEQ ");
            print_label(current->dest);
            putchar(' ');
            print_operand(current->operand1);
            putchar(' ');
            print_operand(current->operand2);
            putchar('\n');
            break;
        case OP_CREATEFRAME:
            printf("CREATEFRAME\n");
            break;
        case OP_PUSHFRAME:
            printf("PUSHFRAME\n");
            break;
        case OP_POPFRAME:
            printf("POPFRAME\n");
            break;
        case OP_RETURN:
            printf("RETURN\n");
            break;
        case OP_CALL:
            emit_1("CALL", current->dest);
            break;
        case OP_READ:
            emit_2("READ", current->dest, current->operand1);
            break;
        case OP_TYPE:
            emit_2("TYPE", current->dest, current->operand1);
            break;
        case OP_CONCAT:
            emit_3("CONCAT", current->dest, current->operand1, current->operand2);
            break;
        case OP_STRLEN:
            emit_2("STRLEN", current->dest, current->operand1);
            break;
        case OP_GETCHAR:
            emit_3("GETCHAR", current->dest, current->operand1, current->operand2);
            break;
        case OP_SETCHAR:
            emit_3("SETCHAR", current->dest, current->operand1, current->operand2);
            break;
        case OP_EXIT:
            emit_1("EXIT", current->dest);
            break;
        default:
            fprintf(stderr, "codegen warning: unknown or unhandled opcode (%d)\n",
                    (int)current->opcode);
            break;
        }
        current = current->next;
    }
    generate_builtins();
}

char *generate_unique_label(void) {
    static int counter = 0;

    char *label_name = malloc(32);
    if (label_name == NULL) {
        return NULL;
    }

    snprintf(label_name, 32, "label_%d", counter);

    counter++;
    return label_name;
}

void free_ir_list(IRNode *head) {
    IRNode *current = head;
    while (current != NULL) {
        IRNode *next_node = current->next;

        if (current->dest != NULL) {
            free(current->dest);
        }
        if (current->operand1 != NULL) {
            free(current->operand1);
        }
        if (current->operand2 != NULL) {
            free(current->operand2);
        }

        free(current);
        current = next_node;
    }
}