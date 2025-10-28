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
#include "expr.c"

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
    N_New_line_check,
    N_Opt_newlines
} Nonterm;

AstNode* parser();

/**
 * @brief pomocná funkcia na vytvorenie struktury terminalu
 * @param T Terminal z enum Term
 */
Symbol make_term(TokenType T);

/**
 * @brief pomocná funkcia pre vytvorenie struktúry termu
 * pri ktorom potrebujeme uchovať value
 * @param type typu tokenu z ktorého hodnotu chceme uchovať
 */
Symbol make_term_with_value(TokenType type);

/**
 * @brief pomocná funkcia na vytvorenie struktury neterminalu
 * @param N Neterminal z enum Nonterm
 */
Symbol make_nonterm(Nonterm N);

/**
 * @brief pomocná funkcia na vytvorenie strukturý Symbol ktorá
 * bude obsahovať aritu detí a is_reduced = true
 * @param kind nazov neterminálu ktorý strom obsahovať
 * @param arity počet detí prisluchaný k danému uzlu
 */
Symbol make_reduce(AstNodeType kind, size_t arity);

/**
 * @brief pomocná funkcia na overenie správnosti konkrétnej hodnoty tokenu
 * @param token token z ktorého potrebujeme prečitať konkrétnu hodnotus
 */
Symbol make_term_value(Token token);


/**
 * @brief kontrolná funkcia zda prebehlo pushnovanie na zásobík správne
 * @param boolean bool returnutý funkciou po pushovaní
 */
void push_check(bool boolean);


/*-----------------------------------POMOCNÉ FUNKCIE PRE NETERMÍNALY-------------------------------------------------------*/

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Program
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Program_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Prolog
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Prolog_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Main_class
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Main_class_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Main_class_block
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Main_class_block_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Block
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Block_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Block_content
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Block_content_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Smth
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void smth_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom smth_tail
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void smth_tail_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom smth_list
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void smth_list_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Oneline_expr_opt
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Oneline_expr_opt_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Func_dec
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Func_dec_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Func_dec_tail
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Func_dec_tail_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Param_func
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Param_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Param_list
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Param_list_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Func_call
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Func_call_func(DynamicSymbolStackPtr Stack, Token token);


/**
 * @brief Funkcia na zaobchádzanie s netermínalom Var
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Var_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom var_ext
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Var_EXT_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Var_arg
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Var_arg_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Var_arg_list
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void var_arg_list_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Call_assign
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Call_assign_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Call_assign_list
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Call_assign_list_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Global_assign
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Global_assign_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom If
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void If_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Else
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Else_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom While
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void While_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom For
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void For_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Break
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Break_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Continue
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Continue_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Return
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Return_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Return_expr
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Return_expr_func(DynamicSymbolStackPtr Stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Expr
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void Expr_func(DynamicSymbolStackPtr Stack, Token token,Token *gLookahead, DynamicAstNodeStackPtr gAstStack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom ElseBody
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void ElseBody_func(DynamicSymbolStackPtr Stack, Token token);


/**
 * @brief Funkcia na zaobchádzanie s netermínalom New_line
 * @param Stack Inicializovaný Stack na ktorom sú strukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void  New_line_func(DynamicSymbolStackPtr Stack, Token token);

void Opt_newlines_func(DynamicSymbolStackPtr Stack, Token token);

Symbol make_term_value_and_capture_identifier(const char *exact);

#endif // PARSER_H
