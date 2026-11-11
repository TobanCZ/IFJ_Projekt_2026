/*
    Implementace překladače imperativního jazyka IFJ26.
*/


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"



// Line number of the character being read.
static int current_line = 1;

// Number of currently open parentheses.
static int open_paren_depth = 0;

// Logical line has no token yet.
static bool line_is_empty = true;


// Prints a lexical error to stderr and exits with code 1.
static Token lexical_error(int line, int c)
{
    fprintf(stderr, "Lexical error, line %d: unexpected character: '%c'\n", line, c);
    exit(1);
}



// Returns a token with the given type and line.
static Token make_token(TokenType type, int line)
{
    Token token = { type, NULL, line };
    return token;
}



// Takes the next character if it is expected, returns 1 if it matched, 0 if not.
static int consume_if_next(int expected)
{
    int next = getchar();
    if (next == expected)
    {
        return 1;
    }
    if (next != EOF)
    {
        ungetc(next, stdin);
    }
    return 0;
}



// Returns the token for a single character.
static Token single_character_token(int character, int line)
{
    switch (character)
    {
        case '(':
            return make_token(TOKEN_PUNCTUATION_LEFT_PAREN, line);
        case ')':
            return make_token(TOKEN_PUNCTUATION_RIGHT_PAREN, line);
        case ',':
            return make_token(TOKEN_PUNCTUATION_COMMA, line);
        case ':':
            return make_token(TOKEN_PUNCTUATION_COLON, line);
        case '.':
            return make_token(TOKEN_PUNCTUATION_DOT, line);
        case '@':
            return make_token(TOKEN_PUNCTUATION_AT, line);
        case ';':
            return make_token(TOKEN_PUNCTUATION_SEMICOLON, line);
        case '+':
            return make_token(TOKEN_OPERATOR_PLUS, line);
        case '-':
            return make_token(TOKEN_OPERATOR_MINUS, line);
        case '*':
            return make_token(TOKEN_OPERATOR_MULTIPLY, line);
        case '/':
            return make_token(TOKEN_OPERATOR_DIVIDE, line);
        default:
            return lexical_error(line, character);
    }
}



// Returns the token for an operator and potential second character.
static Token operator_token(int character, int line)
{
    switch (character)
    {
        case '=':
            if (consume_if_next('='))
            {
                return make_token(TOKEN_OPERATOR_EQUAL, line);
            }
            return make_token(TOKEN_OPERATOR_ASSIGN, line);
        case '<':
            if (consume_if_next('='))
            {
                return make_token(TOKEN_OPERATOR_LESS_EQUAL, line);
            }
            return make_token(TOKEN_OPERATOR_LESS, line);
        case '>':
            if (consume_if_next('='))
            {
                return make_token(TOKEN_OPERATOR_GREATER_EQUAL, line);
            }
            return make_token(TOKEN_OPERATOR_GREATER, line);
        case '!':
            if (consume_if_next('='))
            {
                return make_token(TOKEN_OPERATOR_NOT_EQUAL, line);
            }
            return lexical_error(line, character);
        default:
            return single_character_token(character, line);
    }
}



// Makes a token from a character that is not a space or a new line.
static Token read_token(int character)
{
    Token new_token;

    line_is_empty = false;
    new_token = operator_token(character, current_line);

    if (new_token.type == TOKEN_PUNCTUATION_LEFT_PAREN)
    {
        open_paren_depth++;
    }
    else if (new_token.type == TOKEN_PUNCTUATION_RIGHT_PAREN && open_paren_depth > 0)
    {
        open_paren_depth--;
    }
    return new_token;
}



// Returns true if the line that just ended needs an EOL token.
static bool handle_new_line(void)
{
    bool ends_line = (open_paren_depth == 0 && !line_is_empty);

    current_line++;
    if (ends_line)
    {
        line_is_empty = true;
    }
    return ends_line;
}



Token get_token(void)
{
    int character;

    for (;;)
    {
        character = getchar();

        if (character == EOF)
        {
            if (!line_is_empty)
            {
                line_is_empty = true;
                return make_token(TOKEN_EOL, current_line);
            }
            return make_token(TOKEN_EOF, current_line);
        }

        if (character == ' ' || character == '\t')
        {
            continue;
        }

        if (character == '\n')
        {
            int line = current_line;

            if (handle_new_line())
            {
                return make_token(TOKEN_EOL, line);
            }
            continue;
        }

        return read_token(character);
    }
}