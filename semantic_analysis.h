// TODO hlavička

#ifndef SEMANTIC_ANALYSIS
#define SEMANTIC_ANALYSIS 1

#include "semantic_types.h"
#include "symtable.h"
#include "support_functions.h"
#include "ast_strom.h"
#include "dynamic_stack.h"


bool startSematicValidation(AstNode* root);

/**
 * @brief 
 * 
 */
void freeSematicsMemory();

#endif
// TODO patička