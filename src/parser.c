/*
    Implementace překladače imperativního jazyka IFJ26.
*/
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "symtable.h"
#include "scanner.h"
#include "expr.h"

int parse_program();
int parse_func_def();

int parse_expr_or_call();
int parse_block();

int parse_statement();

int parse_def();
int parse_assign();
int parse_call();
int parse_return();
int parse_if();
int parse_while();
int parse_expr();

Symtable *table;
Token current_token;
bool has_main = false;

void next_token() {
    current_token = get_token();
}

int expect_token(TokenType expected_token) {
    if (current_token.type != expected_token) {
        return SYNTAX_ERROR;
    }
    next_token();
    return SYNTAX_OK;
}

int expect_token_and_attr(TokenType expected_token, char* expected_attr) {
    if (current_token.type != expected_token || current_token.value == NULL || strcmp(current_token.value, expected_attr) != 0) {
        return SYNTAX_ERROR;
    }
    next_token();
    return SYNTAX_OK;
}


int parse(Symtable *symtable)
{
    has_main = false;
    scanner_reset();

    table = symtable;
    int res = parse_program();
    return res;
}

int parse_program(){
    // Kontrola importu ifj26 na zacatku programu
    next_token();
    if (expect_token(TOKEN_KW_IMPORT) != SYNTAX_OK)
        return SYNTAX_ERROR;

    if (expect_token_and_attr(TOKEN_IDENT, "ifj26") != SYNTAX_OK)
        return SYNTAX_ERROR;

    if (expect_token(TOKEN_PUNCTUATION_DOT) != SYNTAX_OK)
        return SYNTAX_ERROR;

    if (expect_token(TOKEN_OPERATOR_MULTIPLY) != SYNTAX_OK)
        return SYNTAX_ERROR;

    if (expect_token(TOKEN_EOL) != SYNTAX_OK)
        return SYNTAX_ERROR;

    //Do konce souboru
    while(current_token.type != TOKEN_EOF)
    {
        int res = parse_func_def();
        if (res != SYNTAX_OK)
            return res;
    }

    if (!has_main) {
        return SEM_ERROR_NON_DEFINE; // chyba 3
    }

    return SYNTAX_OK;
}

int parse_func_def(){
    //Kontrola main funkce
    bool is_main = false;
    if (current_token.type == TOKEN_PUNCTUATION_AT) {
        next_token();
        if(expect_token_and_attr(TOKEN_IDENT, "main") != SYNTAX_OK) {
            return SYNTAX_ERROR;
        }
        if (has_main) {
            return SEM_ERROR_REDEFINITION;
        }
        has_main = true;
        is_main = true;

        if (current_token.type == TOKEN_EOL) {
            next_token();
        }
    }

    //Zacatek funkce
    if (expect_token(TOKEN_KW_DEF) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //Nazev funkce
    if (expect_token(TOKEN_IDENT) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }
    //(
    if(expect_token(TOKEN_PUNCTUATION_LEFT_PAREN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }
    //Parametry funkce
    if(is_main == false) {
        while(current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN && current_token.type != TOKEN_EOF) {
            //Nazev parametru
            if (expect_token(TOKEN_IDENT) != SYNTAX_OK) {
                return SYNTAX_ERROR;
            }
            //:
            if (expect_token(TOKEN_PUNCTUATION_COLON) != SYNTAX_OK) {
                return SYNTAX_ERROR;
            }
            //Typ parametru
            if(current_token.type != TOKEN_KW_INT && current_token.type != TOKEN_KW_DOUBLE && current_token.type != TOKEN_KW_STRING && current_token.type != TOKEN_KW_UNIT) {
                return SYNTAX_ERROR;
            }
            next_token();
            //bud , mezi parametry a nebo musi byt zavorka
            if(current_token.type == TOKEN_PUNCTUATION_COMMA) {
                next_token();
                // za , musi byt identifikator
                if (current_token.type == TOKEN_PUNCTUATION_RIGHT_PAREN) {
                    return SYNTAX_ERROR;
                }
            } else if(current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN) {
                return SYNTAX_ERROR;
            }
        }
        
    }
    
    //pokud je main musi byt bez parametru
    if(expect_token(TOKEN_PUNCTUATION_RIGHT_PAREN) != SYNTAX_OK) {
            return SEM_ERROR_INVALID_ARGUMENT_COUNT;
    }

    //Navratovy typ
    int ret_type = -1;
    if (current_token.type == TOKEN_PUNCTUATION_COLON) {
        next_token();
        if (current_token.type != TOKEN_KW_INT && current_token.type != TOKEN_KW_DOUBLE &&
            current_token.type != TOKEN_KW_STRING && current_token.type != TOKEN_KW_UNIT) {
            return SYNTAX_ERROR;
        }
        ret_type = current_token.type;
        next_token();
    }
    if (is_main && ret_type != TOKEN_KW_UNIT) {
        return SEM_ERROR_TYPE_MISMATCH;
    }
    // =
    if(expect_token(TOKEN_OPERATOR_ASSIGN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //blok nebo jednoratkovej vyraz
    if (current_token.type == TOKEN_EOL) {
        next_token();
        return parse_block();
    } else {
        int res = parse_expr_or_call();
        if (res != SYNTAX_OK) {
            return res;
        }

        // Kontrola, zda je na konci radku nebo souboru, jinak error
        if (current_token.type == TOKEN_EOL) {
            next_token();
        } else if (current_token.type != TOKEN_EOF) {
            return SYNTAX_ERROR;
        }
        return SYNTAX_OK;
    }

    return SYNTAX_OK;
}

int parse_block() {
    //Zacatek bloku
    if (expect_token(TOKEN_BLOCK_START) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    // Blok nesmi byt prazdny
    if (current_token.type == TOKEN_BLOCK_END || current_token.type == TOKEN_EOF) {
        return SYNTAX_ERROR;
    }

    //Do konce bloku nebo souboru
    while (current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        int res = parse_statement();
        if (res != SYNTAX_OK) {
            return res;
        }
    }

    //konec bloku
    if (expect_token(TOKEN_BLOCK_END) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    return SYNTAX_OK;
}

int parse_expr_or_call() {
    if (current_token.type == TOKEN_IDENT) {
        Token next = peek_token();
        if (next.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            return parse_call();
        }
    }
    return parse_expr();
}

int parse_statement() {
    //Blokove prikazy
    if (current_token.type == TOKEN_KW_IF) {
        return parse_if();
    }
    if (current_token.type == TOKEN_KW_WHILE) {
        return parse_while();
    }

    //Jednoradkove
    int res = SYNTAX_ERROR;

    if (current_token.type == TOKEN_KW_VAL || current_token.type == TOKEN_KW_VAR) {
        res = parse_def();
    } 
    else if (current_token.type == TOKEN_KW_RETURN) {
        res = parse_return();
    } 
    else if (current_token.type == TOKEN_IDENT) {
        // prirazeni nebo funkce
        Token next = peek_token();
        if (next.type == TOKEN_OPERATOR_ASSIGN) {
            res = parse_assign();
        } 
        else if (next.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            res = parse_call();
        } 
        else {
            res = parse_expr();
        }
    } 
    else if (current_token.type == TOKEN_PUNCTUATION_LEFT_PAREN || current_token.type == TOKEN_OPERATOR_MINUS || current_token.type == TOKEN_INT || current_token.type == TOKEN_DOUBLE || current_token.type == TOKEN_STRING ||current_token.type == TOKEN_KW_UNIT) {
        res = parse_expr();
    } 
    else {
        return SYNTAX_ERROR;
    }

    if (res != SYNTAX_OK) {
        return res;
    }

    //Oddelovac za jednoradkovym prikazem
    if (current_token.type == TOKEN_EOL || current_token.type == TOKEN_PUNCTUATION_SEMICOLON) {
        next_token();
    } else if (current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        return SYNTAX_ERROR;
    }

    return SYNTAX_OK;
}

int parse_def() {
    //var nebo val
    if(current_token.type != TOKEN_KW_VAL && current_token.type != TOKEN_KW_VAR) {
        return SYNTAX_ERROR;
    }
    next_token();

    //jmeno
    if(expect_token(TOKEN_IDENT) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //typ
    if(current_token.type == TOKEN_PUNCTUATION_COLON) {
        next_token();
        if(current_token.type != TOKEN_KW_INT && current_token.type != TOKEN_KW_DOUBLE && current_token.type != TOKEN_KW_STRING && current_token.type != TOKEN_KW_UNIT) {
            return SYNTAX_ERROR;
        }
        next_token();
    }

    //=
    if(expect_token(TOKEN_OPERATOR_ASSIGN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    return parse_expr_or_call();
}

int parse_assign() {
    //jmeno promenne
    if(expect_token(TOKEN_IDENT) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //=
    if(expect_token(TOKEN_OPERATOR_ASSIGN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    if(parse_expr_or_call() != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    return SYNTAX_OK;
}

int parse_call() {
    //jmeno
    if(expect_token(TOKEN_IDENT) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //(
    if(expect_token(TOKEN_PUNCTUATION_LEFT_PAREN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //parametry
    while (current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN && current_token.type != TOKEN_EOF) {
        //vyraz nebo volani
        if (current_token.type == TOKEN_IDENT || current_token.type == TOKEN_INT || current_token.type == TOKEN_DOUBLE || current_token.type == TOKEN_STRING) {
            next_token();
        } else if (current_token.type == TOKEN_PUNCTUATION_LEFT_PAREN) {
            next_token();
            if (expect_token(TOKEN_PUNCTUATION_RIGHT_PAREN) != SYNTAX_OK) return SYNTAX_ERROR;
        } else {
            return SYNTAX_ERROR;
        }

    if (current_token.type == TOKEN_PUNCTUATION_COMMA) {
        next_token();
        //osetreni odradkovani mezi parametry
        if (current_token.type == TOKEN_EOL) {
            next_token();
            if (current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN) return SYNTAX_ERROR;
        } else if (current_token.type == TOKEN_PUNCTUATION_RIGHT_PAREN) {
            return SYNTAX_ERROR;
        }
    } else if (current_token.type != TOKEN_PUNCTUATION_RIGHT_PAREN) {
        return SYNTAX_ERROR;
    }
}

    //)
    if(expect_token(TOKEN_PUNCTUATION_RIGHT_PAREN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    return SYNTAX_OK;
}

int parse_return() {
    //return
    if(expect_token(TOKEN_KW_RETURN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    if (current_token.type != TOKEN_EOL && current_token.type != TOKEN_PUNCTUATION_SEMICOLON &&
        current_token.type != TOKEN_BLOCK_END && current_token.type != TOKEN_EOF) {
        if (parse_expr_or_call() != SYNTAX_OK) {
            return SYNTAX_ERROR;
        }
    }
    return SYNTAX_OK;
}

int parse_if() {
    //if
    if(expect_token(TOKEN_KW_IF) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //vyraz
    if(parse_expr() != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //then
    if(expect_token(TOKEN_KW_THEN) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }
    if(expect_token(TOKEN_EOL) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //blok 
    if(parse_block() != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //else
    if(expect_token(TOKEN_KW_ELSE) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }
    if(expect_token(TOKEN_EOL) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //else blok
    return parse_block();
}

int parse_while() {
    //while
    if(expect_token(TOKEN_KW_WHILE) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //vyraz
    if(parse_expr() != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    //do
    if(expect_token(TOKEN_KW_DO) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }
    if(expect_token(TOKEN_EOL) != SYNTAX_OK) {
        return SYNTAX_ERROR;
    }

    return parse_block();
}