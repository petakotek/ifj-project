#ifndef CODEGEN_H
#define CODEGEN_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast_strom.h"
#include "symtable.h"
#include "dynamic_stack.h"
#include "support_functions.h"
typedef struct {
    char *func_name;
    DynamiclabelStackStackPtr label_stack;
    int counter;
    char *label_end;
    int value_counter;
} FuncContext;
void emit_prolog();
void emit_command(AstNode *node,FuncContext *funcContext);
char *generatorLable(char *func_name,FuncContext *funcContext, bool pridaniDoStacku);
void codegen_generate(AstNode *root);
char *resize_label_name(char *prefix,size_t *size);
void emitPlus(AstNode *node, FuncContext *funcContext);
void emitMinus(AstNode *node, FuncContext *funcContext);
void emitMul(AstNode *node, FuncContext *funcContext);
void emitDiv(AstNode *node, FuncContext *funcContext);
void emit_assigned(AstNode *node, FuncContext *funcContext);
void emit_var(AstNode *node, FuncContext *funcContext);
void emit_assigned(AstNode *node, FuncContext *funcContext);
void emit_negace(AstNode *node, FuncContext *funcContex);
void emit_or(AstNode *node, FuncContext *funcContext);
void emit_and(AstNode *node, FuncContext *funcContext);
void emit_equal_not(AstNode *node, FuncContext *funcContext);
void emit_greater_equal(AstNode *node, FuncContext *funcContext);
void emitIfChain(AstNode *node, FuncContext *funcContext);
void emitIf(AstNode *node, FuncContext *funcContext, char *endLabel);
void emitElse(AstNode *node, FuncContext *funcContext);

// Pomocná struktura pro ukládání proměnných behem projizdeni uzlem funkce
// abych si na zacatku funkce nadefinovali vsechny promenen ktere budou pouzity
typedef struct {
    char **var_names;  // pole jmen var proměnných
    int count;         // počet proměnných pro resize
    int capacity;      // kapacita pole pro resize
} VarList;
//Pomocna struktura pro ukladani si scopu a counter pro delani docasnych promenych abychom se vyhli konfiktum
typedef struct {
    char* func_name;
} tmpValue;
#endif
