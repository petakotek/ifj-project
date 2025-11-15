/*
 * Projekt IFJ25: Varianta zadání tabulka s rozptýlenými položkami s implicitním zřetězením položek
 * Autoři:
 * Soubor: main.c
 * Popis: Vstupni bod do programu spojuje jednotlivé moduly
 */
 #include "lexer.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"
#include "parser.h"
#include "ast_strom.h"
#include "support_functions.h"
#include "semantic_analysis.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SYMBOL_SIZE 201
#define LITERAL_SIZE 101
int main(){
	/*
	* Nacitani v vstupu
	* source_file je globalni promena extern je v lexer.h
	*/
	if(stdin == NULL){
		return 0;
	}
	source_file = NULL;
	//volani funcke na nacteni souboru
	source_file = read_all_stdin();
	if (source_file == NULL) {
		fprintf(stderr, "Chyba: selhalo načítání vstupů\n");
        return 1;
    }
	// inicializovani struktury staraji se o cteni ze souboru
	lexer_init(source_file);
	//print_and_free_tokens();
	//exit(0);
	// tvorba stromu pres parser
	AstNode *AST;
	AST = parser();
	//funkcia ast_dump -> funkcia na kontrolny vypis ast stromu

	ast_dump(AST);
	bool output;
	output = startSematicValidation(AST);
	printf("Vysledek sem_anal: %i\n", output);

	ast_dump(AST);
//	codegen_generate(AST);
	memorySafeExit(NO_ERROR);

}
