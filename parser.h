/**
* Fakulta informačných technologií VUT Brno
*
* Hlavičkový súbor pre implementáciu lexikálnej analýzy
* pre projekt do predmetu IFJ
* @file  parser.h
* @author Jakub Jambrozi (xjambrj00)
* upravované: 22.10.2025
*/

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include "lexer.h"
#include "support_functions.h"
#include "dynamic_stack.h"
#include "symbol.h"

#define NOT_IMPL(name) \
    do { fprintf(stderr, "[parser] %s not implemented\n", name); } while (0)

/**
 * @brief Definicia neterminálov
 */
typedef enum
{
    N_Program,
    N_Prolog,
    N_MainClass,
    N_MainClassBlock,
    N_MainClassItem,
    N_Block,
    N_Smth,
    N_SmthTail,
    N_SmthList,
    N_FuncDec,
    N_Param,
    N_ParamList,
    N_Getter,
    N_Setter,
    N_Var,
    N_Var_EXT,
    N_Var_EXTSTAT,
    N_VarArg,
    N_VarArgList,
    N_Assign,
    N_If,
    N_Else,
    N_While,
    N_For,
    N_Break,
    N_Continue,
    N_Return,
    N_ReturnExpr,
    N_Expr
} Nonterm;



/**
 * @brief pomocná funkcia na vytvorenie struktury terminalu
 * @param T Terminal z enum Term
 */
Symbol make_term(TokenType T);

/**
 * @brief pomocná funkcia na vytvorenie struktury neterminalu
 * @param T Neterminal z enum Nonterm
 */
Symbol make_nonterm(Nonterm N);

Symbol make_term_value(Token T);
void parser();
void Program_func(DynamicStackPtr Stack, Token token);
void Prolog_func(DynamicStackPtr Stack, Token token);
void MainClass_func(DynamicStackPtr Stack, Token token);
void MainClassBlock_func(DynamicStackPtr Stack, Token token);
void MainClassItem_func(DynamicStackPtr Stack, Token token);
void Block_func(DynamicStackPtr Stack, Token token);
void Smth_func(DynamicStackPtr Stack, Token token);
void SmthTail_func(DynamicStackPtr Stack, Token token);
void SmthList_func(DynamicStackPtr Stack, Token token);
void FuncDec_func(DynamicStackPtr Stack, Token token);
void Param_func(DynamicStackPtr Stack, Token token);
void ParamList_func(DynamicStackPtr Stack, Token token);
void Getter_func(DynamicStackPtr Stack, Token token);
void Setter_func(DynamicStackPtr Stack, Token token);
void Var_func(DynamicStackPtr Stack, Token token);
void Var_EXT_func(DynamicStackPtr Stack, Token token);
void Var_EXTSTAT_func(DynamicStackPtr Stack, Token token);
void VarArg_func(DynamicStackPtr Stack, Token token);
void VarArgList_func(DynamicStackPtr Stack, Token token);
void Assign_func(DynamicStackPtr Stack, Token token);
void If_func(DynamicStackPtr Stack, Token token);
void Else_func(DynamicStackPtr Stack, Token token);
void While_func(DynamicStackPtr Stack, Token token);
void For_func(DynamicStackPtr Stack, Token token);
void Break_func(DynamicStackPtr Stack, Token token);
void Continue_func(DynamicStackPtr Stack, Token token);
void Return_func(DynamicStackPtr Stack, Token token);
void ReturnExpr_func(DynamicStackPtr Stack, Token token);
void Expr_func(DynamicStackPtr Stack, Token token);

#endif // PARSER_H
