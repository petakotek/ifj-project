// TODO hlavička

#ifndef SEMANTIC_ANALYSIS
#define SEMANTIC_ANALYSIS 1

#include "semantic_types.h"
#include "symtable.h"
#include "support_functions.h"
#include "ast_strom.h"
#include "dynamic_stack.h"

/**
 * @brief Funkce pro zahájení sémantické kontroly
 * 
 * @param root Vrchol abstraktního syntaktického stromu
 * @return true Sémantická kontrola je v pořádku
 * @return false Byla nalezena sémantická chyba
 */
bool startSematicValidation(AstNode* root);

/**
 * @brief Uklidí paměť používanou sémantickou analýzou
 * 
 */
void freeSematicsMemory();

#endif
// TODO patička