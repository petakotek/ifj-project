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
	// TODO předělat PSA lgging na prekladovy flag misto proprietární proměnné
	source_file = NULL;
	//volani funcke na nacteni souboru
	source_file = readAllStdin();
	if (source_file == NULL) {
		printError(LEX_ERROR, "Selhalo načítání vstupů\n");
        memorySafeExit(LEX_ERROR);
    }
	// inicializovani struktury staraji se o cteni ze souboru
	lexerInit(source_file);
	/*
	print_and_free_tokens();
	exit(0);
	*/
	// tvorba stromu pres parser
	AstNode *ast = parser();
	//astDump(AST);
	//funkcia astDump -> funkcia na kontrolny vypis ast stromu

	bool output;
	output = startSematicValidation(ast);
	printLog("Vysledek sem_anal: %i\n", output);

	//astDump(AST);

	codegenGenerate(ast);

	memorySafeExit(NO_ERROR);
}
