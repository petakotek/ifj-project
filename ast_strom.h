#pragma once
#include "lexer.h"
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
             struct AstNode *params; // Bude to AST_PARAM_LIST
             struct AstNode *block;
         } Func_dec;

         // NK_IF a NK_WHILE využívají standardizované pojmenování:
         struct {
             struct AstNode *condition;
             struct AstNode *if_block;
             struct AstNode *else_block; // NULL, pokud chybí
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
             struct AstNode *target_var; // Cíl přiřazení (proměnná, volání funkce, atd.)
             struct AstNode *expr;       // Pravá strana
         } Assign;

     } data;

 } AstNode;

 void ast_free_single(AstNode *kids);
AstNode new_leaf(AstNodeType k, Token t);
AstNode *ast_new(AstNodeType k, size_t nkids, AstNode **kids);
