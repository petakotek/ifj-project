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
 * @brief enum, ktorý obsahuje typy (názvy) uzlov
 */
typedef enum AstNodeType {
    // vstupné uzly programu
    AST_PROGRAM,
    AST_PROLOG,
    AST_MAIN_CLASS,
    AST_MAIN_CLASS_BLOCK,

    // blok a jeho príkazy
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

    // deklarovanie alebo priradenie premennej
    AST_VAR,
    AST_ASSIGN,

    // podmienkove vetvy
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
 * @brief štruktúra na uchovávanie hodnoty tokenu pre abstraktný syntaktický strom
 * @param intVal typ int -> uchovanie hodnoty tokenu pre celé číslo
 * @param floatVal typ float -> uchovanie hodnoty tokenu pre desatinné číslo
 * @param str typ *char -> uchovanie hodnoty tokenu pre premennú, globálnu premennú a pre reťazec
 * @param hasStr typ bool -> nastavíme true ak zapisujeme do str
 * @param isInt typ bool -> nastavíme true ak zapisujeme do intVal
 * @param isFloat typ bool -> nastavíme true ak zapisujeme do floatVal
 * @param isBool typ bool -> nastavíme true ak zapisujem AST_BOOL
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
 * @brief štruktúra uzla
 * @param kind typu AstNodeType -> typ uzla(názov)
 * @param arity typu size_t -> počet detí uzla
 * @param child typu Astnode **child -> ukazovateľ na list detí uzla
 * @param isLeaf typu bool podľa, ktorého vieme urcit ci sa jedna o list alebo o uzol
 * false -> uzol, true -> list
 * @param value typu Astvalue -> štruktúra, ktorá obsahuje konkrétne hodnoty pre listy
 * (teda používa sa ak isLeaf = true)
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
 * @param kind predstavuje typ listu ktorý sa má vytvoriť (názov)
 * @param token token, ktorého hodnotu chceme dať do listu
 * @return Astnode*
 */
AstNode *newLeaf(AstNodeType kind, Token token);

/**
 * @brief funkcia, ktorá vytvori novy uzol z vlastnosti parametrov
 * @param kind predstavuje typ listu ktorý sa má vytvoriť (názov)
 * @param arity počet detí nového uzla
 * @param kids ukazovateľ na detí nového uzla, ktorá sa mu majú prideliť
 * ich počet sa rovna arity
 * @return Astnode*, ktorý uz obsahuje všetky svoje detí
 */
AstNode *astNew(AstNodeType kind, size_t arity, AstNode **kids);

/**
 * @brief funkcia, ktorá uvoľni všetky uzly a ich detí
 * @param node koreň uzla, ktorý chceme uvoľniť
 */
void astFreeWhole(AstNode *node);

/**
 * @brief pomocná funkcia na vkladanie hodnoty do abstraktného syntaktického stromu.
 * vypĺňa štruktúru Astvalue podľa toho aky typ hodnoty chceme vkladať
 * lexikálny analyzátor každú hodnotu vráti ako string
 * pre 5.25 vracia TOKEN_FLOAT a jeho hodnotu "5.25"
 * funkcia zabezpečí prevedenie na double 5.25
 * @param out vysledna štruktúra AstValue ktoru potom pridame do abstraktného syntaktického stromu.
 * @param kind predstavuje typ uzla ktorý sa má vytvoriť (názov)
 * @param token token, z ktorého hodnotu chceme vložiť do abstraktného syntaktického stromu
 */
void astValueFromToken(AstValue *out, AstNodeType kind, Token *token);

/**
 * @brief pomocná funkcia na bezpečné vloženie retazca do abstraktného syntaktického stromu.
 * Vklada ho tam tak aby po uvoľnení scannera sa neuvoľní ani samotný reťazec
 * @param string reťazec, ktorý chceme vložiť
 */
char *dupCstr(const char *string);

/**
 * @brief pomocná funkcia pre výpis názvu uzla kde z AstNodetype spravíme reťazec
 * @param kind typ uzla (nazov), ktorý chceme vypísať
 */
const char *astNodeTypeToString(AstNodeType kind);

/**
 * @brief pomocná funkcia pre výpis celého stromu
 * @param root koreň uzla, ktorý chceme vypísať
 */
void     astDump(const AstNode *root);


/**
 * @brief pomocná funkcia, pre upratanie abstraktného syntaktického stromu
 * zo stromu parametrov v štýle param a
 *                                    param b
 *                                          param c
 * spraví jeden uzol param a jeho detmi pr. param a
 *                                                b
 *                                                c
 * @param n počet koľko detí ma mať uzol AST_PARAM
 * @param kids deti, ktoré má AST_PARAM obsahovať
 */
AstNode *buildParamFlat(size_t n, AstNode **kids);

/**
 * @brief pomocná funkcia, pre upratanie abstraktného syntaktického stromu
 * zo stromu statement listov v štýle statement list 
 *                                                  a
 *                                    statement list
 *                                                  b
 * spraví jeden uzol statement list a jeho detmi pr. statement list 
 *                                                                 a
 *                                                                 b
 * @param n počet koľko detí ma mať uzol AST_STATEMENT_LIST
 * @param kids deti, ktoré má AST_STATEMENT_LIST obsahovať
 */
AstNode *buildStmtListFlat(size_t n, AstNode **kids);

AstNode *buildClassBlockFlat(size_t n, AstNode **kids);

/**
 * @brief pomocná funkcia, pre upratanie abstraktného syntaktického stromu
 * odstránenie viacnásobnej if vetvy v štýle if cond a if cond b
 * spraví jeden if chain s uzlami obsahujúcimi if a else a ich deťmi 
 *                                                                
 * @param n počet koľko detí ma mať uzol AST_IF_CHAIN
 * @param kids deti, ktoré má AST_IF_CHAIN obsahovať
 */
AstNode *buildIfChainFlat(size_t n, AstNode **kids);

/**
 * @brief funkcia pre vytvorenie konkrétnej hodnoty pre prevedenie exponenciálneho čísla v reťazci 
 * na konkrétne vyčíslené číslo
 * z "1e1" spraví konkrétne číslo 10 a konkrétne číslo zapíše do listu vo funkcia astValueFromToken
 * @param s string, ktorý obsahuje exponenciálne číslo
 */
double   floatExp(const char *s);

/**
 * @brief zabezpečuje prevod reťazcov do potrebnej formy jazyka wren
 * @param tmp string, ktorý je potrebné previesť
 * */
char    *stringChanging(char *tmp);

/**
 * @brief zabezpečuje prevod reťazcov do potrebnej formy jazyka wren
 * volaní pri tokene AST_STRING, kedy je potrebné brať do úvahy aj escape sekvencie
 * najskôr zavolá funkciu escapeString a potom následne všeobecnú funkciu na prevod StringChanging
 * @param tmp string, ktorý je potrebné previesť
 * */
char    *stringChangingLit(char *tmp);

/**
 * @brief zabezpečuje prevod escape sekvencií v reťazci do potrebného formátu jazyka wren
 * @param tmp string, ktorý je potrebné previesť
 * */
char    *escapeString(char *tmp);

#endif

/*** konec ast.h ***/
