/*
 *
 * Autoři: xpasekj00
 * Soubor: ast_strom.h
 * Popis: hlavickovy soubor pro ast strom, obsahuje strukturu node pro strom
 */
#pragma once
#include "lexer.h"
#include <stdbool.h>
/*
 *
 * Autoři: xpasekj00
 * Soubor: ast_strom.h
 * Popis:
 */
 typedef enum {
     AST_PROGRAM,
     AST_PROLOG,
     AST_MAIN_CLASS,
     AST_FUNC_DEC,
     AST_GETTER,
     AST_SETTER,
     AST_BLOCK,
     AST_VAR,
     AST_ASSIGN,
     AST_IF,
     AST_WHILE,
     AST_FOR,
     AST_RETURN,
     AST_BREAK,
     AST_CONTINUE,
     AST_FUNC_CALL,
     AST_PARAM,      // Reprezentuje jeden parametr
     AST_PARAM_LIST, // Reprezentuje seznam parametrů (doplněno)
     AST_EXPR,       // Uzel pro výsledky precedenční analýzy (binární, unární, literál)
     AST_STATEMENT_LIST, // Pro Main_class_block a Block (doplněno)
     AST_STRING_LIT,
     AST_INT_LIT,
     AST_FLOAT_LIT
 } AstNodeType;

 typedef struct AstNode {
     AstNodeType type;

     // Obecné datové pole pro jména, identifikátory a literály.
     union {
         char *identifier_name;
         long long int_value;
         double float_value;
         char *string_value;
     } value;

     // Standardizované pole pro DĚTI (uzly se seznamy)
     // Používá se pro AST_BLOCK, AST_STATEMENT_LIST, AST_FUNC_CALL (argumenty)
     struct AstNode **children;
     int children_count;

     // Union pro pevně strukturované uzly
     union {
	     struct {
	           struct AstNode *prolog;
	           struct AstNode *main_class;
	       } Program;

	       struct {
	           char *import_string; // "ifj25"
	       } Prolog;

	       struct {
	           char *class_name;
	           struct AstNode *body; // AST_STATEMENT_LIST
	       } Main_class;

	       struct {
	           struct AstNode *params; // AST_PARAM_LIST
	           struct AstNode *block;
	       } Func_dec;

	       struct {
	           struct AstNode *block;
	       } Getter;

	       struct {
	           char *param_name;
	           struct AstNode *block;
	       } Setter;

	       struct {
	           struct AstNode *condition;
	           struct AstNode *if_block;
	           struct AstNode *else_block;
	       } If;

	       struct {
	           struct AstNode *condition;
	           struct AstNode *block;
	       } While;

	       struct {
	           char *iterator_name;
	           struct AstNode *container_expr;
	           struct AstNode *block;
	       } For;

	       struct {
	           struct AstNode *target_var;
	           struct AstNode *expr;
	           bool is_global; // přidané
	       } Assign;

	       struct {
	           struct AstNode *return_expr; // NULL pokud eps
	       } Return;

	       struct {
	           char *identifier_name;
	           struct AstNode *var_ext; // přiřazení nebo NULL
	       } Var;

	       struct {
	           char *function_name;
	           struct AstNode *arguments; // AST_PARAM_LIST nebo AST_STATEMENT_LIST
	       } Func_call;
     } data;

 } AstNode;

 void ast_free_single(AstNode *kids);
AstNode *new_leaf(AstNodeType k, Token t);
AstNode *ast_new(AstNodeType k, size_t nkids, AstNode **kids);
