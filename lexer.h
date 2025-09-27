#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
*Definice typu tokenu v lexeru
*/
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_NUMBER,

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
