/*
 *
 * Autoři: xpasekj00
 * Soubor: lexer.c
 * Popis:
 */
#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * Proměna  která je potřebna byti viditelnou v celem souboru
 * Kdyz tak predelat a predavat pres argumenty
 */
static LexerState *lexer = NULL;
// pocatecni kapacita buffer ktere je pouzivat pro pridavani znaku do stringu
#define INIT_CAPACITY 32
/*
 * Funkce inicializuje lexer a alokuje paměť pro stav lexikálního analyzátoru.
 *
 * @param source  vstupní řetězec zpracovávaný lexerem (z main.c)
 * @return        0 při úspěchu, -1 při chybě alokace
 */
int lexer_init(const char *source) {
    lexer = malloc(sizeof(LexerState));
    if (lexer == NULL) {
        return -1;
    }
    lexer->length = strlen(source);
    lexer->input = source;
    lexer->pos = 0;
    return 0;
}

/*
 * Uvolní paměť spojenou s lexerem a nastaví ukazatel na NULL.
 */
void lexer_free(void) {
    free(lexer);
    lexer = NULL;
}
/*
 *	Funkce na identifikaci identifikator
 * @param char *string
 * @return TokenType
 */
TokenType match_identifikator(const char *identifikator) {
    if (strcmp(identifikator, "if") == 0) {
        return TOKEN_KEYWORD_IF;
    }
    if (strcmp(identifikator, "while") == 0) {
        return TOKEN_KEYWORD_WHILE;
    }
    if (strcmp(identifikator, "return") == 0) {
        return TOKEN_KEYWORD_RETURN;
    }
    if (strcmp(identifikator, "else") == 0) {
        return TOKEN_ELSE;
    }
    if (strcmp(identifikator, "var") == 0) {
        return TOKEN_KEYWORD_VAR;
    }
    if (strcmp(identifikator, "true") == 0) {
        return TOKEN_TRUE;
    }
    if (strcmp(identifikator, "false") == 0) {
        return TOKEN_FALSE;
    }
    if (strcmp(identifikator, "for") == 0) {
        return TOKEN_KEYWORD_FOR;
    }
    return TOKEN_IDENTIFIER;
}
/*
 * Funkce nakrajeni stringu na token interne NUZKY
 *@param int start startovaci pozice
 *@param int end koncici pozice
 * @return vraci char *vysledek
 */
char *substring(size_t start, size_t end) {
    if (end > lexer->length || start >= end) {
        return NULL;
    }
    int length = end - start;
    char *vysledek = malloc((length + 1) * sizeof(char));
    if (vysledek == NULL) {
        return NULL;
    }
    int pos = 0;
    for (size_t i = start; i < end; i++) {
        vysledek[pos++] = lexer->input[i];
    }
    vysledek[pos] = '\0';
    return vysledek;
}
/*
 * Funkce na proces zpracovani cisel
 */

/*
 * Hlavní funkce konečného automatu pro získání dalšího tokenu.
 *
 * @return  další token ze vstupu; pokud není více tokenů,
 *          vrací TOKEN_EOF; při chybě může vrátit ERROR_TOKEN
 */
const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF:
            return "EOF";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_GLOBAL_ID:
            return "GLOBAL_ID";
        case TOKEN_INTEGER:
            return "INTEGER";
        case TOKEN_STRING:
            return "STRING";
        case TOKEN_STRING_BLOCK:
            return "STRING_BLOCK";
        case TOKEN_FLOAT:
            return "FLOAT";
        case TOKEN_HEXA:
            return "HEXA";
        case TOKEN_INTEGER_EXP:
            return "INTEGER_EXP";
        case TOKEN_FLOAT_EXP:
            return "FLOAT_EXP";
        case TOKEN_KEYWORD_IF:
            return "KEYWORD_IF";
        case TOKEN_KEYWORD_WHILE:
            return "KEYWORD_WHILE";
        case TOKEN_PLUS:
            return "PLUS";
        case TOKEN_MINUS:
            return "MINUS";
        case TOKEN_STAR:
            return "STAR";
        case TOKEN_SLASH:
            return "SLASH";
        case TOKEN_ASSIGN:
            return "ASSIGN";
        case TOKEN_EQ:
            return "EQ";
        case TOKEN_LT:
            return "LT";
        case TOKEN_GT:
            return "GT";
        case TOKEN_LPAREN:
            return "LPAREN";
        case TOKEN_RPAREN:
            return "RPAREN";
        case TOKEN_DOT:
            return "DOT";
        case TOKEN_TRUE:
            return "TRUE";
        case TOKEN_FALSE:
            return "FALSE";
        case TOKEN_QUESTION_MARK:
            return "QUESTION_MARK";
        case TOKEN_LTE:
            return "LTE";
        case TOKEN_GTE:
            return "GTE";
        case TOKEN_NOT:
            return "NOT";
        case TOKEN_DOUBLE_DOT:
            return "DOUBLE_DOT";
        case TOKEN_AND:
            return "AND";
        case TOKEN_OR:
            return "OR";
        case TOKEN_ERROR:
            return "ERROR";
        case TOKEN_NEW_LINE:
            return "NEW_LINE";
        case TOKEN_ELSE:
            return "KEYWORD_ELSE";
        case TOKEN_KEYWORD_RETURN:
            return "KEYWORD_RETURN";
        case TOKEN_KEYWORD_FOR:
            return "KEYWORD_FOR";
        case TOKEN_KEYWORD_VAR:
            return "KEYWORD_VAR";
        case TOKEN_BLOCK_STAR:
            return "BLOCK_STAR";
        case TOKEN_BLOCK_END:
            return "BLOCK_END";
        case TOKEN_SQUARE_LEFT:
            return "SQUARE_LEFT";
        case TOKEN_SQUARE_RIGHT:
            return "SQUARE_RIGHT";
        case TOKEN_DOUBLE_MINUS:
            return "DOUBLE_MINUS";
        default:
            return "UNKNOWN_TOKEN";
    }
}
/*
 *Dopomocna funkce na dodavani znaku do buffru ktery se vrati zpatky v podobe
 *stringu s tokenem
 *@params char ** buf je samotny docastny buffer
 *@params int *delka aktualni delka buffru
 *@params c znak dodavany
 */
void append_char(char **buf, int *delka, int *kapacita, char c) {
    if (*delka + 1 >= *kapacita) {
        *kapacita *= 2;
        *buf = realloc(*buf, *kapacita);
    }
    (*buf)[(*delka)++] = c;
}
Token lexer_token_next() {
    // inicializace bufferu na string
    char *buf = malloc(INIT_CAPACITY);
    int kapacity = INIT_CAPACITY;
    int delak = 0;
    Token token = {0};
    if (lexer == NULL || lexer->pos >= lexer->length) {
        token.type = TOKEN_EOF;
        return token;
    }
    // aktualni znak ktery cteme v automatu
    char c;
    /*
     * Promena zacatek slozi k urceni zacatku :) tokenu
     */
    int zacatek = lexer->pos;
    /*
     * Promena ktera urcuje v jakem stavu(slovo,cislo atd)
     */
    StateType stav = START_LEX;
    // promena na pocitani radku
    int radek = 1;
    // kontrola poctu v hexa cisle v stringu
    int hexa_count = 0;
    // buffer pro hexa cisla v stringu
    char hexa_buffer[3];
    while (1) {
        if (lexer->pos <
            lexer->length) {  // kontrola aby nas counter nepreskocil velikost
            c = lexer->input[lexer->pos];
        } else {
            token.type = TOKEN_EOF;
            return token;
            break;
        }
        if (c == '\n') {
            radek++;  // pocitani radku
        }
        switch (stav) {
                /*
                 *Prvni stav kde jsem uprostred niceho a cekame na zacatek
                 *tokenu a nebo jsme na konci
                 */
                /*
                 * Dodelat stav symbolu ve kterem porovname symboly pridat
                 * token
                 */
            case START_LEX: {
                if (c == '\0') {
                    token.type = TOKEN_EOF;
                    return token;
                } else if (isalpha(c)) {
                    stav = ID;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (c == '0') {
                    stav = ZERO;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (isdigit(c)) {
                    stav = NUMBER;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (c == '"') {
                    stav = STRING;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (c == '/') {
                    stav = BACKSLASH;
                    lexer->pos++;
                } else if (c == '_') {
                    stav = UNDERSCORE;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (c == '\n') {
                    token.type = TOKEN_NEW_LINE;
                    lexer->pos++;
                    return token;
                } else if (c == ' ') {
                    lexer->pos++;  // preskakovani mezer
                } else if (c == '{') {
                    stav = BLOCK_START;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '}') {
                    stav = BLOCK_END;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '(') {
                    stav = PAR_LEFT;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == ')') {
                    stav = PAR_RIGHT;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '[') {
                    stav = SQUARE_LEFT;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == ']') {
                    stav = SQUARE_RIGHT;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '+') {
                    stav = PLUS;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '-') {
                    stav = MINUS;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '*') {
                    stav = START;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '=') {
                    stav = ASSIGN;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '<') {
                    stav = LT;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '>') {
                    stav = GT;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '.') {
                    stav = DOT;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == ':') {
                    stav = DOUBLE_DOT;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '?') {
                    stav = QUESTION_MARK;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '&') {
                    stav = AND_HALF;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '|') {
                    stav = OR_HALF;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '!') {
                    stav = NOT_HALF;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else {
                    // dalsi zatim tady dam error
                    fprintf(stderr,
                            "nezname pismeno v lexer_token_next "
                            "%c on line : %lu",
                            c, lexer->pos);
                    token.type = TOKEN_ERROR;
                    return token;
                }
                break;
            }
            case ID: {
                if (isalnum(c) || c == '_') {
                    lexer->pos++;
                } else {
                    token.value.string = substring(zacatek, lexer->pos);
                    token.type = match_identifikator(token.value.string);
                    token.line = radek;
                    return token;
                }
                break;
            }
            case UNDERSCORE: {
                if (c == '_') {
                    stav = GLOBAL_ID;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_ERROR;
                    fprintf(stderr, "Chybny znak v identifikatoru na radku %i",
                            radek);
                    return token;
                }
                break;
            }
            case GLOBAL_ID: {
                if (isalnum(c) || c == '_') {
                    lexer->pos++;
                } else {
                    token.value.string = substring(zacatek, lexer->pos);
                    token.type = TOKEN_GLOBAL_ID;
                    token.line = radek;
                    return token;
                }
                break;
            }
            case NUMBER: {
                if (isdigit(c)) {
                    lexer->pos++;
                } else if (c == '.') {
                    stav = FLOAT_NUMBER;
                    lexer->pos++;
                } else if (c == ' ' || c == '\n') {
                    token.type = TOKEN_INTEGER;
                    char *tmp = substring(zacatek, lexer->pos);
                    char *endptr;
                    token.value.number = strtod(tmp, &endptr);
                    if (endptr == tmp) {
                        fprintf(stderr,
                                "Chyba: neočekávaný řetězec "
                                "místo čísla: %s\n",
                                tmp);
                        token.type = TOKEN_ERROR;
                        return token;
                    }
                    token.line = radek;
                    free(tmp);
                    return token;
                } else if (c == 'e') {
                    stav = DIGIT_EXP;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                    fprintf(stderr, "Chybny znak v čisle %i u symbolu %c",
                            radek, c);
                    return token;
                }
                break;
            }
            case FLOAT_NUMBER: {
                if (c == 'e') {
                    stav = FLOAT_EXP;
                    lexer->pos++;
                } else if (isdigit(c)) {
                    lexer->pos++;
                } else if (c == ' ' || c == '\n' || c == '\0') {
                    token.type = TOKEN_FLOAT;
                    char *tmp = substring(zacatek, lexer->pos);
                    char *endptr;
                    token.value.number = strtod(tmp, &endptr);
                    if (endptr == tmp) {
                        fprintf(stderr,
                                "Chyba: neočekávaný řetězec "
                                "místo čísla: %s\n",
                                tmp);
                        token.type = TOKEN_ERROR;
                        return token;
                    }
                    token.line = radek;
                    free(tmp);
                    return token;
                } else {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                    fprintf(stderr, "Chybny znak v čisle %i symbol %c", radek,
                            c);
                    return token;
                }
                break;
            }
            case ZERO: {
                if (c == 'x') {
                    stav = HEXA_NUM;
                    lexer->pos++;
                } else if (c == '.') {
                    stav = FLOAT_NUMBER;
                    lexer->pos++;
                } else if (isdigit(c)) {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                    fprintf(stderr, "špatny format integer \n");
                    return token;
                } else {
                    token.type = TOKEN_INTEGER;
                    token.value.number = 0;
                    token.line = radek;
                    return token;
                }
                break;
            }
            case HEXA_NUM: {
                if (isxdigit(c)) {
                    lexer->pos++;
                } else if (c == ' ' || c == '\n' || c == '\0') {
                    token.type = TOKEN_HEXA;
                    char *tmp = substring(zacatek, lexer->pos);
                    char *endptr;
                    token.value.number = strtod(tmp, &endptr);
                    if (endptr == tmp) {
                        fprintf(stderr,
                                "Chyba: neočekávaný řetězec "
                                "místo čísla: %s\n",
                                tmp);
                        token.type = TOKEN_ERROR;
                        token.line = radek;
                        return token;
                    }
                    token.line = radek;
                    token.value.number = strtod(tmp, NULL);
                    free(tmp);
                    return token;
                } else {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                    fprintf(stderr,
                            "Chybny znak v čisle hexa num %i u "
                            "symbolu %c \n",
                            radek, c);
                    return token;
                }
                break;
            }
            case DIGIT_EXP: {
                if (isdigit(c)) {
                    lexer->pos++;
                } else if (c == ' ' || c == '\n' || c == '\0') {
                    token.type = TOKEN_INTEGER_EXP;
                    char *tmp = substring(zacatek, lexer->pos);
                    char *endptr;
                    token.value.number = strtod(tmp, &endptr);
                    if (endptr == tmp) {
                        fprintf(stderr,
                                "Chyba: neočekávaný řetězec "
                                "místo čísla: %s\n",
                                tmp);
                        token.type = TOKEN_ERROR;
                        token.line = radek;
                        return token;
                    }
                    token.line = radek;
                    free(tmp);
                    return token;
                } else {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                    fprintf(stderr, "Chybny znak v čisle digit_exp %i", radek);
                    return token;
                }
                break;
            }
            case FLOAT_EXP: {
                if (isdigit(c)) {
                    lexer->pos++;
                } else if (c == ' ' || c == '\n' || c == '\0') {
                    token.type = TOKEN_FLOAT_EXP;
                    char *tmp = substring(zacatek, lexer->pos);
                    char *endptr;
                    token.value.number = strtod(tmp, &endptr);
                    if (endptr == tmp) {
                        fprintf(stderr,
                                "Chyba: neočekávaný řetězec "
                                "místo čísla: %s\n",
                                tmp);
                        token.type = TOKEN_ERROR;
                        token.line = radek;
                        return token;
                    }
                    token.line = radek;
                    free(tmp);
                    return token;
                } else {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                    fprintf(stderr,
                            "Chybny znak v čisle float exp %i a "
                            "znak %c",
                            radek, c);
                    return token;
                }
                break;
            }
            case BACKSLASH: {
                if (c == '/') {
                    stav = LINE_COMMENT;
                    lexer->pos++;
                } else if (c == '*') {
                    stav = BLOCK_COMMENT;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_SLASH;
                    token.line = radek;
                    return token;
                }
                break;
            }
            case LINE_COMMENT: {
                if (c == '\n') {
                    stav = START_LEX;
                    lexer->pos++;
                }
                lexer->pos++;
                break;
            }
            case BLOCK_COMMENT: {
                if (c == '*') {
                    stav = STAR_COMMENT;
                }
                lexer->pos++;
                break;
            }
            case STAR_COMMENT: {
                if (c == '/') {
                    stav = START_LEX;
                    printf("vratime se \n");
                } else {
                    stav = BLOCK_COMMENT;
                }
                lexer->pos++;
                break;
            }
            case STRING: {
                if (c == '"') {
                    stav = DOUBLE_QUO;  // druhá uvozovka
                    lexer->pos++;
                } else if (c == '\\') {
                    lexer->pos++;
                    char next = lexer->input[lexer->pos];
                    if (next == 'x') {
                        stav = ZNAK_HEXA;
                        hexa_count = 0;
                        lexer->pos++;
                    } else if (next == 'n') {
                        append_char(&buf, &delak, &kapacity, '\n');
                        lexer->pos++;
                    } else if (next == 'r') {
                        append_char(&buf, &delak, &kapacity, '\r');
                        lexer->pos++;
                    } else if (next == 't') {
                        append_char(&buf, &delak, &kapacity, '\t');
                        lexer->pos++;
                    } else if (next == '"' || next == '\\') {
                        append_char(&buf, &delak, &kapacity, next);
                        lexer->pos++;
                    } else {
                        fprintf(stderr,
                                "Chyba: neznámá escape "
                                "sekvence na řádku %i\n",
                                radek);
                        token.type = TOKEN_ERROR;
                        return token;
                    }
                } else if (c == '\0' || c == '\n') {
                    fprintf(stderr,
                            "Chyba: neočekávaný konec řádku v "
                            "řetězci na řádku %i\n",
                            radek);
                    token.type = TOKEN_ERROR;
                    return token;
                } else {
                    append_char(&buf, &delak, &kapacity, c);
                    lexer->pos++;
                }
                break;
            }
            case ZNAK_HEXA: {
                if (isxdigit(c) && hexa_count < 2) {
                    hexa_buffer[hexa_count++] = c;
                    lexer->pos++;
                } else {
                    if (hexa_count == 2) {
                        hexa_buffer[2] = '\0';
                        int vysledek_int = (int)strtol(hexa_buffer, NULL, 16);
                        append_char(&buf, &delak, &kapacity,(char)vysledek_int);
                        stav = STRING;
                        lexer->pos++;
                        hexa_count = 0;
                    } else {
                        fprintf(stderr,
                                "Chyba: neplatná hexadecimální "
                                "escape sekvence na řádku %i\n",
                                radek);
                        token.type = TOKEN_ERROR;
                        return token;
                    }
                }
                break;
            }
            case DOUBLE_QUO: {
                if (c == '"') {
                    // třetí uvozovka → block string
                    stav = STRING_BLOCK;
                    lexer->pos++;
                } else {
                    // byl to normální string
                    append_char(&buf, &delak, &kapacity, '\0');
                    token.type = TOKEN_STRING;
                    token.line = radek;
                    token.value.string = buf;
                    return token;
                }
                break;
            }

            case STRING_BLOCK: {
                if (c == '"') {
                    stav = END_QUO;  // první uvozovka na konci
                    lexer->pos++;
                } else if (c == '\0') {
                    fprintf(stderr,
                            "Chyba: nedokončený block string na "
                            "řádku %i\n",
                            radek);
                    token.type = TOKEN_ERROR;
                    return token;
                } else {
                    lexer->pos++;
                }
                break;
            }
            case END_QUO: {
                if (c == '"') {
                    stav = END_DOUBLE_QUO;  // druhá uvozovka na
                                            // konci
                    lexer->pos++;
                } else {
                    stav = STRING_BLOCK;  // nebyl to konec, zpět do
                                          // bloku
                    lexer->pos++;
                }
                break;
            }

            case END_DOUBLE_QUO: {
                if (c == '"') {
                    // třetí uvozovka → konec block stringu
                    lexer->pos++;
                    token.type = TOKEN_STRING_BLOCK;
                    token.line = radek;
                    token.value.string = substring(zacatek, lexer->pos - 3);
                    return token;
                } else {
                    stav = STRING_BLOCK;  // nebyl to konec, zpět do
                                          // bloku
                    lexer->pos++;
                }
                break;
            }
            case BLOCK_START: {
                token.type = TOKEN_BLOCK_STAR;
                token.line = radek;
                return token;
            }
            case BLOCK_END: {
                token.type = TOKEN_BLOCK_END;
                token.line = radek;
                return token;
            }
            case PAR_LEFT: {
                token.type = TOKEN_LPAREN;
                token.line = radek;
                return token;
            }
            case PAR_RIGHT: {
                token.type = TOKEN_RPAREN;
                token.line = radek;
                return token;
            }
            case SQUARE_LEFT: {
                token.type = TOKEN_SQUARE_LEFT;
                token.line = radek;
                return token;
            }
            case SQUARE_RIGHT: {
                token.type = TOKEN_SQUARE_RIGHT;
                token.line = radek;
                return token;
            }
            case START: {
                token.type = TOKEN_STAR;
                token.line = radek;
                return token;
            }
            case PLUS: {
                token.type = TOKEN_PLUS;
                token.line = radek;
                return token;
            }
            case MINUS: {
                if (c == '-') {
                    stav = DOUBLE_MINUS;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_MINUS;
                    token.line = radek;
                    return token;
                }
                break;
            }

            case DOUBLE_MINUS: {
                token.type = TOKEN_DOUBLE_MINUS;
                token.line = radek;
                return token;
            }
            case SLASH: {
                token.type = TOKEN_SLASH;
                token.line = radek;
                return token;
            }
            case ASSIGN: {
                if (c == '=') {
                    stav = EQ;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_ASSIGN;
                    token.line = radek;
                    return token;
                }
                break;
            }
            case EQ: {
                token.type = TOKEN_EQ;
                token.line = radek;
                return token;
            }
            case LT: {
                if (c == '=') {
                    stav = LTE;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_LT;
                    token.line = radek;
                    return token;
                }
                break;
            }
            case LTE: {
                token.type = TOKEN_LTE;
                token.line = radek;
                return token;
            }
            case GT: {
                if (c == '=') {
                    stav = GTE;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_GT;
                    token.line = radek;
                    return token;
                }
                break;
            }
            case GTE: {
                token.type = TOKEN_GTE;
                token.line = radek;
                return token;
            }
            case DOT: {
                token.type = TOKEN_DOT;
                token.line = radek;
                return token;
            }
            case DOUBLE_DOT: {
                token.type = TOKEN_DOUBLE_DOT;
                token.line = radek;
                return token;
            }
            case QUESTION_MARK: {
                token.type = TOKEN_QUESTION_MARK;
                token.line = radek;
                return token;
            }

            case AND_HALF: {
                if (c == '&') {
                    stav = AND;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                }
                break;
            }
            case AND: {
                token.type = TOKEN_AND;
                token.line = radek;
                return token;
            }
            case OR_HALF: {
                if (c == '|') {
                    stav = OR;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                }
                break;
            }
            case OR: {
                token.type = TOKEN_OR;
                token.line = radek;
                return token;
            }
            case NOT_HALF: {
                if (c == '=') {
                    stav = NOT;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_ERROR;
                    token.line = radek;
                }
                break;
            }
            case NOT: {
                token.type = TOKEN_NOT;
                token.line = radek;
                return token;
            }
            default:
                fprintf(stderr,
                        "chyba v konecnem automatu ve switch to "
                        "spadlo do default \n");
                fprintf(stderr,
                        "chyba nastala s tim to charakterem : %c na "
                        "radku %i \n",
                        c, radek);
                token.type = TOKEN_ERROR;
                return token;
        }
    }
    token.type = TOKEN_ERROR;
    return token;
}
