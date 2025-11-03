#ifndef CODEGEN_H
#define CODEGEN_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast_strom.h"
#include "symtable.h"
#include "dynamic_stack.h"
#include "support_functions.h"
void emit_prolog();
void emit_command(AstNode *node,DynamiclabelStackStackPtr label_stack,int counter,DynamicEmitOpStackPtr op_emit);
char *generatorLable(char *func_name, DynamiclabelStackStackPtr label_stack, int counter, bool pridaniDoStacku);
void codegen_generate(AstNode *root);
#endif
