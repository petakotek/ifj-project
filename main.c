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
	while (token.type != TOKEN_EOF ) {
		printf("%s \n",token_type_to_string(token.type));
		if(token.type == TOKEN_STRING ||token.type == TOKEN_STRING ||token.type == TOKEN_INTEGER || token.type == TOKEN_GLOBAL_ID){
		free(token.value.string);
		}
		token = lexer_token_next();
		if(token.type == TOKEN_ERROR){
			break;
		}
	}
	lexer_free();
	free(source);
}
