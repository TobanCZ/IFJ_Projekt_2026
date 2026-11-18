#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void optimizer(IRNode *ir_head) {

    IRNode *current = ir_head;

    while (current != NULL) {
        if (current->opcode == OP_ADD || current->opcode == OP_SUB || current->opcode == OP_MUL ||
            current->opcode == OP_IDIV) {

            int val1, val2;

            if (current->operand1 != NULL && current->operand2 != NULL &&
                sscanf(current->operand1, "int@%d", &val1) == 1 &&
                sscanf(current->operand2, "int@%d", &val2) == 1) {

                int result       = 0;
                int can_optimize = 1;

                switch (current->opcode) {
                case OP_ADD:
                    result = val1 + val2;
                    break;
                case OP_SUB:
                    result = val1 - val2;
                    break;
                case OP_MUL:
                    result = val1 * val2;
                    break;
                case OP_IDIV:
                    if (val2 == 0) {
                        can_optimize = 0;
                    } else {
                        result = val1 / val2;
                    }
                    break;
                default:
                    break;
                }
                if (can_optimize) {
                    char *new_operand = malloc(32);
                    sprintf(new_operand, "int@%d", result);

                    free(current->operand1);
                    free(current->operand2);

                    current->opcode   = OP_MOVE;
                    current->operand1 = new_operand;
                    current->operand2 = NULL;
                }
            }
        } else if (current->opcode == OP_LT || current->opcode == OP_GT ||
                   current->opcode == OP_EQ) {

            int val1, val2;
            if (current->operand1 != NULL && current->operand2 != NULL &&
                sscanf(current->operand1, "int@%d", &val1) == 1 &&
                sscanf(current->operand2, "int@%d", &val2) == 1) {

                int result = 0;

                switch (current->opcode) {
                case OP_LT:
                    result = (val1 < val2);
                    break;
                case OP_GT:
                    result = (val1 > val2);
                    break;
                case OP_EQ:
                    result = (val1 == val2);
                    break;
                default:
                    break;
                }
                char *new_operand = malloc(32);

                if (result == 1) {
                    strcpy(new_operand, "bool@true");
                } else {
                    strcpy(new_operand, "bool@false");
                }

                free(current->operand1);
                free(current->operand2);

                current->opcode   = OP_MOVE;
                current->operand1 = new_operand;
                current->operand2 = NULL;
            }
        }

        else if (current->opcode == OP_STRLEN) {
            if (current->operand1 != NULL && strncmp(current->operand1, "string@", 7) == 0) {

                const char *str_val = current->operand1 + 7;

                int length = strlen(str_val);

                char *new_operand = malloc(32);
                sprintf(new_operand, "int@%d", length);

                free(current->operand1);

                current->opcode   = OP_MOVE;
                current->operand1 = new_operand;
            }
        }
        current = current->next;
    }
}
