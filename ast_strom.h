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
    AST_MAIN_CLASS_BLOCK,
    AST_FUNC_DEC,
    AST_GETTER,
    AST_SETTER,
    AST_BLOCK,
    AST_STATEMENT,
    AST_EXPR,
    AST_VAR,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_RETURN,
    AST_FUNC_CALL,
    AST_PARAM,
} AstNodeType;

typedef struct AstNode {
    AstNodeType type;
    char* name;

    struct AstNode** children;
    int children_count;

    union {
    	struct {
        	struct AstNode* prolog;
            struct AstNode* main_class;
           } Program;
     	struct {
      		char* import_string;
      	}Prolog;

       	struct {
        	struct AstNode* main_class_block;
        } Main_class;
        struct {
            struct AstNode** items;
        } Main_class_block;
        struct {
            char* func_name;
            struct AstNode* params;
            struct AstNode* block;
        } Func_dec;
        struct {
            char* getter_name;
            struct AstNode* block;
        } Getter;
        struct {
            char* setter_name;
            char* param_name;
            struct AstNode* block;
        } Setter;
        struct {
            struct AstNode** statements;
            int statements_count;
        } Block;
        struct {
            char* var_name;
            struct AstNode* var_ext;
        } Var;
        struct {
            char* assign_target;
            struct AstNode* expr;
        } Assign;
        struct {
                struct AstNode* condition;
                struct AstNode* if_block;
                struct AstNode* else_block;
        } If;
        struct {
                struct AstNode* condition;
                struct AstNode* block;
            } While;

        struct {
            char* iterator;
            struct AstNode* container_expr;
            struct AstNode* block;
        } For;

        struct {
            struct AstNode* return_expr;
        } Return;
        struct {
            struct AstNode* return_expr;
        } Break;

        struct {
            struct AstNode* return_expr;
        } Continue;
        struct {
            char* func_name;
            struct AstNode** arguments;
            int arguments_count;
        } Func_call;
//precedenční analýzy dodelat
        struct {
        } Expr;
    }data;

} AstNode;


AstNode new_leaf(AstNodeType k, Token t);
AstNode *ast_new(AstNodeType k, size_t nkids, AstNode **kids);
