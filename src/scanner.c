/*
    Implementace překladače imperativního jazyka IFJ26.
*/


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include <ctype.h>
#include <string.h>


// Maximum number of levels.
#define MAX_DEPTH 256

// Stack of levels.
static int indent_levels[MAX_DEPTH] = { 0 };

// Number of levels.
static int current_depth = 1;

// Tokens waiting for return.
static int block_end_count = 0;

// True when a token is waiting for return.
static bool block_start = false;

// Line number of the character being read.
static int current_line = 1;

// Number of currently open parentheses.
static int open_paren_depth = 0;

// Logical line has no token yet.
static bool line_is_empty = true;

// Logical line spaces count.
static int line_start_spaces = 0;

// Tab at the start of the logical line.
static bool line_start_has_tab = false;

// Longest possible keyword.
#define MAX_WORD_LENGTH 256



// Types for keywords.
static const struct
{
    const char *word;
    TokenType type;
} keywords[] = 
{
    { "def", TOKEN_KW_DEF },
    { "val", TOKEN_KW_VAL },
    { "var", TOKEN_KW_VAR },
    { "if", TOKEN_KW_IF },
    { "then", TOKEN_KW_THEN },
    { "else", TOKEN_KW_ELSE },
    { "while", TOKEN_KW_WHILE },
    { "do", TOKEN_KW_DO },
    { "return", TOKEN_KW_RETURN },
    { "import", TOKEN_KW_IMPORT },
    { "Int", TOKEN_KW_INT },
    { "Double", TOKEN_KW_DOUBLE },
    { "String", TOKEN_KW_STRING },
    { "Unit", TOKEN_KW_UNIT },
};

#define KEYWORD_COUNT (sizeof(keywords) / sizeof(keywords[0]))



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

    if (ends_line)
    {
        line_is_empty = true;
    }
    return ends_line;
}


// Skips a line comment.
static void skip_line_comment(void)
{
    int curr_char;

    for (curr_char = getchar(); curr_char != '\n' && curr_char != EOF; curr_char = getchar())
    {
    }
    if (curr_char == '\n')
    {
        ungetc(curr_char, stdin);
    }
}



// Skips a block comment and returns true if the comment has a new line.
static bool skip_block_comment(void)
{
    int comment_depth = 1;
    bool has_new_line = false;
    int starting_line = current_line;
    int curr_char;

    for (;;)
    {
        curr_char = getchar();

        if (curr_char == EOF)
        {
            fprintf(stderr, "Lexical error, line %d: unclosed comment\n", starting_line);
            exit(1);
        }

        if (curr_char == '/' && consume_if_next('*'))
        {
            comment_depth++;
        }
        else if (curr_char == '*' && consume_if_next('/'))
        {
            comment_depth--;
            if (comment_depth == 0)
            {
                break;
            }
        }
        else if (curr_char == '\n')
        {
            current_line++;
            has_new_line = true;
        }
    }

    return has_new_line;
}



// Checks the leading spaces of a new line against the open blocks.
static void check_block_level(int spaces)
{
    int top_level = indent_levels[current_depth - 1];

    if (spaces > top_level)
    {
        if (current_depth == MAX_DEPTH)
        {
            fprintf(stderr, "Lexical error, line %d: too many nested blocks\n", current_line);
            exit(1);
        }

        indent_levels[current_depth] = spaces;
        current_depth++;
        block_start = true;
        return;
    }

    for (; spaces < indent_levels[current_depth - 1]; current_depth--)
    {
        block_end_count++;
    }

    if (spaces != indent_levels[current_depth - 1])
    {
        fprintf(stderr, "Lexical error, line %d: invalid block level\n", current_line);
        exit(1);
    }
}



// Checks if the character can start a word.
static bool is_word_start(int character)
{
    return isalpha(character) || character == '_';
}



// Checks if the character can be inside a word.
static bool is_word_char(int character)
{
    return isalnum(character) || character == '_';
}



// Allocates memory for a word, exits with code 99 if it fails.
static char *allocate_word(void)
{
    char *word = malloc(MAX_WORD_LENGTH + 1);

    if (word == NULL)
    {
        fprintf(stderr, "Internal error: out of memory\n");
        exit(99);
    }
    return word;
}



// Reads the rest of a word that starts with first_character.
static char *read_word(int first_character)
{
    char *word = allocate_word();
    int length = 0;
    int character = first_character;

    for (; is_word_char(character); character = getchar())
    {
        if (length == MAX_WORD_LENGTH)
        {
            fprintf(stderr, "Lexical error, line %d: word is too long\n", current_line);
            exit(1);
        }
        word[length] = (char)character;
        length++;
    }

    ungetc(character, stdin);
    word[length] = '\0';
    return word;
}



// Returns the keyword or identifier.
static TokenType keyword_type(const char *word)
{
    for (size_t index = 0; index < KEYWORD_COUNT; index++)
    {
        if (strcmp(word, keywords[index].word) == 0)
        {
            return keywords[index].type;
        }
    }
    return TOKEN_IDENT;
}



// Makes a keyword or identifier token from a word.
static Token word_token(int first_character)
{
    char *word = read_word(first_character);
    TokenType type = keyword_type(word);
    Token token;

    line_is_empty = false;

    if (strcmp(word, "_") == 0)
    {
        fprintf(stderr, "Lexical error, line %d: invalid name\n", current_line);
        exit(1);
    }

    token = make_token(type, current_line);
    if (type == TOKEN_IDENT)
    {
        token.value = word;
    }
    else
    {
        free(word);
    }
    return token;
}



// Adds a character to the token text or exits if the text is too long.
static void append_char(char *token_text, int *text_length, int character_to_add)
{
    if (*text_length == MAX_WORD_LENGTH)
    {
        fprintf(stderr, "Lexical error, line %d: number is too long\n", current_line);
        exit(1);
    }
    token_text[*text_length] = (char)character_to_add;
    (*text_length)++;
}



// Reads digits and the _ character.
static int read_digits(char *token_text, int *text_length, int current_character)
{
    for (; isdigit(current_character) || current_character == '_'; current_character = getchar())
    {
        if (current_character == '_')
        {
            int character_after_underscore = getchar();
            if (!isdigit(character_after_underscore))
            {
                lexical_error(current_line, '_');
            }
            ungetc(character_after_underscore, stdin);
            continue;
        }
        append_char(token_text, text_length, current_character);
    }
    return current_character;
}



// Reads the part after the dot.
static int read_fraction(char *token_text, int *text_length)
{
    int character_after_dot;

    append_char(token_text, text_length, '.');
    character_after_dot = getchar();
    if (!isdigit(character_after_dot))
    {
        lexical_error(current_line, character_after_dot);
    }
    return read_digits(token_text, text_length, character_after_dot);
}



// Reads the exponent.
static int read_exponent(char *token_text, int *text_length, int current_character)
{
    append_char(token_text, text_length, current_character);
    current_character = getchar();
    if (current_character == '+' || current_character == '-')
    {
        append_char(token_text, text_length, current_character);
        current_character = getchar();
    }
    if (!isdigit(current_character))
    {
        lexical_error(current_line, current_character);
    }
    return read_digits(token_text, text_length, current_character);
}



// Makes the number token from the text and the type.
static Token finish_number_token(char *token_text, int text_length, TokenType token_type, int character_after_number)
{
    Token new_token;

    token_text[text_length] = '\0';
    ungetc(character_after_number, stdin);
    line_is_empty = false;
    new_token = make_token(token_type, current_line);
    new_token.value = token_text;
    return new_token;
}



// Reads the whole number and returns its token.
static Token number_token(int first_character)
{
    char *token_text = allocate_word();
    int text_length = 0;
    int current_character = read_digits(token_text, &text_length, first_character);
    TokenType token_type = TOKEN_INT;

    if (current_character == '.')
    {
        current_character = read_fraction(token_text, &text_length);
        token_type = TOKEN_DOUBLE;
    }
    if (current_character == 'e' || current_character == 'E')
    {
        current_character = read_exponent(token_text, &text_length, current_character);
        token_type = TOKEN_DOUBLE;
    }
    if (current_character == 'd' || current_character == 'D')
    {
        token_type = TOKEN_DOUBLE;
        current_character = getchar();
    }

    if (token_type == TOKEN_INT && first_character == '0' && text_length > 1)
    {
        lexical_error(current_line, '0');
    }

    return finish_number_token(token_text, text_length, token_type, current_character);
}



// Reads the character after a backslash and returns the real character.
static int read_escape_character(void)
{
    int character_after_backslash = getchar();

    switch (character_after_backslash)
    {
        case '\\':
            return '\\';
        case '"':
            return '"';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
    }
    lexical_error(current_line, character_after_backslash);
    return 0;
}



// Reads the string up to the second quote.
static void read_string_characters(char *string_text, int *string_length)
{
    int current_character;

    for (current_character = getchar(); current_character != '"'; current_character = getchar())
    {
        if (current_character == EOF || current_character == '\n')
        {
            fprintf(stderr, "Lexical error, line %d: unclosed string\n", current_line);
            exit(1);
        }
        if (current_character == '\\')
        {
            current_character = read_escape_character();
        }
        append_char(string_text, string_length, current_character);
    }
}



// Ends the text and makes the string token.
static Token finish_string_token(char *string_text, int string_length)
{
    Token new_token;

    string_text[string_length] = '\0';
    line_is_empty = false;
    new_token = make_token(TOKEN_STRING, current_line);
    new_token.value = string_text;
    return new_token;
}



// Reads a string and returns its token.
static Token make_string_token(void)
{
    char *string_text = allocate_word();
    int string_length = 0;

    read_string_characters(string_text, &string_length);
    return finish_string_token(string_text, string_length);
}



Token get_token(void)
{
    int character;

    for (;;)
    {
        if (block_end_count > 0)
        {
            block_end_count--;
            return make_token(TOKEN_BLOCK_END, current_line);
        }
        if (block_start)
        {
            block_start = false;
            return make_token(TOKEN_BLOCK_START, current_line);
        }

        character = getchar();


        if (character == EOF)
        {
            if (!line_is_empty)
            {
                line_is_empty = true;
                return make_token(TOKEN_EOL, current_line);
            }
            if (current_depth > 1)
            {
                block_end_count = current_depth - 1;
                current_depth = 1;
                continue;
            }
            return make_token(TOKEN_EOF, current_line);
        }


        if (character == ' ' || character == '\t')
        {
            if (line_is_empty && character == ' ')
            {
                line_start_spaces++;
            }
            if (line_is_empty && character == '\t')
            {
                line_start_has_tab = true;
            }
            continue;
        }


        if (character == '\n')
        {
            int line = current_line;

            current_line++;
            line_start_spaces = 0;
            line_start_has_tab = false;
            if (handle_new_line())
            {
                return make_token(TOKEN_EOL, line);
            }
            continue;
        }


        if (character == '/' && consume_if_next('/'))
        {
            skip_line_comment();
            continue;
        }


        if (character == '/' && consume_if_next('*'))
        {
            int line = current_line;

            if (skip_block_comment() && handle_new_line())
            {
                return make_token(TOKEN_EOL, line);
            }
            continue;
        }


        if (line_is_empty && open_paren_depth == 0)
        {
            if (line_start_has_tab)
            {
                fprintf(stderr, "Lexical error, line %d: tab at line start\n", current_line);
                exit(1);
            }
            check_block_level(line_start_spaces);
            line_is_empty = false;
            ungetc(character, stdin);
            continue;
        }

        
        if (is_word_start(character))
        {
            return word_token(character);
        }


        if (isdigit(character))
        {
            return number_token(character);
        }


        if (character == '"')
        {
            return make_string_token();
        }


        return read_token(character);
    }
}
