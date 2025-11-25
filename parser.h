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

#include "dynamic_stack.h"

/**
 * @brief Definicia neterminálov
 */
typedef enum
{
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

AstNode* parser();

/**
 * @brief pomocná funkcia na vytvorenie struktury terminalu
 * @param tokenType Terminal z enum Term
 */
Symbol makeTerm(TokenType tokenType);

/**
 * @brief pomocná funkcia pre vytvorenie struktúry termu
 * pri ktorom potrebujeme uchovať value
 * @param type typu tokenu z ktorého hodnotu chceme uchovať
 */
Symbol makeTermWithValue(TokenType type);

/**
 * @brief pomocná funkcia na vytvorenie struktury neterminalu
 * @param nonTerm Neterminal z enum Nonterm
 */
Symbol makeNonterm(Nonterm nonTerm);

/**
 * @brief pomocná funkcia na vytvorenie strukturý Symbol ktorá
 * bude obsahovať aritu detí a is_reduced = true
 * @param kind nazov neterminálu ktorý strom obsahovať
 * @param arity počet detí prisluchaný k danému uzlu
 */
Symbol makeReduce(AstNodeType kind, size_t arity);

/**
 * @brief pomocná funkcia na overenie správnosti konkrétnej hodnoty tokenu
 * @param token token z ktorého potrebujeme prečitať konkrétnu hodnotus
 */
Symbol makeTermValue(Token token);


/**
 * @brief kontrolná funkcia zda prebehlo pushnovanie na zásobík správne
 * @param boolean bool returnutý funkciou po pushovaní
 */
void pushCheck(bool boolean);


/*-----------------------------------POMOCNÉ FUNKCIE PRE NETERMÍNALY-------------------------------------------------------*/

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Program
 * @param stack Inicializovaný stack na ktorom sú strukturý Symbol neterminálov a terminálov
 */
void programFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Prolog
 * @param stack Inicializovaný stack na ktorom sú strukturý Symbol neterminálov a terminálov
 */
void prologFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Main_class
 * @param stack Inicializovaný stack na ktorom sú strukturý Symbol neterminálov a terminálov
 */
void mainClassFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Main_class_block
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void mainClassBlockFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Block
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void blockFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Block_content
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void blockContentFunc(DynamicSymbolStackPtr stack, Token token);


/**
 * @brief Funkcia na zaobchádzanie s netermínalom smth_tail
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void smthTailFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom smth_list
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void smthListFunc(DynamicSymbolStackPtr stack, Token token);


/**
 * @brief Funkcia na zaobchádzanie s netermínalom Func_dec
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void funcDecFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Func_dec_tail
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void funcDecTailFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Param_func
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void paramFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Param_list
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void paramListFunc(DynamicSymbolStackPtr stack, Token token);



/**
 * @brief Funkcia na zaobchádzanie s netermínalom Var
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void varFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom var_ext
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void varExtFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Var_arg
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void varArgFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Var_arg_list
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void varArgListFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Call_assign
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void callAssignFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Call_assign_list
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void callAssignListFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Global_assign
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void globalAssignFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom If
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void ifFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom While
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void whileFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom For
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void forFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Break
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void breakFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Continue
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 */
void continueFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Return
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void returnFunc(DynamicSymbolStackPtr stack);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Return_expr
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void returnExprFunc(DynamicSymbolStackPtr stack, Token token);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom Expr
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param gLookahead ukazatel na aktualny token, cez ktory sa zaroven vrati token z precedencnej analyzy
 * @param AstStack stack uzlov
 * @param flag typ int ->  0  na konci vyrazu moze byt TOKEN_NEW_LINE || TOKEN_RPAREN || TOKEN_BLOCK_END
 *  -1  na konci vyrazu musi byt TOKEN_RPAREN
 *  1 na konci vyrazu moze byt TOKEN_COMMA || TOKEN_RPAREN
 *  -2 na konci musi byt TOKEN_BLOCK_END
 */
void exprFunc(DynamicSymbolStackPtr stack,Token *gLookahead, DynamicAstNodeStackPtr AstStack,int flag);

/**
 * @brief Funkcia na zaobchádzanie s netermínalom New_line
 * @param stack Inicializovaný stack na ktorom sú štrukturý Symbol neterminálov a terminálov
 * @param token Aktuálne použivaný token z lexer.c
 */
void  newLineFunc(DynamicSymbolStackPtr stack, Token token);

void optNewlinesFunc(DynamicSymbolStackPtr stack, Token token);

Symbol makeTermValueAndCaptureIdentifier(const char *exact);
void blockContentFuncCheck(DynamicSymbolStackPtr stack, Token token);
void tokenFree(Token *token);
void callIfjfunc(DynamicSymbolStackPtr stack, Token token);
void elseItemFunc(DynamicSymbolStackPtr stack, Token token);
void elseChainFunc(DynamicSymbolStackPtr stack, Token token);
void callIfjCheckfunc(DynamicSymbolStackPtr stack, Token token);
void AssignIfjIdCheckFunc(DynamicSymbolStackPtr stack, Token token);
void parserFree(void);
#endif // PARSER_H
