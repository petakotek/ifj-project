/*
 * Projekt IFJ25: Varianta zadání tabulka s rozptýlenými položkami s implicitním zřetězením položek
 * Autoři:
 * Soubor: main.c
 * Popis: Vstupni bod do programu spojuje jednotlivé moduly
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "hashtable.h"
#define SYMBOL_SIZE 201
#define LITERAL_SIZE 101
int main(){
	fseek(stdin, 0, SEEK_END);
	size_t len =ftell(stdin);
	fseek(stdin, 0, SEEK_SET);
	char *source = malloc(len +1);
	fread(source, 1, len, stdin);
	source[len] = '\0';
	lexer_init(source);
	//testovani
	Token token;
	token = lexer_token_next();
	while (token.type != TOKEN_EOF) {
		if(token.type == TOKEN_ERROR){
   			printf("%s\n", token_type_to_string(token.type));
			break;
		}
		if(token.type == TOKEN_STRING || token.type == TOKEN_STRING_BLOCK){
			free(token.value.string);
		}else if (token.type == TOKEN_INTEGER || token.type == TOKEN_HEXA || token.type == TOKEN_FLOAT || token.type == TOKEN_FLOAT_EXP || token.type == TOKEN_INTEGER_EXP) {
		}else if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_KEYWORD_VAR || token.type == TOKEN_KEYWORD_IF || token.type == TOKEN_KEYWORD_FOR || token.type == TOKEN_KEYWORD_WHILE || token.type == TOKEN_KEYWORD_ELSE || token.type == TOKEN_GLOBAL_ID) {
			free(token.value.identifier);
		}
		printf("%s\n", token_type_to_string(token.type));
  		token = lexer_token_next();
	}if (token.type == TOKEN_ERROR) {
	    fprintf(stderr, "Lexer error on line %d.\n", token.line);
	}
	lexer_free();
	free(source);
}
