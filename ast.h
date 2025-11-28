/** 
 * Fakulta informačných technologií VUT Brno
 * 
 * @brief hlavičkový súbor pre abstraktný syntaktický strom, 
 * @file ast.h
 * @author: Jakub Jambrozi (xjambrj00)
 */

#ifndef AST_STROM_H
#define AST_STROM_H

#include <stdbool.h>
#include <stdlib.h>

#include "lexer.h"

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
    AST_STATEMENT_LIST,

    // deklarácie a volania
    AST_FUNC_DEC,
    AST_PARAM,
    AST_FUNC_CALL,
    AST_IFJ_CALL,
    AST_GETTER,
    AST_SETTER,
    AST_GETTER_CALL,
    AST_SETTER_CALL,

    // premenné / priradenie
    AST_VAR,
    AST_ASSIGN,

    // riadiace štruktúry
    AST_IF,
    AST_ELSE,
    AST_IF_CHAIN,
    AST_CONDITION,
    AST_WHILE,
    AST_FOR,
    AST_BREAK,
    AST_CONTINUE,
    AST_RETURN,
    AST_TERNARY,

    // listy
    AST_IDENTIFIER,
    AST_GLOBAL_ID,
    AST_INT_LIT,
    AST_FLOAT_LIT,
    AST_STRING_LIT,
    AST_BOOL_LIT,

    AST_EMPTY,

    // expression
    AST_PLUS,        // +
    AST_MINUS,       // -
    AST_MUL,         // *
    AST_DIV,         // /
    AST_EQ,          // ==
    AST_NEQ,         // !=
    AST_LT,          // <
    AST_GT,          // >
    AST_NOT,         // !
    AST_AND,         // &&
    AST_OR,          // ||
    AST_IS,          // is
    AST_GTE,         // >=
    AST_LTE,         // <=
    AST_LPAREN,      // (
    AST_RPAREN,      // )
    AST_NULL,        // Prázdný blok
    AST_TRIPLE_DOT,  // ...
    AST_DOUBLE_DOT,  // ..

    AST_KEYWORD_NUM,     // Num
    AST_KEYWORD_STRING,  // String
    AST_SMALL_NULL,
    AST_FLOAT_LIT_EXP,
    AST_INT_LIT_EXP,
    AST_STRING_BLOCK,
} AstNodeType;

/**
 * @brief struktura na uchovavanie hodnoty tokenu pre abstraktny syntakticky strom
 * @param intVal typ int -> uchovanie hodnoty tokenu pre cele cislo
 * @param floatVal typ float -> uchovanie hodnoty tokenu pre desatine cislo
 * @param str typ *char -> uchovanie hodnoty tokenu pre premennu, globalnu premennu a pre retazec
 * @param hasStr typ bool -> nastavime true ak zapisujeme do str
 * @param isInt typ bool -> nastavime true ak zapisujeme do intVal
 * @param isFloat typ bool -> nastavime true ak zapisujeme do floatVal
 * @param isBool typ bool -> nastavime true ak zapisujem AST_BOOL
 *
 */
typedef struct AstValue {
    long long intVal;
    double    floatVal;
    char     *str;  // pre IDENTIFIER / GLOBAL_ID / STRING_LIT / BOOl_LIT
    bool      hasStr;
    bool      isInt;
    bool      isFloat;
    bool      isBool;
} AstValue;

/**
 * @brief struktura uzla
 * @param kind typu AstNodeType -> typ uzla(názov)
 * @param arity typu size_t -> pocet deti uzla
 * @param child typu Astnode **child -> ukazovateľ na list deti uzla
 * @param isLeaf typu bool podla ktoreho vieme urcit ci sa jedna o list alebo o uzol
 * false -> uzol, true -> list
 * @param value typu Astvalue -> štruktúra ktora obsahuje konkretne hodnoty pre listy
 * (teda pouziva sa ak isLeaf = true)
 */
typedef struct AstNode {
    AstNodeType      kind;
    size_t           arity;
    struct AstNode **child;
    bool             isLeaf;
    AstValue         value;  // platný len ak isLeaf == true
    // pre generovanie dočasných premenných používa iba codegen
    int tmpId;
} AstNode;

/**
 * @brief funkcia na vytvorenie listu aj s konkretnou hodnotou
 * očakáva sa že tento list nemôže byť uzlom s dalšími listami
 * @param kind predstavuje typ listu ktorý sa ma vytvoriť (názov)
 * @param token token ktoreho hodnotu chceme dat do listu
 * @return Astnode*
 */
AstNode *newLeaf(AstNodeType kind, Token token);

/**
 * @brief funkcia ktora vytvori novy uzol z vlastnosti parametrov
 * @param kind predstavuje typ listu ktorý sa ma vytvoriť (názov)
 * @param arity pocet deti noveho uzla
 * @param kids ukazovateľ na deti noveho uzla ktora sa mu maju pridelit
 * ich pocet sa rovna arity
 * @return Astnode* ktory uz obsahuje vsetky svoje deti
 */
AstNode *astNew(AstNodeType kind, size_t arity, AstNode **kids);

/**
 * @brief funkcia ktora uvolni vsetky uzly a ich deti
 * @param node koren uzla ktory chceme uvolnit
 */
void astFreeWhole(AstNode *node);

/**
 * @brief pomocna funkcia na vkladanie hodnoty do abstraktneho syntaktického stromu.
 * vyplna strukturu Astvalue podla toho aky typ hodnoty chceme vkladat
 * @param out vysledna struktura AstValue ktoru potom pridame do abstraktneho syntaktickeho stromu.
 * @param kind predstavuje typ uzla ktorý sa ma vytvoriť (názov)
 * @param token token z ktoreho hodnotu chceme vlozit do abstraktneho syntaktickeho stromu
 */
void astValueFromToken(AstValue *out, AstNodeType kind, Token *token);

/**
 * @brief pomocna funkcia na bezpecne vlozenie retazca do abstraktneho syntaktickeho stromu.
 * Vklada ho tam tak aby po uvolneni scannera sa neuvolni ani samotny retazec
 * @param string retazec ktory chceme vlozit
 */
char *dupCstr(const char *string);

/**
 * @brief pomocna funkcia pre vypis nazvu uzla kde z AstNodetype spravime retazec
 * @param kind typ uzla (nazov) ktory chceme vypisat
 */
const char *astNodeTypeToString(AstNodeType kind);

/**
 * @brief pomocna funkcia pre vypis celeho stromu
 * @param root koren uzla ktory chceme vypisat
 */
void     astDump(const AstNode *root);
AstNode *buildParamFlat(size_t n, AstNode **kids);
AstNode *buildStmtListFlat(size_t n, AstNode **kids);
AstNode *buildClassBlockFlat(size_t n, AstNode **kids);
AstNode *buildIfChainFlat(size_t n, AstNode **kids);
double   floatExp(const char *s);
char    *stringChanging(char *tmp);
char    *stringChangingLit(char *tmp);
char    *escapeString(char *tmp);

#endif

/*** konec ast.h ***/
