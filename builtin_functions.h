/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Hlavičkový soubor pro implementaci výpisu kódu pro vestavěné funkce
 * @file builtin_functions.h
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#ifndef BUILTIN
#define BUILTIN

#include "ast_strom.h"
#include "codegen.h"

/**
 * @brief Funkce zajišťuje zpracování volání vestavěných funkcí
 *
 * @param ifjFunctionCall Uzel s voláním libovolné vestavěné funkce
 * @param context Kontext
 */
void handleBuiltin(AstNode* ifjFunctionCall, FuncContext* context);

#endif

/*** konec builtin_functions.h ***/
