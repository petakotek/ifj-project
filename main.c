/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Projekt IFJ25: Varianta zadání tabulka s rozptýlenými položkami s implicitním zřetězením položek
 * @file main.c
 * @author Vojtěch Voldřich (xvoldrv00), Jakub Jambrozi (xjambrj00), Petr Kotek (xkotekp00), Josef Pasek (xpasekj00)
 */

#include "ast.h"
#include "code_generation.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analysis.h"
#include "support_functions.h"

#define SYMBOL_SIZE 201
#define LITERAL_SIZE 101

int main() {
    // Načítání vstupu
    if (stdin == NULL) {
        return 0;
    }
    // Globální proměnná z v lexer.h
    source_file = NULL;
    // Volání funkce na načtení souboru
    source_file = readAllStdin();
    if (source_file == NULL) {
        printError(LEX_ERROR, "Selhalo načítání vstupů\n");
        memorySafeExit(LEX_ERROR);
    }
    // Inicializace struktury pro čtení ze souboru
    lexerInit(source_file);
    // tvorba stromu pres parser
    // print_and_free_tokens();
    //exit(1);
    // Syntaktická kontrola a sestavení AST stromu
    AstNode *ast = parser();

    // Sémantická kontrolo a úprava AST stromu
    startSematicValidation(ast);

    // Generování kódu
    codegenGenerate(ast);

    memorySafeExit(NO_ERROR);
}
/*** konec main.c ***/
