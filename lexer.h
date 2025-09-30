#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
*Definice typu tokenu v lexeru
*/
//Stav do kterych se muze dostat
typedef enum{
	START_LEX,
	ID,
	GLOBAL_ID,
	NUMBER,
	FLOAT_NUMBER,
	HEXA_NUM,
	FLOAT_EXP,
	DIGIT_EXP,
	NEG_DIGIT_EXP,
	STRING,
	STRING_BACK_S,
	STRING_HEX_N,
	BACKSLASH,
	LINE_COMMENT,
	BLOCK_COMMENT,

}StateType ;
//nějaky natrel co by tam mohlo byt
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_WHILE,
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_STAR,      // *
    TOKEN_SLASH,     // /
    TOKEN_ASSIGN,    // =
    TOKEN_EQ,        // ==
    TOKEN_LT,        // <
    TOKEN_GT,        // >
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_SEMICOLON, // ;
    TOKEN_ERROR,
    TOKEN_KEYWORD_RETURN
} TokenType;
/*
*Definuje samotny jeden token
*obecne prametry tokenu: Line(pro error zpravy) .. vice mene nenapadlo
* Union pro specificke parametry daneho tokenu ty nejdulezitejsi hodnoty()
*/
typedef struct {
    TokenType type;
    union {
           char *identifier;
           double number;
           char *string;
       } value;
    int line;
} Token;
/*
* Struktura pro samotny vstup at vime jak dlouhy je a kde se nachazime
*/
typedef struct {
    const char *input;
    size_t pos;
    size_t length;
} LexerState;
/*
 * Funkce inicializuje lexer a alokuje paměť pro stav lexikálního analyzátoru.
 *
 * @param source  vstupní řetězec zpracovávaný lexerem (z main.c)
 * @return        0 při úspěchu, -1 při chybě alokace
 */
int lexer_init(const char *source);
/*
 * Uvolní paměť spojenou s lexerem a nastaví ukazatel na NULL.
 */
void lexer_free(void);
/*
 * Hlavní funkce konečného automatu pro získání dalšího tokenu.
 *
 * @return  další token ze vstupu; pokud není více tokenů,
 *          vrací TOKEN_EOF; při chybě může vrátit ERROR_TOKEN
 */
Token lexer_token_next();
