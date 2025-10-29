#ifndef CODEGEN_H
#define CODEGEN_H
#include "ast_strom.h"
#include "symtable.h"
void emit_prolog();
void emit_command(AstNode *node);
#endif
