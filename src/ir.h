typedef enum {
    OP_DEFVAR,
    OP_MOVE,
    OP_MUL,
    OP_ADD,
    OP_SUB,
    OP_IDIV,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_INT2FLOAT,
    OP_FLOAT2INT,
    OP_EQ,
    OP_GT,
    OP_LT,
    OP_LABEL,
    OP_JUMP,
    OP_JUMPIFEQ,
    OP_JUMPIFNEQ,
    OP_CREATEFRAME,
    OP_PUSHFRAME,
    OP_POPFRAME,
    OP_CALL,
    OP_RETURN,
    OP_READ,
    OP_TYPE,
    OP_CONCAT,
    OP_STRLEN,
    OP_GETCHAR,
    OP_SETCHAR,
    OP_EXIT,
    OP_WRITE
} Opcode;

typedef struct IRNode {
    Opcode opcode;
    char *dest;
    char *operand1;
    char *operand2;
    struct IRNode *next;
} IRNode;

void generate_target_code(IRNode *ir_head);
void optimizer(IRNode *ir_head);
char *generate_unique_label(void);
void free_ir_list(IRNode *head);
