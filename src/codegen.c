#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"



void gen_header() {
    printf(".IFJcode26\n");
}


void generate_target_code (IRNode *ir_head){
    gen_header();

    IRNode *current = ir_head;
    
    while (current != NULL) {
        switch(current->opcode){
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
        current=current->next;
    }
}

char* generate_unique_label() {
    static int counter = 0; 
    
    char *label_name = malloc(32);
    sprintf(label_name, "label_%d", counter);
    
    counter++;
    return label_name;
}


void free_ir_list(IRNode *head) {
    IRNode *current = head;
    while (current != NULL){
        IRNode *next_node = current->next;



        free(current);
        current = next_node;
    }
}