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
void emit_command(AstNode *node,DynamiclabelStackStackPtr label_stack,int counter,DynamicEmitOpStackPtr op_emit,char *func_name);
char *generatorLable(char *func_name, DynamiclabelStackStackPtr label_stack, int counter, bool pridaniDoStacku);
void codegen_generate(AstNode *root);
char *resize_label_name(char *prefix,size_t *size);
void emitPlus(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter, DynamicEmitOpStackPtr op_stack,char *func_name);
void emit_assigned(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter,
                   DynamicEmitOpStackPtr op_emit, char *func_name);

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
    int value_counter;
} tmpValue;
#endif
