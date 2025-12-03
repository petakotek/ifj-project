/*
 * Fakulta informačních technologií VUT Brno
 *
 * Autoři: xpasekj00
 * Soubor: lexer.c
 * Popis: Lexer zajistuje pridel tokenu pro parser pres konecny automat
 */
#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "support_functions.h"

/*
 * Proměnné  která jsou potřeba byti viditelnou v celem souboru
 */
static LexerState *lexer        = NULL;
static int         CountComents = 0;
char              *buf          = NULL;
char              *source_file  = NULL;
// pocatecni capacity buffer ktere je pouzivat pro pridavani znaku do řetězců
#define INIT_CAPACITY 32
/*
 * @brief Funkce inicializuje lexer a alokuje paměť pro stav lexikálního analyzátoru.
 *
 * @param source  vstupní řetězec (z main.c)
 * @return        0 při úspěchu, -1 při chybě alokace
 */
int lexerInit(const char *source) {
    lexer = malloc(sizeof(LexerState));
    if (lexer == NULL) {
        printError(INTERNAL_ERROR, "nepovedeny malloc v lex\n");
        memorySafeExit(INTERNAL_ERROR);
    }
    lexer->length = strlen(source);
    lexer->input  = source;
    lexer->pos    = 0;
    return 0;
}

/*
 * @brief Uvolní paměť spojenou s lex a nastaví ukazatel na NULL.
 * @param void
 */
void lexerFree(void) {
    if (lexer != NULL) {
        free(lexer);
        lexer = NULL;
    } else {
    }
    if (buf != NULL) {
        free(buf);
    }
}
/*
 * @briefFunkce na identifikaci identifikator
 * @param char *string
 * @return TokenTyped
 */
TokenType matchIdentifikator(const char *identifikator) {
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
        return TOKEN_KEYWORD_ELSE;
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
    if (strcmp(identifikator, "class") == 0) {
        return TOKEN_KEYWORD_CLASS;
    }
    if (strcmp(identifikator, "Ifj") == 0) {
        return TOKEN_KEYWORD_IFJ;
    }
    if (strcmp(identifikator, "import") == 0) {
        return TOKEN_KEYWORD_IMPORT;
    }
    if (strcmp(identifikator, "Num") == 0) {
        return TOKEN_KEYWORD_NUM;
    }
    if (strcmp(identifikator, "String") == 0) {
        return TOKEN_KEYWORD_STRING;
    }
    if (strcmp(identifikator, "Null") == 0) {
        return TOKEN_KEYWORD_NULL;
    }
    if (strcmp(identifikator, "static") == 0) {
        return TOKEN_KEYWORD_STATIC;
    }
    if (strcmp(identifikator, "is") == 0) {
        return TOKEN_KEYWORD_IS;
    }
    if (strcmp(identifikator, "null") == 0) {
        return TOKEN_KEYWORD_NULL_SMALL;
    }
    if (strcmp(identifikator, "continue") == 0) {
        return TOKEN_KEYWORD_CONTINUE;
    }
    if (strcmp(identifikator, "break") == 0) {
        return TOKEN_KEYWORD_BREAK;
    }
    if (strcmp(identifikator, "in") == 0) {
        return TOKEN_KEYWORD_IN;
    }
    return TOKEN_IDENTIFIER;
}
/*
 * Funkce nakrájení řetězců na token
 *@param int start startovaci pozice
 *@param int end koncici pozice
 * @return vraci char *vysledek
 */
char *substring(size_t start, size_t end) {
    if (end > lexer->length || start >= end) {
        return NULL;
    }
    int   length   = end - start;
    char *vysledek = malloc((length + 1) * sizeof(char));
    if (vysledek == NULL) {
        printError(INTERNAL_ERROR, "nepovedeny malloc v podřetězců v lex\n");
        memorySafeExit(INTERNAL_ERROR);
    }
    int pos = 0;
    for (size_t i = start; i < end; i++) {
        vysledek[pos++] = lexer->input[i];
    }
    vysledek[pos] = '\0';
    return vysledek;
}
/*
 *Funkce ktera prevadi typ tokenu na char * pro print
 */
char *tokenTypeToString(TokenType type) {
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

        case TOKEN_KEYWORD_CLASS:
            return "KEYWORD_CLASS";
        case TOKEN_KEYWORD_IFJ:
            return "KEYWORD_IFJ";
        case TOKEN_KEYWORD_IMPORT:
            return "KEYWORD_IMPORT";
        case TOKEN_KEYWORD_NUM:
            return "KEYWORD_NUM";
        case TOKEN_KEYWORD_STRING:
            return "KEYWORD_STRING";
        case TOKEN_KEYWORD_NULL:
            return "KEYWORD_NULL_CAPS";
        case TOKEN_KEYWORD_STATIC:
            return "KEYWORD_STATIC";
        case TOKEN_KEYWORD_IS:
            return "KEYWORD_IS";
        case TOKEN_KEYWORD_NULL_SMALL:
            return "KEYWORD_NULL_SMALL";
        case TOKEN_KEYWORD_IN:
            return "KEYWORD_IN";
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
        case TOKEN_NOT_EQUAL:
            return "NOT";
        case TOKEN_COLON:
            return "COLON";
        case TOKEN_AND:
            return "AND";
        case TOKEN_OR:
            return "OR";
        case TOKEN_NEW_LINE:
            return "NEW_LINE";
        case TOKEN_KEYWORD_ELSE:
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
        case TOKEN_KEYWORD_CONTINUE:
            return "CONTINUE";
        case TOKEN_KEYWORD_BREAK:
            return "BREAK";
        case TOKEN_COMMA:
            return "COMMA";
        case TOKEN_NOT:
            return "NEGACE";
        case TOKEN_DOLLAR:
            return "DOLLAR";
        case TOKEN_EXPRESSION:
            return "EXPRESSION";
        case TOKEN_STOP:
            return "STOP";
        case TOKEN_DOUBLE_DOT:
            return "DOUBLE_DOT";
        case TOKEN_TRIPLE_DOT:
            return "TRIPLE_DOT";
        default:
            return "UNKNOWN_TOKEN";
    }
}
/**
 * Pomocna funkce na dodavani znaku do bufferu ktery se vrati zpatky v podobe
 * řetězců s token
 *
 * @params char ** buf je samotny dočasný buffer
 * @params int *length aktualni length buffer
 * @params c znak dodavany
 */
void appendChar(char **buf, int *length, int *capacity, char c) {
    if (*length + 1 >= *capacity) {  // ve chvili kdy hrozi preteceni realloc
        *capacity *= 2;
        *buf = realloc(*buf, *capacity);
        if (buf == NULL) {
            printError(INTERNAL_ERROR, "Chyba v realokace\n");
            memorySafeExit(INTERNAL_ERROR);
        }
    }
    (*buf)[(*length)++] = c;  // zapisovani znaku do bufferu
    (*buf)[*length]     = '\0';
}
Token lexerTokenNext() {
    // inicializace bufferu na string

    buf = calloc(INIT_CAPACITY, sizeof(char)); /* Musi se dojit na konec pomoci goto jinak MUSI BYT FREE!!! jinak memory
                                                  leak typicky nasobky 32byt */
    int   capacity = INIT_CAPACITY;
    int   length    = 0;
    Token token    = {0};
    if (lexer == NULL || lexer->pos >= lexer->length) {
        token.type = TOKEN_EOF;
    }
    // aktualni znak ktery cteme v automatu
    char c;
    /*
     * Promena zacatek slozi k urceni zacatku
     */
    int zacatek = lexer->pos;
    /*
     * Promena ktera urcuje v jakem stavu(slovo,cislo atd)
     */
    StateType stav = START_LEX;
    // promena na pocitani radku
    int radek = 1;
    // kontrola poctu v hexa cisle v řetězců
    int hexa_count = 0;
    // buffer pro hexa cisla v řetězců
    char hexa_buffer[3];
    /*
     * Popis fungování konečného automatu (FSA) lexikální analýzy.
     *
     * 1. Postup lex:
     * - Posouváme ukazatel pozice vstupu (lexer->pos) po znacích.
     *
     * 2. Detekce a zpracování tokenů s proměnnou délkou (ID, čísla):
     * - Když ve výchozím stavu START_LEX detekujeme **číslo** nebo **identifikátor**,
     * uložíme aktuální pozici (lexer->pos) do proměnné **počátek**.
     * - Jakmile v těchto stavech narazíme na znak, který neodpovídá pravidlům
     * aktuálního stavu, provedeme **substring** od **počátku** do aktuální
     * pozice (lexer->pos), čímž získáme hotový token (identifikátor, číslo).
     *
     * 3. Zpracování speciálních typů tokenů:
     * - **Blokový řetězec (block string)**: Podobný princip jako ID/čísla (substring).
     * - **Normální řetězec (string)**: Musí se ukládat do **průběžného bufferu (buf)**
     * kvůli možnosti zpracování **hexadecimálních kódů znaků** uvnitř řetězce.
     * - **Jednoduché symboly/operátory**: Vracíme je **okamžitě** (bez nutnosti podřetězců
     * nebo mezi bufferu).
     *
     * 4. Zpracování konce souboru (EOF):
     * - Speciální switch blok na začátku cyklu zajišťuje, že při dosažení EOF
     * (lexer->pos >= lexer->length) je korektně vyhodnocen a vrácen i poslední
     * rozpracovaný token.
     */
    while (1) {
        if (lexer->pos >= lexer->length) {  // kontrola aby nas counter nepreskocil velikost
            /*
             * Switch pro situace ze jsme v urcitem stavu a najdeme EOF bez toho aby tam byl \n tak abychom
             * i ten posledni token vyhodnotili spravnym vystupem
             */
            switch (stav) {
                case START_LEX:
                    token.type = TOKEN_EOF;
                    goto token_found;

                case ID:
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = matchIdentifikator(token.value.identifier);
                    token.line             = radek;
                    goto token_found;

                case GLOBAL_ID:
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = TOKEN_GLOBAL_ID;
                    token.line             = radek;
                    goto token_found;

                case NUMBER:
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = TOKEN_INTEGER;
                    token.line             = radek;
                    goto token_found;

                case HEXA_NUM:
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = TOKEN_HEXA;
                    token.line             = radek;
                    goto token_found;

                case FLOAT_NUMBER:
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = TOKEN_FLOAT;
                    token.line             = radek;
                    goto token_found;

                case DIGIT_EXP:
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = TOKEN_INTEGER_EXP;
                    token.line             = radek;
                    goto token_found;

                case FLOAT_EXP:
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = TOKEN_FLOAT_EXP;
                    token.line             = radek;
                    goto token_found;
                case BLOCK_COMMENT:
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Nedokonceny Komentar"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                    break;
                case STRING:
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Nedokonceny string"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                    break;

                case STRING_BLOCK:
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Nedokonceny string"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                    break;

                case ZNAK_HEXA:
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Nedokonceny string"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                    break;

                case DOUBLE_QUO:
                    token.type             = TOKEN_STRING;
                    token.line             = radek;
                    token.value.identifier = substring(zacatek, lexer->pos);
                    goto token_found_string;

                case END_QUO:
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Nedokonceny string"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                    break;
                case END_DOUBLE_QUO:
                    token.type             = TOKEN_STRING_BLOCK;
                    token.line             = radek;
                    token.value.identifier = substring(zacatek, lexer->pos);
                    goto token_found_string;
                // Jednoznakové operátory a symboly
                case PLUS:
                    token.type = TOKEN_PLUS;
                    goto token_found;

                case MINUS:
                    token.type = TOKEN_MINUS;
                    goto token_found;

                case SLASH:
                    token.type = TOKEN_SLASH;
                    goto token_found;

                case ASSIGN:
                    token.type = TOKEN_ASSIGN;
                    goto token_found;

                case EQ:
                    token.type = TOKEN_EQ;
                    goto token_found;

                case LT:
                    token.type = TOKEN_LT;
                    goto token_found;

                case GT:
                    token.type = TOKEN_GT;
                    goto token_found;

                case LTE:
                    token.type = TOKEN_LTE;
                    goto token_found;

                case GTE:
                    token.type = TOKEN_GTE;
                    goto token_found;

                case START:
                    token.type = TOKEN_STAR;
                    goto token_found;

                case DOT:
                    token.type = TOKEN_DOT;
                    goto token_found;

                case COMMA:
                    token.type = TOKEN_COMMA;
                    goto token_found;

                case COLON:
                    token.type = TOKEN_COLON;
                    goto token_found;

                case QUESTION_MARK:
                    token.type = TOKEN_QUESTION_MARK;
                    goto token_found;

                // Závorky
                case BLOCK_START:
                    token.type = TOKEN_BLOCK_STAR;
                    goto token_found;

                case BLOCK_END:
                    token.type = TOKEN_BLOCK_END;
                    goto token_found;

                case PAR_LEFT:
                    token.type = TOKEN_LPAREN;
                    goto token_found;

                case PAR_RIGHT:
                    token.type = TOKEN_RPAREN;
                    goto token_found;

                // Logické operátory
                case AND_HALF:
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Neznamy charakter"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                    break;

                case AND:
                    token.type = TOKEN_AND;
                    goto token_found;

                case OR_HALF:
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Neznamy charakter"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                    break;

                case OR:
                    token.type = TOKEN_OR;
                    goto token_found;

                case NEGACE:
                    token.type = TOKEN_NOT;
                    goto token_found;

                case NOT:
                    token.type = TOKEN_NOT_EQUAL;
                    goto token_found;

                default:
                    token.type = TOKEN_EOF;
                    goto token_found;
            }
        }
        /*
         * Samotna rozhodovaci cast automatu
         */
        c = lexer->input[lexer->pos];
        if (c == '\n') {
            radek++;
        }
        switch (stav) {
                /*
                 *Pocatecni stav
                 */
            case START_LEX: {
                // pocatecni vstup a jeho rozpoznani
                if (c == '\0') {
                    token.type = TOKEN_EOF;
                    goto token_found;
                } else if (isalpha(c)) {
                    stav    = ID;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (c == '0') {
                    stav    = ZERO;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (isdigit(c)) {
                    stav    = NUMBER;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (c == '"') {
                    stav    = STRING;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (c == '/') {
                    stav = BACKSLASH;
                    lexer->pos++;
                } else if (c == '_') {
                    stav    = UNDERSCORE;
                    zacatek = lexer->pos;
                    lexer->pos++;
                } else if (c == '\n') {
                    token.type = TOKEN_NEW_LINE;
                    lexer->pos++;
                    goto token_found;
                } else if (isblank(c)) {
                    lexer->pos++;  // preskakovani mezer
                } else if (c == '{') {
                    stav = BLOCK_START;
                    lexer->pos++;
                } else if (c == '}') {
                    stav = BLOCK_END;
                    lexer->pos++;
                } else if (c == '(') {
                    stav = PAR_LEFT;
                    lexer->pos++;
                } else if (c == ')') {
                    stav = PAR_RIGHT;
                    lexer->pos++;
                } else if (c == '+') {
                    stav = PLUS;
                    lexer->pos++;
                } else if (c == '-') {
                    stav = MINUS;
                    lexer->pos++;
                } else if (c == '*') {
                    stav = START;
                    lexer->pos++;
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
                } else if (c == ':') {
                    stav = COLON;
                    lexer->pos++;
                } else if (c == '?') {
                    stav = QUESTION_MARK;
                    lexer->pos++;
                } else if (c == '&') {
                    stav = AND_HALF;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '|') {
                    stav = OR_HALF;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == '!') {
                    stav = NEGACE;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else if (c == ',') {
                    stav = COMMA;
                    lexer->pos++;
                    zacatek = lexer->pos;
                } else {
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Nezname pismeno v lexerTokenNext "
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                }
                break;
            }
            // Sekce identifikatoru
            case ID: {
                if (isalnum(c) || c == '_') {
                    lexer->pos++;
                } else {
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = matchIdentifikator(token.value.identifier);
                    token.line             = radek;
                    goto token_found;
                }
                break;
            }
            /*
             *Sekce globalnich identifikatoru
             * Mezi stav pro _
             */
            case UNDERSCORE: {
                if (c == '_') {
                    stav = GLOBAL_ID;
                    lexer->pos++;
                } else {
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Nezname pismeno v lexerTokenNext"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                }
                break;
            }
            case GLOBAL_ID: {
                if (isalnum(c) || c == '_') {
                    lexer->pos++;
                } else {
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.type             = TOKEN_GLOBAL_ID;
                    token.line             = radek;
                    goto token_found;
                }
                break;
            }
                /*
                 * Stav cislo ze ktereho se da dostat do float_number, Digit_exp
                 */
            case NUMBER: {
                if (isdigit(c)) {
                    lexer->pos++;
                } else if (c == '.') {
                    char next_char = lexer->input[lexer->pos + 1];

                    if (isdigit(next_char)) {
                        stav = FLOAT_NUMBER;
                        lexer->pos++;
                    } else {
                        token.type             = TOKEN_INTEGER;
                        token.value.identifier = substring(zacatek, lexer->pos);
                        token.line             = radek;
                        goto token_found;
                    }
                } else if (c == 'e' || c == 'E') {
                    stav = DIGIT_EXP;
                    lexer->pos++;
                } else {
                    token.type             = TOKEN_INTEGER;
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.line             = radek;
                    goto token_found;
                }
                break;
            }

                /*
                 * Stav cislo ze ktereho se da dostat do float_exp
                 */

            case FLOAT_NUMBER: {
                if (isdigit(c)) {
                    lexer->pos++;
                } else if (c == 'e' || c == 'E') {
                    stav = FLOAT_EXP;
                    lexer->pos++;
                } else {
                    char *lexeme = substring(zacatek, lexer->pos);
                    token        = isEffectivelyInteger(lexeme, token);
                    if (token.type == 0)
                        token.type = TOKEN_FLOAT;
                    token.value.identifier = lexeme;
                    token.line             = radek;
                    goto token_found;
                }
                break;
            }

            /*
             * Stav pro nulo ve kterem se bud dostaneme do float_number pokud je nasledovana tecko
             * nebo do hexa pokud nasleduje x
             */
            case ZERO: {
                if (c == 'x') {
                    char current_char = lexer->input[lexer->pos + 1];
                    if (!isxdigit(current_char)) {
                        free(buf);
                        buf = NULL;
                        printError(LEX_ERROR,
                                   "Neznamy charakter v lexerTokenNext"
                                   "%c on line : %lu",
                                   current_char, lexer->pos + 1);
                        memorySafeExit(LEX_ERROR);
                    }
                    stav = HEXA_NUM;
                    lexer->pos++;
                } else if (c == '.') {
                    stav = FLOAT_NUMBER;
                    lexer->pos++;
                } else if (isdigit(c)) {
                   stav = NUMBER;
                   lexer->pos++;
                }else if(c == 'e' || c == 'E'){
            	   	stav = DIGIT_EXP;
                    lexer->pos++;
                }else {
                    token.type             = TOKEN_INTEGER;
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.line             = radek;
                    goto token_found;
                }
                break;
            }
            /*
             * Stav pro hexa cislo pokud nedostaneme platne hexa cislo hazime error
             */
            case HEXA_NUM: {
                if (isxdigit(c)) {
                    lexer->pos++;
                } else {
                    token.type             = TOKEN_HEXA;
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.line             = radek;
                    goto token_found;
                }
                break;
            }
                /*
                 * Stav digit_exp a float_exp jsou identicke pokud nedostanou platne cislo tak vraci
                 * error
                 */
            case DIGIT_EXP: {
                if (isdigit(c)) {
                    lexer->pos++;
                } else if ((c == '+' || c == '-')) {
                    char next_char = lexer->input[lexer->pos + 1];
                    if (isdigit(next_char)) {
                        lexer->pos++;
                    } else {
                        free(buf);
                        buf=NULL;
                        printError(LEX_ERROR, "Neznamy charakter v lexerTokenNext: %c on line : %lu", next_char,
                                   lexer->pos);
                        memorySafeExit(LEX_ERROR);
                    }
                } else {
                    char prev_char = lexer->input[lexer->pos - 1];
                    if (!isdigit(prev_char)) {
                        free(buf);
                        buf=NULL;
                        printError(LEX_ERROR, "Neplatny format cisla");
                        memorySafeExit(LEX_ERROR);
                    }
                    token.type             = TOKEN_INTEGER_EXP;
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.line             = radek;
                    goto token_found;
                }
                break;
            }
            case FLOAT_EXP: {
                if (isdigit(c)) {
                    lexer->pos++;
                } else if ((c == '+' || c == '-')) {
                    char next_char = lexer->input[lexer->pos + 1];
                    if (isdigit(next_char)) {
                        lexer->pos++;
                    } else {
                        free(buf);
                        buf = NULL;
                        printError(LEX_ERROR, "Neznamy charakter v lexerTokenNext: %c on line : %lu", next_char,
                                   lexer->pos);
                        memorySafeExit(LEX_ERROR);
                    }
                } else {
                    char prev_char = lexer->input[lexer->pos - 1];
                    if (!isdigit(prev_char)) {
                        free(buf);
                        buf = NULL;
                        printError(LEX_ERROR, "Neplatny format cisla (chybi exponent): %s",
                                   substring(zacatek, lexer->pos), lexer->pos);
                        memorySafeExit(LEX_ERROR);
                    }
                    token.type             = TOKEN_FLOAT_EXP;
                    token.value.identifier = substring(zacatek, lexer->pos);
                    token.line             = radek;
                    goto token_found;
                }
                break;
            }
                /*
                 * Zpracovani komentaru a jejich preskoceni
                 */

            case BACKSLASH: {
                if (c == '/') {
                    stav = LINE_COMMENT;
                    lexer->pos++;
                } else if (c == '*') {
                    stav         = BLOCK_COMMENT;
                    CountComents = 1;
                    lexer->pos++;
                } else {
                	token.type = TOKEN_SLASH;
                 	goto token_found;
                }
                break;
            }

            case LINE_COMMENT: {
                if (c == '\n') {
                    token.type = TOKEN_NEW_LINE;
                    token.line = radek;
                    lexer->pos++;
                    goto token_found;
                }
                lexer->pos++;
                break;
            }

            case BLOCK_COMMENT: {
                if (c == '/' && lexer->input[lexer->pos + 1] == '*') {
                    CountComents++;
                    lexer->pos += 2;
                    break;
                }

                if (c == '*' && lexer->input[lexer->pos + 1] == '/') {
                    CountComents--;
                    lexer->pos += 2;
                    if (CountComents == 0) {
                        stav = START_LEX;
                    }
                    break;
                }

                lexer->pos++;
                break;
            }
            case STRING: {
                if (c == '"') {
                    stav = DOUBLE_QUO;  // druhá uvozovka
                    lexer->pos++;
                } else if (c == '\\') {
                    // Escape znaky
                    lexer->pos++;
                    char next = lexer->input[lexer->pos];
                    if (next == 'x') {
                        // hura hexa cislo
                        stav       = ZNAK_HEXA;
                        hexa_count = 0;
                        lexer->pos++;
                    } else if (next == 'n') {
                        appendChar(&buf, &length, &capacity, '\n');
                        lexer->pos++;
                    } else if (next == 'r') {
                        appendChar(&buf, &length, &capacity, '\r');
                        lexer->pos++;
                    } else if (next == 't') {
                        appendChar(&buf, &length, &capacity, '\t');
                        lexer->pos++;
                    } else if (next == '"') {
                        appendChar(&buf, &length, &capacity, '\"');
                        lexer->pos++;
                    } else if (next == '\\') {
                        appendChar(&buf, &length, &capacity, '\\');
                        lexer->pos++;
                    }

                    else {
                        free(buf);
                        buf = NULL;
                        printError(LEX_ERROR,
                                   "Neznamy jsem tu v else v string charakter v lexerTokenNext"
                                   "%c on line : %lu",
                                   c, lexer->pos);
                        memorySafeExit(LEX_ERROR);
                    }
                } else if (c == '\0' || c == '\n') {
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Neznamy charakter v lexerTokenNext"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                } else {
                	if(c > 31){
                    // tady je nacitani znaku do dočasného bufferu
                    appendChar(&buf, &length, &capacity, c);
                    lexer->pos++;
                 }else {
                 	free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                                "Neznamy jsem tu v else v string charakter v lexerTokenNext"
                                "%c on line : %lu",
                                c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                 }
                }
                break;
            }
            case ZNAK_HEXA: {
                if (isxdigit(c) && hexa_count < 2) {
                    // pridavame hexa cislo do hexa_buffer coz je docasny buffer pro hexa cislo
                    hexa_buffer[hexa_count++] = c;
                    lexer->pos++;
                } else {
                    if (hexa_count == 2) {
                        hexa_buffer[2] = '\0';
                        // prevedeni na cislo
                        int vysledek_int = (int)strtol(hexa_buffer, NULL, 16);
                        // prevedeni na znak
                        appendChar(&buf, &length, &capacity, (char)vysledek_int);
                        stav       = STRING;
                        hexa_count = 0;
                    } else {
                        free(buf);
                        buf = NULL;
                        printError(LEX_ERROR,
                                   "Neznamy charakter v lexerTokenNext"
                                   "%c on line : %lu",
                                   c, lexer->pos);
                        memorySafeExit(LEX_ERROR);
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
                    // byl to normální string a tak predavame docasny buffer do return tokenu
                    token.type             = TOKEN_STRING;
                    token.line             = radek;
                    token.value.identifier = buf;
                    goto token_found_string;
                }
                break;
            }

            case STRING_BLOCK: {
                if (c == '"') {
                    stav = END_QUO;  // první uvozovka na konci
                    lexer->pos++;
                } else if (c == '\0') {
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Neznamy charakter v lexerTokenNext"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
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
                    // třetí uvozovka → konec block řetězců
                    lexer->pos++;
                    token.type             = TOKEN_STRING_BLOCK;
                    token.line             = radek;
                    token.value.identifier = substring(zacatek + 3, lexer->pos - 3);
                    goto token_found;
                } else {
                    stav = STRING_BLOCK;  // nebyl to konec, zpět do
                                          // bloku
                    lexer->pos++;
                }
                break;
            }
            // Velka sekce symbolu
            case BLOCK_START: {
                token.type = TOKEN_BLOCK_STAR;
                token.line = radek;
                goto token_found;
            }
            case BLOCK_END: {
                token.type = TOKEN_BLOCK_END;
                token.line = radek;
                goto token_found;
            }
            case PAR_LEFT: {
                token.type = TOKEN_LPAREN;
                token.line = radek;
                goto token_found;
            }
            case PAR_RIGHT: {
                token.type = TOKEN_RPAREN;
                token.line = radek;
                goto token_found;
            }
            case START: {
                token.type = TOKEN_STAR;
                token.line = radek;
                goto token_found;
            }
            case PLUS: {
                token.type = TOKEN_PLUS;
                token.line = radek;
                goto token_found;
            }
            case MINUS: {
                token.type = TOKEN_MINUS;
                token.line = radek;
                goto token_found;
                break;
            }
            case SLASH: {
                token.type = TOKEN_SLASH;
                token.line = radek;
                goto token_found;
            }
            case COMMA: {
                token.type = TOKEN_COMMA;
                token.line = radek;
                goto token_found;
            }
            // mezi stav pro mozne == ale vraci take samotne =
            // to same u >= nebo u <=
            case ASSIGN: {
                if (c == '=') {
                    stav = EQ;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_ASSIGN;
                    token.line = radek;
                    goto token_found;
                }
                break;
            }
            case EQ: {
                token.type = TOKEN_EQ;
                token.line = radek;
                goto token_found;
            }
            case LT: {
                if (c == '=') {
                    stav = LTE;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_LT;
                    token.line = radek;
                    goto token_found;
                }
                break;
            }
            case LTE: {
                token.type = TOKEN_LTE;
                token.line = radek;
                goto token_found;
            }
            case GT: {
                if (c == '=') {
                    stav = GTE;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_GT;
                    token.line = radek;
                    goto token_found;
                }
                break;
            }
            case GTE: {
                token.type = TOKEN_GTE;
                token.line = radek;
                goto token_found;
            }
            case DOT: {
                if (c == '.') {
                    stav = DOUBLE_DOT;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_DOT;
                    token.line = radek;
                    goto token_found;
                }
                break;
            }
            case DOUBLE_DOT: {
                if (c == '.') {
                    stav = TRIPLE_DOT;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_DOUBLE_DOT;
                    token.line = radek;
                    goto token_found;
                }
                break;
            }
            case TRIPLE_DOT: {
                token.type = TOKEN_TRIPLE_DOT;
                token.line = radek;
                goto token_found;
            }
            // terciární operatory
            case COLON: {
                token.type = TOKEN_COLON;
                token.line = radek;
                goto token_found;
            }
            case QUESTION_MARK: {
                token.type = TOKEN_QUESTION_MARK;
                token.line = radek;
                goto token_found;
            }
            // pravdivostni operatory
            // take mezi stav tady pokud nenasleduje druhe & tak se vraci error
            case AND_HALF: {
                if (c == '&') {
                    stav = AND;
                    lexer->pos++;
                } else {
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Neznamy charakter v lexerTokenNext"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                }
                break;
            }
            case AND: {
                token.type = TOKEN_AND;
                token.line = radek;
                goto token_found;
            }
            // take mezi stav tady pokud nenasleduje druhe | tak se vraci error
            case OR_HALF: {
                if (c == '|') {
                    stav = OR;
                    lexer->pos++;
                } else {
                    free(buf);
                    buf = NULL;
                    printError(LEX_ERROR,
                               "Neznamy charakter v lexerTokenNext"
                               "%c on line : %lu",
                               c, lexer->pos);
                    memorySafeExit(LEX_ERROR);
                }
                break;
            }
            case OR: {
                token.type = TOKEN_OR;
                token.line = radek;
                goto token_found;
            }

            case NEGACE: {
                if (c == '=') {
                    stav = NOT;
                    lexer->pos++;
                } else {
                    token.type = TOKEN_NOT;
                    token.line = radek;
                    goto token_found;
                }
                break;
            }
            case NOT: {
                token.type = TOKEN_NOT_EQUAL;
                token.line = radek;
                goto token_found;
            }
            default:
                free(buf);
                buf = NULL;
                printError(LEX_ERROR,
                           "Neznamy charakter v lexerTokenNext"
                           "%c on line : %lu",
                           c, lexer->pos);
                memorySafeExit(LEX_ERROR);
        }
    }
    free(buf);
    buf = NULL;
    printError(LEX_ERROR,
               "Neznamy charakter v lexerTokenNext"
               "%c on line : %lu",
               c, lexer->pos);
    memorySafeExit(LEX_ERROR);
token_found:;
    free(buf);
    buf = NULL;
token_found_string:;
    return token;
}

Token isEffectivelyInteger(char *input, Token token) {
    bool dot = false;
    int  pos_dot;
    int  i = 0;
    while (input[i] != '\0') {
        if (input[i] == '.') {
            dot = true;
            break;
        }
        i++;
    }

    if (!dot) {
        token.value.identifier = input;
        token.type             = TOKEN_INTEGER;
        return token;
    }
    i++;
    pos_dot = i;
    while (input[i] != '\0') {
        if (input[i] != '0') {
            token.value.identifier = input;
            token.type             = TOKEN_FLOAT;
            return token;
        }
        i++;
    }
    input[pos_dot - 1]     = '\0';
    token.value.identifier = input;
    token.type             = TOKEN_INTEGER;
    return token;
}
/*** konec soubor ***/
