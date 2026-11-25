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

// Line where the current string starts.
static int string_start_line = 1;

// Number of currently open parentheses.
static int open_paren_depth = 0;

// Logical line has no token yet.
static bool line_is_empty = true;

// Logical line spaces count.
static int line_start_spaces = 0;

// Tab at the start of the logical line.
static bool line_start_has_tab = false;

// Spaces at the start of the line.
static int logical_line_spaces = 0;

// True when the next character is the first on a line.
static bool is_first_char_on_line = true;

// True when only spaces or a line end next.
static bool line_end_required = false;

// True when the next token is a divide.
static bool divide_is_next = false;

// True when the main is the next token after @.
static bool main_is_next = false;

// Starting size of the text of a token.
#define INITIAL_TOKEN_TEXT_CAPACITY 16

// Text of a token, its length and capacity.
typedef struct
{
    char *text;
    int length;
    int capacity;
} TokenText;



// States of the scanner.
typedef enum
{
    STATE_START,
    STATE_EQUAL,
    STATE_LESS,
    STATE_GREATER,
    STATE_EXCLAMATION,
    STATE_WORD,
    STATE_ZERO,
    STATE_ZERO_UNDERSCORE,
    STATE_ZERO_DIGITS,
    STATE_ZERO_DIGITS_UNDERSCORE,
    STATE_INT,
    STATE_INT_UNDERSCORE,
    STATE_DOT,
    STATE_FRACTION,
    STATE_FRACTION_UNDERSCORE,
    STATE_EXP,
    STATE_EXP_SIGN,
    STATE_EXP_DIGITS,
    STATE_EXP_UNDERSCORE,
    STATE_NUMBER_SUFFIX,
    STATE_STRING,
    STATE_STRING_ESCAPE,
    STATE_QUOTE,
    STATE_QUOTE2,
    STATE_TRIPLE,
    STATE_TRIPLE_QUOTE1,
    STATE_TRIPLE_QUOTE2,
    STATE_TRIPLE_QUOTE3,
    STATE_SLASH,
    STATE_LINE_COMMENT,
    STATE_BLOCK_COMMENT,
    STATE_BLOCK_STAR,
    STATE_BLOCK_SLASH,
    STATE_DOT_START,
    STATE_ANNOTATION,
    STATE_ANNOTATION_M,
    STATE_ANNOTATION_MA,
    STATE_ANNOTATION_MAI,
    STATE_ANNOTATION_MAIN,
    STATE_ZERO_NONZERO,
    STATE_ZERO_NONZERO_UNDERSCORE
} ScannerState;



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
        default:
            return lexical_error(line, character);
    }
}



// Makes a token from a character that is not a space or a new line.
static Token read_token(int character)
{
    Token new_token;

    line_is_empty = false;
    new_token = single_character_token(character, current_line);

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



// Checks the indentation of a line that has its first token.
static void check_line_start(void)
{
    if (line_start_has_tab)
    {
        fprintf(stderr, "Lexical error, line %d: tab at line start\n", current_line);
        exit(1);
    }
    logical_line_spaces = line_start_spaces;
    check_block_level(line_start_spaces);
    line_is_empty = false;
}



// Checks a continuation line inside parentheses.
static void check_line_continuation(void)
{
    if (line_start_has_tab || line_start_spaces <= logical_line_spaces)
    {
        fprintf(stderr, "Lexical error, line %d: continuation line is not indented\n", current_line);
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



// Starts an empty text for a token, exits with code 99 if it fails.
static void allocate_word(TokenText *token_text)
{
    token_text->text = malloc(INITIAL_TOKEN_TEXT_CAPACITY);
    if (token_text->text == NULL)
    {
        fprintf(stderr, "Internal error: out of memory\n");
        exit(99);
    }
    token_text->text[0] = '\0';
    token_text->length = 0;
    token_text->capacity = INITIAL_TOKEN_TEXT_CAPACITY;
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



// Makes a keyword or identifier token from the word.
static Token make_word_token(char *word)
{
    TokenType type = keyword_type(word);
    Token token;

    line_is_empty = false;

    if (strcmp(word, "_") == 0)
    {
        fprintf(stderr, "Lexical error, line %d: identifier '_' is not allowed\n", current_line);
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



// Adds a character to the end of the token text, the text grows when it is full.
static void append_char(TokenText *token_text, int character_to_add)
{
    char *new_text;
    int new_capacity;

    if (token_text->length + 2 > token_text->capacity)
    {
        new_capacity = token_text->capacity * 2;
        new_text = realloc(token_text->text, new_capacity);
        if (new_text == NULL)
        {
            fprintf(stderr, "Internal error: out of memory\n");
            exit(99);
        }
        token_text->text = new_text;
        token_text->capacity = new_capacity;
    }
    token_text->text[token_text->length] = (char)character_to_add;
    token_text->length++;
    token_text->text[token_text->length] = '\0';
}



// Makes the number token from the text and its type.
static Token make_number_token(TokenText *number, TokenType token_type)
{
    Token token;

    token = make_token(token_type, current_line);
    token.value = number->text;
    return token;
}



// Ends the number, or moves to the exponent or the suffix.
static bool number_end_step(int character, ScannerState *current_state, TokenText *number, TokenType token_type, Token *result_token)
{
    if (character == 'e' || character == 'E')
    {
        append_char(number, character);
        *current_state = STATE_EXP;
        return false;
    }
    if (character == 'd' || character == 'D')
    {
        if (token_type == TOKEN_INT)
        {
            lexical_error(current_line, character);
        }
        *current_state = STATE_NUMBER_SUFFIX;
        return false;
    }
    ungetc(character, stdin);
    *result_token = make_number_token(number, token_type);
    return true;
}



// Moves to the fraction on a dot or ends the integer.
static bool integer_end_step(int character, ScannerState *current_state, TokenText *number, Token *result_token)
{
    if (character == '.')
    {
        append_char(number, '.');
        *current_state = STATE_DOT;
        return false;
    }
    return number_end_step(character, current_state, number, TOKEN_INT, result_token);
}



// Checks that a digit follows an underscore, then adds it.
static void add_digit_after_underscore(int character, TokenText *number)
{
    if (!isdigit(character))
    {
        lexical_error(current_line, '_');
    }
    append_char(number, character);
}



// Reads the integer part of a number, one character at the time.
static bool integer_state_step(int character, ScannerState *current_state, TokenText *number, Token *result_token)
{
    switch (*current_state)
    {
        case STATE_ZERO:
            if (character == '_')
            {
                *current_state = STATE_ZERO_UNDERSCORE;
                return false;
            }
            if (isdigit(character))
            {
                append_char(number, character);
                *current_state = (character == '0') ? STATE_ZERO_DIGITS : STATE_ZERO_NONZERO;
                return false;
            }
            return integer_end_step(character, current_state, number, result_token);

        case STATE_ZERO_UNDERSCORE:
            if (character == '_')
            {
                return false;
            }
            add_digit_after_underscore(character, number);
            *current_state = (character == '0') ? STATE_ZERO_DIGITS : STATE_ZERO_NONZERO;
            return false;

        case STATE_ZERO_DIGITS:
            if (isdigit(character))
            {
                append_char(number, character);
                if (character != '0')
                {
                    *current_state = STATE_ZERO_NONZERO;
                }
                return false;
            }
            if (character == '_')
            {
                *current_state = STATE_ZERO_DIGITS_UNDERSCORE;
                return false;
            }
            if (character == '.')
            {
                append_char(number, '.');
                *current_state = STATE_DOT;
                return false;
            }
            if (character == 'e' || character == 'E')
            {
                return number_end_step(character, current_state, number, TOKEN_DOUBLE, result_token);
            }
            return integer_end_step(character, current_state, number, result_token);

        case STATE_ZERO_DIGITS_UNDERSCORE:
            if (character == '_')
            {
                return false;
            }
            add_digit_after_underscore(character, number);
            *current_state = (character == '0') ? STATE_ZERO_DIGITS : STATE_ZERO_NONZERO;
            return false;

        case STATE_ZERO_NONZERO:
            if (isdigit(character))
            {
                append_char(number, character);
                return false;
            }
            if (character == '_')
            {
                *current_state = STATE_ZERO_NONZERO_UNDERSCORE;
                return false;
            }
            if (character == '.')
            {
                append_char(number, '.');
                *current_state = STATE_DOT;
                return false;
            }
            if (character == 'e' || character == 'E')
            {
                return number_end_step(character, current_state, number, TOKEN_DOUBLE, result_token);
            }
            lexical_error(current_line, character);
            return false;

        case STATE_ZERO_NONZERO_UNDERSCORE:
            if (character == '_')
            {
                return false;
            }
            add_digit_after_underscore(character, number);
            *current_state = STATE_ZERO_NONZERO;
            return false;

        case STATE_INT:
            if (isdigit(character))
            {
                append_char(number, character);
                return false;
            }
            if (character == '_')
            {
                *current_state = STATE_INT_UNDERSCORE;
                return false;
            }
            return integer_end_step(character, current_state, number, result_token);

        case STATE_INT_UNDERSCORE:
            if (character == '_')
            {
                return false;
            }
            add_digit_after_underscore(character, number);
            *current_state = STATE_INT;
            return false;

        default:
            return false;
    }
}



// Reads the fraction part of a number, one character at a time.
static bool fraction_state_step(int character, ScannerState *current_state, TokenText *number, Token *result_token)
{
    switch (*current_state)
    {
        case STATE_DOT:
            if (!isdigit(character))
            {
                lexical_error(current_line, character);
            }
            append_char(number, character);
            *current_state = STATE_FRACTION;
            return false;

        case STATE_FRACTION:
            if (isdigit(character))
            {
                append_char(number, character);
                return false;
            }
            if (character == '_')
            {
                *current_state = STATE_FRACTION_UNDERSCORE;
                return false;
            }
            return number_end_step(character, current_state, number, TOKEN_DOUBLE, result_token);

        case STATE_FRACTION_UNDERSCORE:
            if (character == '_')
            {
                return false;
            }
            add_digit_after_underscore(character, number);
            *current_state = STATE_FRACTION;
            return false;

        default:
            return false;
    }
}



// Reads the exponent part of a number, one character at a time.
static bool exponent_state_step(int character, ScannerState *current_state, TokenText *number, Token *result_token)
{
    switch (*current_state)
    {
        case STATE_EXP:
            if (character == '+' || character == '-')
            {
                append_char(number, character);
                *current_state = STATE_EXP_SIGN;
                return false;
            }
            if (!isdigit(character))
            {
                lexical_error(current_line, character);
            }
            append_char(number, character);
            *current_state = STATE_EXP_DIGITS;
            return false;

        case STATE_EXP_SIGN:
            if (!isdigit(character))
            {
                lexical_error(current_line, character);
            }
            append_char(number, character);
            *current_state = STATE_EXP_DIGITS;
            return false;

        case STATE_EXP_DIGITS:
            if (isdigit(character))
            {
                append_char(number, character);
                return false;
            }
            if (character == '_')
            {
                *current_state = STATE_EXP_UNDERSCORE;
                return false;
            }
            if (character == 'd' || character == 'D')
            {
                *current_state = STATE_NUMBER_SUFFIX;
                return false;
            }
            ungetc(character, stdin);
            *result_token = make_number_token(number, TOKEN_DOUBLE);
            return true;

        case STATE_EXP_UNDERSCORE:
            if (character == '_')
            {
                return false;
            }
            add_digit_after_underscore(character, number);
            *current_state = STATE_EXP_DIGITS;
            return false;

        case STATE_NUMBER_SUFFIX:
            ungetc(character, stdin);
            *result_token = make_number_token(number, TOKEN_DOUBLE);
            return true;

        default:
            return false;
    }
}



// Makes the string token from the text.
static Token finish_string_token(TokenText *string_text)
{
    Token new_token;

    line_is_empty = false;
    new_token = make_token(TOKEN_STRING, string_start_line);
    new_token.value = string_text->text;
    return new_token;
}



// Returns the real character for a backslash escape.
static int escape_character(int character_after_backslash)
{
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



// Reads one character of a string.
static bool string_state_step(int character, ScannerState *current_state, TokenText *string_text, Token *result_token)
{
    switch (*current_state)
    {
        case STATE_STRING:
            if (character == EOF || character == '\n')
            {
                fprintf(stderr, "Lexical error, line %d: unclosed string\n", current_line);
                exit(1);
            }
            if (character == '"')
            {
                *result_token = finish_string_token(string_text);
                return true;
            }
            if (character == '\\')
            {
                *current_state = STATE_STRING_ESCAPE;
                return false;
            }
            append_char(string_text, character);
            return false;

        case STATE_STRING_ESCAPE:
            append_char(string_text, escape_character(character));
            *current_state = STATE_STRING;
            return false;

        default:
            return false;
    }
}



// Reads the character after the first quote, or after two quotes.
static bool read_after_opening_quote(int next_character, ScannerState *current_state, TokenText *string_buffer, Token *finished_token)
{
    if (*current_state == STATE_QUOTE)
    {
        if (next_character == '"')
        {
            *current_state = STATE_QUOTE2;
            return false;
        }
        ungetc(next_character, stdin);
        *current_state = STATE_STRING;
        return false;
    }

    if (next_character == '"')
    {
        *current_state = STATE_TRIPLE;
        return false;
    }
    ungetc(next_character, stdin);
    *finished_token = finish_string_token(string_buffer);
    return true;
}



// Reads one character inside a """ string.
static bool read_inside_triple_quotes(int next_character, ScannerState *current_state, TokenText *string_buffer, Token *finished_token)
{
    if (*current_state == STATE_TRIPLE)
    {
        if (next_character == EOF)
        {
            fprintf(stderr, "Lexical error, line %d: unclosed string\n", current_line);
            exit(1);
        }
        if (next_character == '\n')
        {
            current_line++;
        }
        if (next_character == '"')
        {
            *current_state = STATE_TRIPLE_QUOTE1;
        }
        else
        {
            append_char(string_buffer, next_character);
        }
        return false;
    }

    if (*current_state == STATE_TRIPLE_QUOTE1)
    {
        if (next_character == '"')
        {
            *current_state = STATE_TRIPLE_QUOTE2;
            return false;
        }
        append_char(string_buffer, '"');
        ungetc(next_character, stdin);
        *current_state = STATE_TRIPLE;
        return false;
    }

    if (*current_state == STATE_TRIPLE_QUOTE2)
    {
        if (next_character == '"')
        {
            *current_state = STATE_TRIPLE_QUOTE3;
            return false;
        }
        append_char(string_buffer, '"');
        append_char(string_buffer, '"');
        ungetc(next_character, stdin);
        *current_state = STATE_TRIPLE;
        return false;
    }

    if (next_character == '"')
    {
        append_char(string_buffer, '"');
        return false;
    }
    ungetc(next_character, stdin);
    *finished_token = finish_string_token(string_buffer);
    return true;
}




Token get_token(void)
{
    int character;
    ScannerState state = STATE_START;
    TokenText word = { NULL, 0, 0 };
    TokenText number = { NULL, 0, 0 };
    Token token;
    TokenText string_text = { NULL, 0, 0 };
    int comment_depth = 0;
    int comment_start_line = 0;
    bool comment_has_new_line = false;


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

        if (divide_is_next)
        {
            divide_is_next = false;
            return make_token(TOKEN_OPERATOR_DIVIDE, current_line);
        }

        if (main_is_next)
        {
            TokenText main_name;

            main_is_next = false;
            allocate_word(&main_name);
            strcpy(main_name.text, "main");
            return make_word_token(main_name.text);
        }

        character = getchar();

        switch (state)
        {
            case STATE_EQUAL:
                if (character == '=')
                {
                    return make_token(TOKEN_OPERATOR_EQUAL, current_line);
                }
                ungetc(character, stdin);
                return make_token(TOKEN_OPERATOR_ASSIGN, current_line);
            case STATE_LESS:
                if (character == '=')
                {
                    return make_token(TOKEN_OPERATOR_LESS_EQUAL, current_line);
                }
                ungetc(character, stdin);
                return make_token(TOKEN_OPERATOR_LESS, current_line);
            case STATE_GREATER:
                if (character == '=')
                {
                    return make_token(TOKEN_OPERATOR_GREATER_EQUAL, current_line);
                }
                ungetc(character, stdin);
                return make_token(TOKEN_OPERATOR_GREATER, current_line);
            case STATE_EXCLAMATION:
                if (character == '=')
                {
                    return make_token(TOKEN_OPERATOR_NOT_EQUAL, current_line);
                }
                return lexical_error(current_line, '!');
            case STATE_WORD:
                if (is_word_char(character))
                {
                    append_char(&word, character);
                    continue;
                }
                ungetc(character, stdin);
                return make_word_token(word.text);
            case STATE_ZERO:
            case STATE_ZERO_UNDERSCORE:
            case STATE_ZERO_DIGITS:
            case STATE_ZERO_DIGITS_UNDERSCORE:
            case STATE_INT:
            case STATE_INT_UNDERSCORE:
            case STATE_ZERO_NONZERO:
            case STATE_ZERO_NONZERO_UNDERSCORE:
                if (integer_state_step(character, &state, &number, &token))
                {
                    return token;
                }
                continue;
            case STATE_DOT:
            case STATE_FRACTION:
            case STATE_FRACTION_UNDERSCORE:
                if (fraction_state_step(character, &state, &number, &token))
                {
                    return token;
                }
                continue;
            case STATE_EXP:
            case STATE_EXP_SIGN:
            case STATE_EXP_DIGITS:
            case STATE_EXP_UNDERSCORE:
            case STATE_NUMBER_SUFFIX:
                if (exponent_state_step(character, &state, &number, &token))
                {
                    return token;
                }
                continue;
            case STATE_STRING:
            case STATE_STRING_ESCAPE:
                if (string_state_step(character, &state, &string_text, &token))
                {
                    return token;
                }
                continue;
            case STATE_QUOTE:
            case STATE_QUOTE2:
                if (read_after_opening_quote(character, &state, &string_text, &token))
                {
                    return token;
                }
                continue;
            case STATE_TRIPLE:
            case STATE_TRIPLE_QUOTE1:
            case STATE_TRIPLE_QUOTE2:
            case STATE_TRIPLE_QUOTE3:
                if (read_inside_triple_quotes(character, &state, &string_text, &token))
                {
                    return token;
                }
                continue;
            case STATE_SLASH:
                if (character == '/')
                {
                    state = STATE_LINE_COMMENT;
                    continue;
                }
                if (character == '*')
                {
                    comment_start_line = current_line;
                    comment_depth = 1;
                    state = STATE_BLOCK_COMMENT;
                    continue;
                }
                if (line_is_empty && open_paren_depth == 0)
                {
                    check_line_start();
                }
                line_is_empty = false;
                ungetc(character, stdin);
                divide_is_next = true;
                state = STATE_START;
                continue;
            case STATE_LINE_COMMENT:
                if (character == '\n' || character == EOF)
                {
                    ungetc(character, stdin);
                    state = STATE_START;
                }
                continue;
            case STATE_BLOCK_COMMENT:
                if (character == EOF)
                {
                    fprintf(stderr, "Lexical error, line %d: unclosed comment\n", comment_start_line);
                    exit(1);
                }
                if (character == '/')
                {
                    state = STATE_BLOCK_SLASH;
                }
                else if (character == '*')
                {
                    state = STATE_BLOCK_STAR;
                }
                else if (character == '\n')
                {
                    current_line++;
                    comment_has_new_line = true;
                }
                continue;
            case STATE_BLOCK_SLASH:
                if (character == '*')
                {
                    comment_depth++;
                    state = STATE_BLOCK_COMMENT;
                }
                else if (character != '/')
                {
                    ungetc(character, stdin);
                    state = STATE_BLOCK_COMMENT;
                }
                continue;
            case STATE_BLOCK_STAR:
                if (character == '/')
                {
                    comment_depth--;
                    is_first_char_on_line = false;
                    state = STATE_BLOCK_COMMENT;
                    if (comment_depth == 0)
                    {
                        state = STATE_START;
                        line_end_required = comment_has_new_line && open_paren_depth == 0;
                        if (comment_has_new_line && handle_new_line())
                        {
                            return make_token(TOKEN_EOL, comment_start_line);
                        }
                    }
                }
                else if (character != '*')
                {
                    ungetc(character, stdin);
                    state = STATE_BLOCK_COMMENT;
                }
                continue;
            case STATE_DOT_START:
                if (isdigit(character))
                {
                    allocate_word(&number);
                    line_is_empty = false;
                    append_char(&number, '0');
                    append_char(&number, '.');
                    ungetc(character, stdin);
                    state = STATE_DOT;
                    continue;
                }
                ungetc(character, stdin);
                return read_token('.');
            case STATE_ANNOTATION:
                if (character != 'm')
                {
                    lexical_error(current_line, '@');
                }
                state = STATE_ANNOTATION_M;
                continue;
            case STATE_ANNOTATION_M:
                if (character != 'a')
                {
                    lexical_error(current_line, '@');
                }
                state = STATE_ANNOTATION_MA;
                continue;
            case STATE_ANNOTATION_MA:
                if (character != 'i')
                {
                    lexical_error(current_line, '@');
                }
                state = STATE_ANNOTATION_MAI;
                continue;
            case STATE_ANNOTATION_MAI:
                if (character != 'n')
                {
                    lexical_error(current_line, '@');
                }
                state = STATE_ANNOTATION_MAIN;
                continue;
            case STATE_ANNOTATION_MAIN:
                if (is_word_char(character))
                {
                    lexical_error(current_line, '@');
                }
                ungetc(character, stdin);
                line_is_empty = false;
                main_is_next = true;
                return make_token(TOKEN_PUNCTUATION_AT, current_line);
            case STATE_START:
            
                break;
        }

        if (line_end_required && character != ' ' && character != '\t')
        {
            if (character != '\n' && character != EOF)
            {
                fprintf(stderr, "Lexical error, line %d: token after multi-line comment\n", current_line);
                exit(1);
            }
            line_end_required = false;
        }

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
            if (is_first_char_on_line && character == ' ')
            {
                line_start_spaces++;
            }
            if (is_first_char_on_line && character == '\t')
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
            is_first_char_on_line = true;
            if (handle_new_line())
            {
                return make_token(TOKEN_EOL, line);
            }
            continue;
        }


        if (is_first_char_on_line && open_paren_depth > 0)
        {
            check_line_continuation();
        }
        is_first_char_on_line = false;


        if (character == '/')
        {
            state = STATE_SLASH;
            continue;
        }


        if (line_is_empty && open_paren_depth == 0)
        {
            check_line_start();
            ungetc(character, stdin);
            continue;
        }

        
        if (is_word_start(character))
        {
            allocate_word(&word);
            append_char(&word, character);
            line_is_empty = false;
            state = STATE_WORD;
            continue;
        }


        if (isdigit(character))
        {
            allocate_word(&number);
            line_is_empty = false;
            append_char(&number, character);
            state = (character == '0') ? STATE_ZERO : STATE_INT;
            continue;
        }


        if (character == '"')
        {
            allocate_word(&string_text);
            string_start_line = current_line;
            line_is_empty = false;
            state = STATE_QUOTE;
            continue;
        }

        
        if (character == '.')
        {
            state = STATE_DOT_START;
            continue;
        }


        if (character == '@')
        {
            state = STATE_ANNOTATION;
            continue;
        }


        switch (character)
        {
            case '=':
                state = STATE_EQUAL;
                break;
            case '<':
                state = STATE_LESS;
                break;
            case '>':
                state = STATE_GREATER;
                break;
            case '!':
                state = STATE_EXCLAMATION;
                break;
            default:
                return read_token(character);
        }
        line_is_empty = false;
    }
}
