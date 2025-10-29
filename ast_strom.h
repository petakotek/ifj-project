/*
 *
 * Autoři: xpasekj00, xjambrj00
 * Soubor: ast_strom.h
 * Popis: hlavickovy soubor pro ast strom, obsahuje strukturu node pro strom
 */

#ifndef AST_STROM_H
#define AST_STROM_H
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lexer.h"
#include "support_functions.h"
#include <stdio.h>
#include <assert.h>


typedef enum AstNodeType {
    // high-level
    AST_PROGRAM,
    AST_PROLOG,
    AST_MAIN_CLASS,
    AST_MAIN_CLASS_BLOCK,

    // bloky a zoznamy
    AST_BLOCK,
    AST_STATEMENT,
    AST_STATEMENT_LIST,

    // deklarácie a volania
    AST_FUNC_DEC,
    AST_PARAM,
    AST_FUNC_CALL,

    // premenné / priradenie
    AST_VAR,
    AST_ASSIGN,

    // riadiace štruktúry
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_BREAK,
    AST_CONTINUE,
    AST_RETURN,

    // listy
    AST_IDENTIFIER,
    AST_GLOBAL_ID,
    AST_INT_LIT,
    AST_FLOAT_LIT,
    AST_STRING_LIT,

	AST_EMPTY,

    // expression
    AST_PLUS,       // +
    AST_MINUS,      // -
    AST_MUL,        // *
    AST_DIV,        // /
    AST_EQ,         // ==
    AST_LT,         // <
    AST_GT,         // >
    AST_NOT,        // !
    AST_AND,        // &&
    AST_OR,         // ||
    AST_IS,         // is
    AST_GTE,        // >=
    AST_LTE,        // <=
} AstNodeType;


// Hodnota listu (len pre listové uzly)
typedef struct AstValue {
    long long int_val;
    double      float_val;
    char       *str;       // pre IDENTIFIER / GLOBAL_ID / STRING_LIT
    bool        has_str;
    bool        is_int;
    bool        is_float;
} AstValue;

typedef struct AstNode {
    AstNodeType kind;
    size_t      arity;
    struct AstNode **child;

    bool is_leaf;
    AstValue value;          // platný len ak is_leaf == true
} AstNode;


AstNode *new_leaf(AstNodeType kind, Token token);
AstNode *ast_new(AstNodeType kind, size_t arity, AstNode **kids);
void ast_free_whole(AstNode *node);

void ast_traverse(AstNode *node, int depth);
void ast_value_from_token(AstValue *out, AstNodeType kind, Token *token);
char *dup_cstr(const char *string);
const char *ast_node_type_to_string(AstNodeType kind);
void ast_dump(const AstNode *root);
AstNode *build_param_flat(size_t n, AstNode **kids);
AstNode *build_stmtlist_flat(size_t n, AstNode **kids);
AstNode *build_classblock_flat(size_t n, AstNode **kids);


#endif // AST_STROM_H
