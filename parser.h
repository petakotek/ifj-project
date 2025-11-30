/**
 * Fakulta informačných technológií VUT Brno
 *
 * @brief Hlavičkový súbor pre implementáciu lexikálnej analýzy
 * pre projekt do predmetu IFJ
 * @file  parser.h
 * @author Jakub Jambrozi (xjambrj00)
 */

#ifndef PARSER_H
#define PARSER_H

#include "dynamic_stack.h"

/**
 * @brief Definicia neterminálov
 */
typedef enum {
    N_PROGRAM,
    N_PROLOG,
    N_MAIN_CLASS,
    N_MAIN_CLASS_BLOCK,
    N_BLOCK_CONTENT,
    N_BLOCK,
    N_STMT_TAIL,
    N_STMT_LIST,
    N_FUNC_DEC,
    N_FUNC_DEC_TAIL,
    N_PARAM,
    N_PARAM_LIST,
    N_VAR,
    N_VAR_EXT,
    N_VAR_ARG,
    N_VAR_ARG_LIST,
    N_CALL_ASSIGN,
    N_CALL_ASSIGN_LIST,
    N_GLOBAL_ASSIGN,
    N_IF,
    N_WHILE,
    N_FOR,
    N_BREAK,
    N_CONTINUE,
    N_RETURN,
    N_RETURN_EXPR,
    N_EXPR,
    N_NEW_LINE_CHECK,
    N_OPT_NEWLINES,
    N_L_EXPR,
    N_C_EXPR,
    N_BLOCK_CONTENT_CHECK,
    N_O_EXPR,
    N_CALL_IFJ,
    N_ELSE_CHAIN,
    N_ELSE_ITEM,
    N_CALL_IFJ_CHECK,
    N_ASSIGN_IFJ_ID_CHECK,
} Nonterm;

AstNode *parser();

/**
 * @brief pomocná funkcia na vytvorenie štruktúry terminalu
 * @param tokenType Terminal z enum Term
 */
Symbol makeTerm(TokenType tokenType);

/**
 * @brief pomocná funkcia pre vytvorenie štruktúry termu
 * pri ktorom potrebujeme uchovať value
 * @param type typu tokenu z ktorého hodnotu chceme uchovať
 */
Symbol makeTermWithValue(TokenType type);

/**
 * @brief pomocná funkcia na vytvorenie štruktúry neterminálu
 * @param nonTerm Neterminal z enum Nonterm
 */
Symbol makeNonterm(Nonterm nonTerm);

/**
 * @brief pomocná funkcia na vytvorenie štruktúry Symbol ktorá
 * bude obsahovať počet detí a is_reduced = true
 * @param kind nazov neterminálu ktorý strom obsahovať
 * @param arity počet detí príslušný k danému uzlu
 */
Symbol makeReduce(AstNodeType kind, size_t arity);

/**
 * @brief pomocná funkcia na overenie správnosti konkrétnej hodnoty tokenu
 * @param token token z ktorého potrebujeme prečítať konkrétnu hodnotu
 */
Symbol makeTermValue(Token token);

/**
 * @brief pomocná funkcia vytvorenie štruktúry Symbol, ktorá bude obsahovať
 * konkrétnu hodnotu z token.value a konkrétne značky pre zachytenie tejto hodnoty
 * @param token token z ktorého potrebujeme prečítať konkrétnu hodnotu
 */
Symbol makeTermValueAndCaptureIdentifier(const char *exact);

/**
 * @brief kontrolná funkcia zda prebehol vklad na zásobník správne
 * @param boolean bool vrátený funkciou po pushovaní
 */
void pushCheck(bool boolean);

/*---------POMOCNÉ FUNKCIE PRE NETERMINÁLY----------------------------*/

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Program
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void programFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Prolog
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void prologFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Main_class
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void mainClassFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Main_class_block
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void mainClassBlockFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Block
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void blockFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Block_content
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void blockContentFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom smth_tail
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void smthTailFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom smth_list
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void smthListFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Func_dec
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void funcDecFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Func_dec_tail
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void funcDecTailFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Param_func
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void paramFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Param_list
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void paramListFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Var
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void varFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom var_ext
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void varExtFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Var_arg
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void varArgFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Var_arg_list
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void varArgListFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Call_assign
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void callAssignFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Call_assign_list
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void callAssignListFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Global_assign
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void globalAssignFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom If
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void ifFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom While
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void whileFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom For
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void forFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Break
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void breakFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Continue
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void continueFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Return
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 */
void returnFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Return_expr
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void returnExprFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom Expr
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param gLookahead ukazovateľ na aktualny token, cez ktory sa zároveň vrati token z precedenčnej analýzy
 * @param AstStack stack uzlov
 * @param flag typ int ->  0  na konci vyrazu moze byt TOKEN_NEW_LINE || TOKEN_RPAREN || TOKEN_BLOCK_END
 *  -1  na konci vyrazu musi byt TOKEN_RPAREN
 *  1 na konci vyrazu moze byt TOKEN_COMMA || TOKEN_RPAREN
 *  -2 na konci musi byt TOKEN_BLOCK_END
 */
void exprFunc(DynamicSymbolStackPtr stack, Token *gLookahead, DynamicAstNodeStackPtr AstStack, int flag);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom New_line
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void newLineFunc(DynamicSymbolStackPtr stack, Token token);


/**
 * @brief Funkcia na zaobchádzanie s neterminálom OPT_NEW_LINES
 * kontroluje výskyt a zabezpečuje načítanie za sebou idúcich nových riadkov
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void optNewlinesFunc(DynamicSymbolStackPtr stack, Token token);


/**
 * @brief Funkcia na zaobchádzanie s neterminálom  BLOCK_CONTENT_FUNC
 * zabezpečuje medziskok v blocku
 * obstaráva token nového riadku a token koncového bloku inak volá funkciu zaobchádzania
 * s príkazmi v bloku
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void   blockContentFuncCheck(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom  CALL_IFJ
 * kontrolu, či volanie vstavanej funkcie je správne syntakticky zapísané následne
 * volá funkciu pre kontrolu existencie vstavanej funkcie
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void   callIfjfunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom  CALL_IFJ_CHECK
 * Prebieha kontrola existencie danej vstavanej funkcie
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void   callIfjCheckfunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom  ELSE_ITEM
 * funkcia obstaráva token if a token začiatku nového bloku, podľa toho, aký
 * aktuálny token sa načíta z lexikálneho analyzátora, vyhodnotí či ide o else if vetvu 
 * alebo o else vetu
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void   elseItemFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom  ELSE_CHAIN
 * kontroluje, či po if vetve nasleduje aj else vetva, ak áno, tak volá 
 * funkciu neterminál ELSE_ITEM a následne aj samú seba
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void   elseChainFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s neterminálom  ASSIGN_IDJ_ID_CHECK
 * funkcia rozhodujúca,či sa jedná o priradenie volaním vstavanej funkcie alebo sa jedná 
 * o priradenie výrazom následne, tak volá precedenčnú analýzu
 * @param stack Inicializovaný stack na ktorom sú štruktúry Symbol neterminálov a terminálov
 * @param token Aktuálne používaný token z lexer.c
 */
void   AssignIfjIdCheckFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief funkcia používaná na uvoľnenie jedného tokenu
 * jedná sa hlavne o uvoľňovanie tokenov s potrebou zachovania nejakých hodnôt
 * @param token Aktuálne používaný token z lexer.c, ktorý chceme uvoľniť 
 */
void   tokenFree(Token *token);

/**
 * @brief funkcia zaobstarajúca uvoľnenie pamäte, ktorá je využívaná
 * alebo alokovaná v syntaktickej analýze a pri tvorbe abstraktného syntaktického stromu
 */
void   parserFree(void);
#endif

/*** konec parser.h ***/
