/*
    Implementace překladače imperativního jazyka IFJ26.
*/

#ifndef TOKEN_H
#define TOKEN_H

// Types of tokens made by the scanner.
typedef enum {
    // Identifiers and literals.
    TOKEN_IDENT,
    TOKEN_INT,
    TOKEN_DOUBLE,
    TOKEN_STRING,

    // Keywords.
    TOKEN_KW_DEF,
    TOKEN_KW_VAL,
    TOKEN_KW_VAR,
    TOKEN_KW_IF,
    TOKEN_KW_THEN,
    TOKEN_KW_ELSE,
    TOKEN_KW_WHILE,
    TOKEN_KW_DO,
    TOKEN_KW_RETURN,
    TOKEN_KW_IMPORT,
    TOKEN_KW_INT,
    TOKEN_KW_DOUBLE,
    TOKEN_KW_STRING,
    TOKEN_KW_UNIT,

    // Operators.
    TOKEN_OPERATOR_ASSIGN,
    TOKEN_OPERATOR_PLUS,
    TOKEN_OPERATOR_MINUS,
    TOKEN_OPERATOR_MULTIPLY,
    TOKEN_OPERATOR_DIVIDE,
    TOKEN_OPERATOR_LESS,
    TOKEN_OPERATOR_LESS_EQUAL,
    TOKEN_OPERATOR_GREATER,
    TOKEN_OPERATOR_GREATER_EQUAL,
    TOKEN_OPERATOR_EQUAL,
    TOKEN_OPERATOR_NOT_EQUAL,

    // Punctuation.
    TOKEN_PUNCTUATION_LEFT_PAREN,
    TOKEN_PUNCTUATION_RIGHT_PAREN,
    TOKEN_PUNCTUATION_COMMA,
    TOKEN_PUNCTUATION_COLON,
    TOKEN_PUNCTUATION_DOT,
    TOKEN_PUNCTUATION_AT,
    TOKEN_PUNCTUATION_SEMICOLON,

    // End of line and blocks.
    TOKEN_EOL,
    TOKEN_BLOCK_START,
    TOKEN_BLOCK_END,

    // Errors.
    TOKEN_LEX_ERROR,
    TOKEN_SYNTAX_ERROR,
    TOKEN_INTERNAL_ERROR,

    // End of input.
    TOKEN_EOF
} TokenType;

// Type, value, line, of token.
typedef struct {
    TokenType type;
    char *value;
    int line;
} Token;

#endif
