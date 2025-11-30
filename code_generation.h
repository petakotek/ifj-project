#ifndef CODEGEN_H
#define CODEGEN_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "dynamic_stack.h"
#include "support_functions.h"
#include "symtable.h"
typedef struct {
    char                     *funcName;
    DynamiclabelStackStackPtr labelStack;
    char                     *labelEnd;
    int                       valueCounter;
    bool                      main;
} FuncContext;
void  emitProlog();
void  emitCommand(AstNode *node, FuncContext *funcContext);
char *generatorLable(char *func_name, FuncContext *funcContext, bool pridaniDoStacku, size_t counter);
void  codegenGenerate(AstNode *root);
char *resizeLabelName(char *prefix, size_t *size);
void  emitPlus(AstNode *node, FuncContext *funcContext);
void  emitMinus(AstNode *node, FuncContext *funcContext);
void  emitMul(AstNode *node, FuncContext *funcContext);
void  emitDiv(AstNode *node, FuncContext *funcContext);
void  emitAssigned(AstNode *node, FuncContext *funcContext);
void  emitVar(AstNode *node, FuncContext *funcContext);
void  emitNegace(AstNode *node, FuncContext *funcContex);
void  emitOr(AstNode *node, FuncContext *funcContext);
void  emitAnd(AstNode *node, FuncContext *funcContext);
void  emitEqualNot(AstNode *node, FuncContext *funcContext);
void  emitGreaterEqual(AstNode *node, FuncContext *funcContext);
void  emitSetterCall(AstNode *node);
void  emitGetterCall(AstNode *node);
void  emitSetter(AstNode *node);
void  emitGetter(AstNode *node);
void  emitCondition(AstNode *node, FuncContext *funcContext);
void  printN(const char *fmt, ...);

void         emitUnaryMinus(AstNode *node, FuncContext *funcStrukt);
FuncContext *ConstuctorFuncContext();

/**
 * @brief Funkcia pre generovanie kodu pustupnosti if a else funkcii
 * @param node aktualny uzol s ktorym pracujeme
 * @param funcContext struktura pre vytvorenie labla obsahujuca counter,...
 */
void emitIfChain(AstNode *node, FuncContext *funcContext);
/**
 * @brief Funkcia pre generovanie kodu if funkcii
 * @param node aktualny uzol s ktorym pracujeme
 * @param endlabel label konca if retazca
 * @param funcContext struktura pre vytvorenie labla obsahujuca counter,...
 */
void emitIf(AstNode *node, FuncContext *funcContext, char *endLabel);
/**
 * @brief Funkcia pre generovanie kodu else funkcii
 * @param node aktualny uzol s ktorym pracujeme
 * @param funcContext struktura pre vytvorenie labla obsahujuca counter,...
 */
void emitElse(AstNode *node, FuncContext *funcContext);

// Pomocná struktura pro ukládání proměnných behem projizdeni uzlem funkce
// abych si na zacatku funkce nadefinovali vsechny promenen ktere budou pouzity
typedef struct {
    char **varNames;  // pole jmen var proměnných
    int    count;      // počet proměnných pro resize
    int    capacity;   // kapacita pole pro resize
} VarList;
// Pomocna struktura pro ukladani si scopu a counter pro delani docasnych promenych abychom se vyhli konfiktum
#endif
