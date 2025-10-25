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
#include <string.h>

#include "lexer.h"
#include "support_functions.h"
#include "dynamic_stack.h"
#include "symbol.h"
#include "ast_strom.h"

#define NOT_IMPL(name) \
    do { fprintf(stderr, "[parser] %s not implemented\n", name); } while (0)

/**
 * @brief Definicia neterminálov
 */
typedef enum
{
    N_Program,
    N_Prolog,
    N_Main_class,
    N_Main_class_block,
    N_Block_content,
    N_Block,
    N_smth,
    N_smth_tail,
    N_smth_list,
    N_Oneline_expr_opt,
    N_Func_dec,
    N_Func_dec_tail,
    N_Param,
    N_Param_list,
    N_Func_call,
    N_Var,
    N_Var_EXT,
    N_Var_arg,
    N_Var_arg_list,
    N_Call_assign,
    N_Call_assign_list,
    N_Global_assign,
    N_If,
    N_Else,
    N_While,
    N_For,
    N_Break,
    N_Continue,
    N_Return,
    N_Return_expr,
    N_Else_body,
    N_Expr,
    N_New_line_check
} Nonterm;







/**
 * @brief pomocná funkcia na vytvorenie struktury terminalu
 * @param T Terminal z enum Term
 */
Symbol make_term(TokenType T);

/**
 * @brief pomocná funkcia na vytvorenie struktury neterminalu
 * @param N Neterminal z enum Nonterm
 */
Symbol make_nonterm(Nonterm N);

Symbol make_reduce(AstNodeType kind, size_t arity);

Symbol make_term_value(Token T);

void aststack_push(DynamicStackPtr S, struct AstNode *n);
struct AstNode* aststack_read(DynamicStackPtr S);
AstNode *parser();

void Program_func(DynamicStackPtr Stack, Token token);
void Prolog_func(DynamicStackPtr Stack, Token token);
void MainClass_func(DynamicStackPtr Stack, Token token);
void MainClassBlock_func(DynamicStackPtr Stack, Token token);

void Block_func(DynamicStackPtr Stack, Token token);
void BlockContent_func(DynamicStackPtr Stack, Token token);
void Smth_func(DynamicStackPtr Stack, Token token);
void SmthTail_func(DynamicStackPtr Stack, Token token);
void SmthList_func(DynamicStackPtr Stack, Token token);
void OnelineExprOpt_func(DynamicStackPtr Stack, Token token);

void FuncDec_func(DynamicStackPtr Stack, Token token);
void FuncDecTail_func(DynamicStackPtr Stack, Token token);
void Param_func(DynamicStackPtr Stack, Token token);
void ParamList_func(DynamicStackPtr Stack, Token token);
void FuncCall_func(DynamicStackPtr Stack, Token token);

void Var_func(DynamicStackPtr Stack, Token token);
void Var_EXT_func(DynamicStackPtr Stack, Token token);
void VarArg_func(DynamicStackPtr Stack, Token token);
void VarArgList_func(DynamicStackPtr Stack, Token token);

void CallAssign_func(DynamicStackPtr Stack, Token token);
void CallAssignList_func(DynamicStackPtr Stack, Token token);
void GlobalAssign_func(DynamicStackPtr Stack, Token token);

void If_func(DynamicStackPtr Stack, Token token);
void Else_func(DynamicStackPtr Stack, Token token);
void While_func(DynamicStackPtr Stack, Token token);
void For_func(DynamicStackPtr Stack, Token token);
void Break_func(DynamicStackPtr Stack, Token token);
void Continue_func(DynamicStackPtr Stack, Token token);

void Return_func(DynamicStackPtr Stack, Token token);
void ReturnExpr_func(DynamicStackPtr Stack, Token token);
void Expr_func(DynamicStackPtr Stack, Token token,Token *gLookahead, DynamicStackPtr gAstStack);
void ElseBody_func(DynamicStackPtr Stack, Token token);

#endif // PARSER_H
