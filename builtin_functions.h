// TODO hlavička

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
void handleBuiltin(AstNode * ifjFunctionCall, FuncContext* context);

#endif

// TODO patička