/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Hlavičkový soubor implementace generátoru kódu
 * @file code_generation.c
 * @author Josef Pasek (xpasekj00), Petr Kotek (xkotekp00), Jakub Jambrozi(xjambrj00)
 */


#ifndef CODEGEN_H
#define CODEGEN_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "dynamic_stack.h"
#include "support_functions.h"
#include "symtable.h"
/*
* Struktura na uložení informaci o funkci ve ktere se generator kodu nachází.
*/
typedef struct {
    char                     *funcName;
    DynamiclabelStackStackPtr labelStack;
    char                     *labelEnd;
    int                       valueCounter;
    bool                      main;
} FuncContext;
void  emitProlog();
void  emitCommand(AstNode *node, FuncContext *funcContext);
char *generatorLabel(char *func_name, FuncContext *funcContext, bool addToStack, size_t counter);
void  codegenGenerate(AstNode *root);
char *resizeLabelName(char *prefix, size_t *size);
void  emitPlus(AstNode *node, FuncContext *funcContext);
void  emitMinus(AstNode *node, FuncContext *funcContext);
void  emitMul(AstNode *node, FuncContext *funcContext);
void  emitDiv(AstNode *node, FuncContext *funcContext);
void  emitAssigned(AstNode *node, FuncContext *funcContext);
void  emitVar(AstNode *node, FuncContext *funcContext);
void  emitNegace(AstNode *node, FuncContext *funcContext);
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

void         emitUnaryMinus(AstNode *node, FuncContext *funcStruct);
FuncContext *ConstructorFuncContext();

/**
 * @brief Funkcia pre generovanie kodu retezu if a else funkcii
 * @param node aktualny uzol s ktorym pracujeme
 * @param funcContext struktura pre vytvorenie navesti obsahujuca counter,...
 */
void emitIfChain(AstNode *node, FuncContext *funcContext);
/**
 * @brief Funkcia pre generovanie kodu if funkcii
 * @param node aktualny uzol s ktorym pracujeme
 * @param endlabel label konca if retazca
 * @param funcContext struktura pre vytvorenie navesti obsahujuca counter,...
 */
void emitIf(AstNode *node, FuncContext *funcContext, char *endLabel);
/**
 * @brief Funkcia pre generovanie kodu else funkcii
 * @param node aktualny uzol s ktorym pracujeme
 * @param funcContext struktura pre vytvorenie navesti obsahujuca counter,...
 */
void emitElse(AstNode *node, FuncContext *funcContext);

// Pomocná struktura pro ukládání proměnných behem projizdeni uzlem funkce
// abych si na zacatku funkce nadefinovali vsechny promenen ktere budou pouzity
typedef struct {
    char **varNames;  // pole jmen var proměnných
    int    count;      // počet proměnných pro resize
    int    capacity;   // kapacita pole pro resize
} VarList;
// Pomocna struktura pro ukladani si ramce a counter pro delani docasnych proměnných abychom se vyhli konfliktům
#endif

/* Konec code_generation.h */
