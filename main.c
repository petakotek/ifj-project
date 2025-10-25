/*
 * Projekt IFJ25: Varianta zadání tabulka s rozptýlenými položkami s implicitním zřetězením položek
 * Autoři:
 * Soubor: main.c
 * Popis: Vstupni bod do programu spojuje jednotlivé moduly
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#define SYMBOL_SIZE 201
#define LITERAL_SIZE 101
int main(){
	/*
	* Nacitani v vstupu
	* source_file je globalni promena extern je v lexer.h
	*/
	fseek(stdin, 0, SEEK_END);
	size_t len =ftell(stdin);
	fseek(stdin, 0, SEEK_SET);
	char *source_file = malloc(len +1);
	fread(source_file, 1, len, stdin);
	source_file[len] = '\0';
	lexer_init(source_file);
	//print_and_free_tokens();
	AstNode *AST;
	AST = parser();
	lexer_free();
	free(source_file);
}
