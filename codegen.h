#ifndef CODEGEN_H
#define CODEGEN_H
#include "ast_strom.h"
#include "symtable.h"
void emit_prolog();
void emit_globals(HashTable *symtab);
void emit_command(AstNode *node, HashTable *hash_table);

#endif
