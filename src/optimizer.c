/**
 * Projekt: Překladač jazyka IFJ26 (IFJ/IAL)
 * Soubor: src/optimizer.c
 * Autoři: Jan Drcmánek (xdrcmaj00)
 */

#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Projde seznam IR instrukcí a vyhodnotí operace s konstantami při překladu (constant folding).
// Výslednou instrukci nahradí přesunem konstanty (OP_MOVE).
void optimizer(IRNode *ir_head) {
    IRNode *current = ir_head;

    while (current != NULL) {
        // 1. Celočíselná aritmetika
        if (current->opcode == OP_ADD || current->opcode == OP_SUB || current->opcode == OP_MUL ||
            current->opcode == OP_IDIV) {

            long long val1, val2;
            if (current->operand1 != NULL && current->operand2 != NULL &&
                sscanf(current->operand1, "int@%lld", &val1) == 1 &&
                sscanf(current->operand2, "int@%lld", &val2) == 1) {

                long long result = 0;
                int can_optimize = 1;

                if (current->opcode == OP_ADD) {
                    result = val1 + val2;
                } else if (current->opcode == OP_SUB) {
                    result = val1 - val2;
                } else if (current->opcode == OP_MUL) {
                    result = val1 * val2;
                } else if (current->opcode == OP_IDIV) {
                    if (val2 == 0) {
                        can_optimize = 0;
                    } else {
                        result = val1 / val2;
                    }
                }

                if (can_optimize) {
                    char *new_operand = malloc(64);
                    if (new_operand != NULL) {
                        snprintf(new_operand, 64, "int@%lld", result);

                        free(current->operand1);
                        free(current->operand2);

                        current->opcode   = OP_MOVE;
                        current->operand1 = new_operand;
                        current->operand2 = NULL;
                    }
                }
            }
        }
        // 2. Desetinné dělení, výsledek se ukládá v hexadecimálním formátu
        else if (current->opcode == OP_DIV) {
            double f1, f2;
            if (current->operand1 != NULL && current->operand2 != NULL &&
                sscanf(current->operand1, "float@%lf", &f1) == 1 &&
                sscanf(current->operand2, "float@%lf", &f2) == 1 && f2 != 0.0) {

                char *new_operand = malloc(64);
                if (new_operand != NULL) {
                    snprintf(new_operand, 64, "float@%a", f1 / f2);

                    free(current->operand1);
                    free(current->operand2);

                    current->opcode   = OP_MOVE;
                    current->operand1 = new_operand;
                    current->operand2 = NULL;
                }
            }
        }
        // 3. Porovnávání celých čísel (<, >, ==)
        else if (current->opcode == OP_LT || current->opcode == OP_GT || current->opcode == OP_EQ) {

            long long val1, val2;
            if (current->operand1 != NULL && current->operand2 != NULL &&
                sscanf(current->operand1, "int@%lld", &val1) == 1 &&
                sscanf(current->operand2, "int@%lld", &val2) == 1) {

                int result = 0;
                if (current->opcode == OP_LT) {
                    result = (val1 < val2);
                } else if (current->opcode == OP_GT) {
                    result = (val1 > val2);
                } else if (current->opcode == OP_EQ) {
                    result = (val1 == val2);
                }

                char *new_operand = malloc(32);
                if (new_operand != NULL) {
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
        }
        // 4. Logické operace (AND, OR) nad konstantami bool@
        else if (current->opcode == OP_AND || current->opcode == OP_OR) {
            if (current->operand1 != NULL && current->operand2 != NULL) {
                int b1 = -1, b2 = -1;

                if (strcmp(current->operand1, "bool@true") == 0)
                    b1 = 1;
                else if (strcmp(current->operand1, "bool@false") == 0)
                    b1 = 0;

                if (strcmp(current->operand2, "bool@true") == 0)
                    b2 = 1;
                else if (strcmp(current->operand2, "bool@false") == 0)
                    b2 = 0;

                if (b1 != -1 && b2 != -1) {
                    int result        = (current->opcode == OP_AND) ? (b1 && b2) : (b1 || b2);
                    char *new_operand = malloc(32);
                    if (new_operand != NULL) {
                        strcpy(new_operand, result ? "bool@true" : "bool@false");

                        free(current->operand1);
                        free(current->operand2);

                        current->opcode   = OP_MOVE;
                        current->operand1 = new_operand;
                        current->operand2 = NULL;
                    }
                }
            }
        }
        // 5. Logická negace (NOT) nad konstantou bool@
        else if (current->opcode == OP_NOT) {
            if (current->operand1 != NULL) {
                int is_true  = (strcmp(current->operand1, "bool@true") == 0);
                int is_false = (strcmp(current->operand1, "bool@false") == 0);

                if (is_true || is_false) {
                    char *new_operand = malloc(32);
                    if (new_operand != NULL) {
                        strcpy(new_operand, is_true ? "bool@false" : "bool@true");

                        free(current->operand1);
                        free(current->operand2);

                        current->opcode   = OP_MOVE;
                        current->operand1 = new_operand;
                        current->operand2 = NULL;
                    }
                }
            }
        }
        // 6. Převody konstant mezi typy Int (int@) a Double (float@)
        else if (current->opcode == OP_INT2FLOAT) {
            long long val;
            if (current->operand1 != NULL && sscanf(current->operand1, "int@%lld", &val) == 1) {
                char *new_operand = malloc(64);
                if (new_operand != NULL) {
                    snprintf(new_operand, 64, "float@%a", (double)val);

                    free(current->operand1);
                    free(current->operand2);

                    current->opcode   = OP_MOVE;
                    current->operand1 = new_operand;
                    current->operand2 = NULL;
                }
            }
        } else if (current->opcode == OP_FLOAT2INT) {
            double val;
            if (current->operand1 != NULL && sscanf(current->operand1, "float@%lf", &val) == 1) {
                char *new_operand = malloc(64);
                if (new_operand != NULL) {
                    snprintf(new_operand, 64, "int@%lld", (long long)val);

                    free(current->operand1);
                    free(current->operand2);

                    current->opcode   = OP_MOVE;
                    current->operand1 = new_operand;
                    current->operand2 = NULL;
                }
            }
        }
        // 7. Výpočet délky konstantního řetězce (přeskočí prefix "string@")
        else if (current->opcode == OP_STRLEN) {
            if (current->operand1 != NULL && strncmp(current->operand1, "string@", 7) == 0) {
                size_t length     = strlen(current->operand1 + 7);
                char *new_operand = malloc(64);
                if (new_operand != NULL) {
                    snprintf(new_operand, 64, "int@%zu", length);

                    free(current->operand1);
                    free(current->operand2);

                    current->opcode   = OP_MOVE;
                    current->operand1 = new_operand;
                    current->operand2 = NULL;
                }
            }
        }

        current = current->next;
    }
}

// Odstraní nedosažitelné instrukce (dead code) za nepodmíněným skokem, návratem z funkce
// nebo ukončením programu až po nejbližší label (OP_LABEL).
void eliminate_dead_code(IRNode *ir_head) {
    IRNode *current = ir_head;

    while (current != NULL) {
        if (current->opcode == OP_JUMP || current->opcode == OP_RETURN ||
            current->opcode == OP_EXIT) {

            IRNode *dead_node = current->next;

            // Maže a uvolňuje všechny následující uzly, dokud nenarazí na další label.
            while (dead_node != NULL && dead_node->opcode != OP_LABEL) {
                IRNode *next_dead = dead_node->next;

                free(dead_node->dest);
                free(dead_node->operand1);
                free(dead_node->operand2);
                free(dead_node);

                dead_node = next_dead;
            }
            // Napojíme seznam zpět na nalezený label (nebo NULL na konci seznamu).
            current->next = dead_node;
        }
        current = current->next;
    }
}