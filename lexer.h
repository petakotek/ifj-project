
/*
 * Fakulta informačních technologií VUT Brno
 *
 * Autoři: xpasekj00
 * Soubor: lexer.h
 * Popis:
 */
 #ifndef LEXER_H
 #define LEXER_H

#include <stdio.h>
/*
*Definice typu tokenu v lexeru
*/
//Stav do kterych se muze dostat
typedef enum{
	START_LEX, ///start
	ID,  ///Promena
	UNDERSCORE,
	GLOBAL_ID,
	NUMBER,
	FLOAT_NUMBER,
	ZERO,
	HEXA_NUM,
	FLOAT_EXP,
	DIGIT_EXP,
	NEG_DIGIT_EXP,
	STRING,
	STRING_BLOCK,
	STRING_HEX_N,
	BACKSLASH,
	LINE_COMMENT,
	BLOCK_COMMENT,
	STAR_COMMENT,
	SYMBOL,
	BLOCK_START,
	BLOCK_END,
	PAR_LEFT,
	PAR_RIGHT,
	PLUS,      // +
    MINUS,     // -
    SLASH,     // /
    ASSIGN,    // =
    EQ,        // ==
    LT,        // <
    GT,        // >
    START,	//*
    DOT, // .
    DOUBLE_DOT, // ..
    TRIPLE_DOT, // ...
    COMMA,
    LTE, // <=
    GTE ,// >=
    AND_HALF, //&
    AND, // &&
    OR_HALF, // |
    OR, // ||
    NEGACE, // !
    NOT, // !=
    DOUBLE_QUO,//""
    END_QUO,   //ukoncovaci stavy pro block string
    END_DOUBLE_QUO,
    COLON, // :
    QUESTION_MARK, // ?
    DOUBLE_MINUS, // --
    ZNAK_HEXA  //stav kdy ve stringu dostaneme hexa cislo a tim doplnime znak
//dopsat vsechny operatory jako stav a jako token
}StateType ;
//nějaky natrel co by tam mohlo byt
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_GLOBAL_ID,
    TOKEN_INTEGER,
    TOKEN_STRING,
    TOKEN_STRING_BLOCK,
    TOKEN_FLOAT,
    TOKEN_HEXA,
    TOKEN_INTEGER_EXP,
    TOKEN_FLOAT_EXP,
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
    TOKEN_DOT, // .
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_QUESTION_MARK,
    TOKEN_LTE,
    TOKEN_GTE,
    TOKEN_NOT_EQUAL, // !=
    TOKEN_COLON,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_ERROR,
    TOKEN_NEW_LINE,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_RETURN,   //dopsat true a dalsi
    TOKEN_KEYWORD_VAR,
    TOKEN_BLOCK_STAR,
    TOKEN_BLOCK_END,
    TOKEN_DOUBLE_MINUS,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_CLASS,
    TOKEN_KEYWORD_IFJ,
    TOKEN_KEYWORD_IN,
    TOKEN_KEYWORD_IMPORT,
    TOKEN_KEYWORD_NUM,
    TOKEN_KEYWORD_STRING,
    TOKEN_KEYWORD_NULL,
    TOKEN_KEYWORD_STATIC,
    TOKEN_KEYWORD_IS,
    TOKEN_KEYWORD_NULL_SMALL,
    TOKEN_KEYWORD_CONTINUE,
	TOKEN_KEYWORD_BREAK,
	TOKEN_COMMA,
	TOKEN_NOT,
    TOKEN_DOLLAR,
    TOKEN_STOP,
    TOKEN_EXPRESSION,
    TOKEN_PSA_END,
    TOKEN_DOUBLE_DOT,
    TOKEN_TRIPLE_DOT
} TokenType;

typedef union {
    char *identifier;
} TokenValue;

/*
*Definuje samotny jeden token
*obecne prametry tokenu: Line(pro error zpravy) .. vice mene nenapadlo
* Union pro specificke parametry daneho tokenu ty nejdulezitejsi hodnoty()
*/
typedef struct {
    TokenType type;
    TokenValue value;
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
int lexerInit(const char *source);
/*
 * Uvolní paměť spojenou s lexerem a nastaví ukazatel na NULL.
 */
void lexerFree(void);
/*
 * Hlavní funkce konečného automatu pro získání dalšího tokenu.
 *
 * @return  další token ze vstupu; pokud není více tokenů,
 *          vrací TOKEN_EOF; při chybě může vrátit ERROR_TOKEN
 */
Token lexerTokenNext();
TokenType matchIdentifikator(const char *identifikator);
char *tokenTypeToString(TokenType type);

Token isEffectivelyInteger(char *input,Token token);
extern char *source_file;
extern char *buf;
#endif

/*** konec soubor ***/
