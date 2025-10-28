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



/**
 * @brief enum ktory obsahuje typy (nazvi) uzlov
 */
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
} AstNodeType;


/**
 * @brief struktura na uchovavanie hodnoty tokenu pre abstraktny syntakticky strom
 * @param int_value typ int -> uchovanie hodnoty tokenu pre cele cislo 
 * @param float_val typ float -> uchovanie hodnoty tokenu pre desatine cislo
 * @param str typ *char -> uchovanie hodnoty tokenu pre premennu, globalnu premennu a pre retazec
 * @param has_str typ bool -> nastavime true ak zapisujeme do str
 * @param is_int typ bool -> nastavime true ak zapisujeme do int_val
 * @param is_float typ bool -> nastavime true ak zapisujeme do float_val
 *  
 */
typedef struct AstValue {
    int         int_val;
    float       float_val;
    char       *str;       // pre IDENTIFIER / GLOBAL_ID / STRING_LIT
    bool        has_str;
    bool        is_int;
    bool        is_float;
} AstValue;

/**
 * @brief struktura uzla
 * @param kind typu AstNodeType -> typ uzla(názov)
 * @param arity typu size_t -> pocet deti uzla
 * @param child typu Astnode **child -> ukazel na list deti uzla
 * @param is_leaf typu bool podla ktoreho vieme urcit ci sa jedna o list alebo o uzol
 * false -> uzol, true -> list
 * @param value typu Astvalue -> struktura ktora obsahuje konkretne hodnoty pre listy
 * (teda pouziva sa ak is_leaf = true)
 */
typedef struct AstNode {
    AstNodeType kind;
    size_t      arity;
    struct AstNode **child;

    bool is_leaf;
    AstValue value;          // platný len ak is_leaf == true
} AstNode;


/**
 * @brief funkcia na vytvorenie listu aj s konkretnou hodnotou
 * očakáva sa že tento list nemože byť uzlom s dalšími listami
 * @param kind predstavuje typ listu ktorý sa ma vytvoriť (názov)
 * @param token token ktoreho hodnotu chceme dat do listu
 * @return Astnode*
*/
AstNode *new_leaf(AstNodeType kind, Token token);

/**
 * @brief funkcia ktora vytvori novy uzol z vlastnosti parametrov
 * @param kind predstavuje typ listu ktorý sa ma vytvoriť (názov)
 * @param arity pocet deti noveho uzla
 * @param kids ukazel na deti noveho uzla ktora sa mu maju pridelit
 * ich pocet sa rovna arity
 * @return Astnode* ktory uz obsahuje vsetky svoje deti
 */
AstNode *ast_new(AstNodeType kind, size_t arity, AstNode **kids);

/**
 * @brief funkcia ktora uvolni vsetky uzly a ich deti
 * @param node koren uzla ktory chceme uvolnit
 */
void ast_free_whole(AstNode *node);

/**
 * @brief pomocna funkcia na vkladanie hodnoty do abstraktneho syntaktickeho stromu.
 * vyplna strukturu Astvalue podla toho aky typ hodnoty chceme vkladat
 * @param out vysledna struktura AstValue ktoru potom pridame do abstraktneho syntaktickeho stromu.
 * @param kind predstavuje typ uzla ktorý sa ma vytvoriť (názov)
 * @param token token z ktoreho hodnotu chceme vlozit do abstraktneho syntaktickeho stromu
 */
void ast_value_from_token(AstValue *out, AstNodeType kind, Token *token);

/**
 * @brief pomocna funkcia na bezpecne vlozenie retazca do abstraktneho syntaktickeho stromu.
 * Vklada ho tam tak aby po uvolneni scannera sa neuvolni ani samotny retazec
 * @param string retazec ktory chceme vlozit
 */
char *dup_cstr(const char *string);

/**
 * @brief pomocna funkcia pre vypis nazvu uzla kde z AstNodetype spravime retazec
 * @param kind typ uzla (nazov) ktory chceme vypisat
 */
const char *ast_node_type_to_string(AstNodeType kind);

/**
 * @brief pomocna funkcia pre vypis celeho stromu
 * @param root koren uzla ktory chceme vypisat
 */
void ast_dump(const AstNode *root);
AstNode *build_param_flat(size_t n, AstNode **kids);
AstNode *build_stmtlist_flat(size_t n, AstNode **kids);
AstNode *build_classblock_flat(size_t n, AstNode **kids);


#endif // AST_STROM_H
