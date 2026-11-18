#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gen_header() {
    printf(".IFJcode26\n");
}

void generate_builtins() {

    printf("LABEL $length\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("STRLEN LF@%%retval LF@%%0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    printf("LABEL $chr\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("INT2CHAR LF@%%retval LF@%%0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    printf("LABEL $ord\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("STRI2INT LF@%%retval LF@%%0 LF@%%1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    printf("LABEL $readInt\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("READ LF@%%retval int\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    printf("LABEL $readDouble\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("READ LF@%%retval float\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    printf("LABEL $readString\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("READ LF@%%retval string\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    printf("LABEL $substring\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@%%retval\n");
    printf("MOVE LF@%%retval string@\n");
    printf("DEFVAR LF@cmp\n");
    printf("DEFVAR LF@char\n");
    printf("LABEL $substring_loop\n");
    printf("EQ LF@cmp LF@%%1 LF@%%2\n");
    printf("JUMPIFEQ $substring_end LF@cmp bool@true\n");
    printf("GETCHAR LF@char LF@%%0 LF@%%1\n");
    printf("CONCAT LF@%%retval LF@%%retval LF@char\n");
    printf("ADD LF@%%1 LF@%%1 int@1\n");
    printf("JUMP $substring_loop\n");
    printf("LABEL $substring_end\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
}

char *escape_ifj_string(char *original) {

    if (original == NULL || strncmp(original, "string@", 7) != 0) {
        return original;
    }

    char *escaped = malloc(strlen(original) * 4 + 1);

    strcpy(escaped, "string@");

    int j = 7;

    for (int i = 7; original[i] != '\0'; i++) {

        unsigned char c = (unsigned char)original[i];

        if (c <= 32 || c == 35 || c == 92) {
            sprintf(escaped + j, "\\%03d", c);
            j += 4;
        } else {
            escaped[j++] = c;
        }
    }
    escaped[j] = '\0';

    free(original);
    return escaped;
}

void generate_target_code(IRNode *ir_head) {
    gen_header();

    IRNode *current = ir_head;

    while (current != NULL) {
        current->dest     = escape_ifj_string(current->dest);
        current->operand1 = escape_ifj_string(current->operand1);
        current->operand2 = escape_ifj_string(current->operand2);

        switch (current->opcode) {
        case OP_DEFVAR:
            printf("DEFVAR %s\n", current->dest);
            break;
        case OP_MOVE:
            printf("MOVE %s %s\n", current->dest, current->operand1);
            break;
        case OP_ADD:
            printf("ADD %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_SUB:
            printf("SUB %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_MUL:
            printf("MUL %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_IDIV:
            printf("IDIV %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_EQ:
            printf("EQ %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_GT:
            printf("GT %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_LT:
            printf("LT %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_WRITE:
            printf("WRITE %s\n", current->dest);
            break;
        case OP_LABEL:
            printf("LABEL %s\n", current->dest);
            break;
        case OP_JUMP:
            printf("JUMP %s\n", current->dest);
            break;
        case OP_JUMPIFEQ:
            printf("JUMPIFEQ %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_JUMPIFNEQ:
            printf("JUMPIFNEQ %s %s %s\n", current->dest, current->operand1, current->operand2);
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
            printf("CALL %s\n", current->dest);
            break;
        case OP_READ:
            printf("READ %s %s\n", current->dest, current->operand1);
            break;
        case OP_TYPE:
            printf("TYPE %s %s\n", current->dest, current->operand1);
            break;
        case OP_CONCAT:
            printf("CONCAT %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_STRLEN:
            printf("STRLEN %s %s\n", current->dest, current->operand1);
            break;
        case OP_GETCHAR:
            printf("GETCHAR %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_SETCHAR:
            printf("SETCHAR %s %s %s\n", current->dest, current->operand1, current->operand2);
            break;
        case OP_EXIT:
            printf("EXIT %s\n", current->dest);
            break;
        default:
            break;
        }
        current = current->next;
    }
    generate_builtins();
}

char *generate_unique_label() {
    static int counter = 0;

    char *label_name = malloc(32);
    sprintf(label_name, "label_%d", counter);

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