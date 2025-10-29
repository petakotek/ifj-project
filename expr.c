/**
 * @brief Soubor s funkcí pro zpracování výrazu
 */

#ifndef EXPR_C
#define EXPR_C

#include "expr.h"

/**
 * TODO: vyřešit ukončování výrazu? Například počítáním závorek? Popřípadě pokud závorka
 * není a je tam znak nového řádku tak projít nový řádek a podívat se dál? O 2 tokeny?
 * 
 * TODO: zkontrolovat precedeční tabulku, jestli dává smysl a je správná!!!
 */

typedef enum {
    R, // >
    L, // <
    E, // =
    N, // empty space

} precedentRules;

int PSA_TABLE[12][12] = {
//  +- */ >< >= <=
    {R, L, R, R, R, L, R, R, L, R, L, R}, // + -
    {R, R, R, R, R, L, R, R, L, R, L, R}, // * /
    {L, L, R, R, R, L, R, R, L, R, L, R}, // < > >= <=
    {L, L, L, R, R, L, R, R, L, R, L, R}, // is
    {L, L, L, L, R, L, R, R, L, R, L, R}, // == !=
    {R, R, R, R, R, R, R, R, L, R, L, R}, // negace !
    {L, L, L, L, L, L, R, R, L, R, L, R}, // &&
    {L, L, L, L, L, L, L, R, L, R, L, R}, // ||
    {R, R, R, R, R, N, L, L, N, L, N, L}, // id
    {L, L, L, L, L, L, L, L, L, N, L, N}, // dollar
    {L, L, L, L, L, L, L, L, L, N, L, E}, // (
    {R, R, R, R, R, N, R, R, N, R, N, R}, // )
};

 // vsechny operatory, ktere se mohou vyskytnout ve vyrazu
typedef enum{
    P_PLUS_MINUS, // + -
    P_MUL_DIV, // + /
    P_COMPARE, // > < >= <=
    P_IS, // is
    P_EQ, // == !=
    P_NEG, // !
    P_AND, // &&
    P_OR, // ||
    P_ID, // identifikator
    P_DOLLAR, // dollar
    P_LP, // (
    P_RP, // )
} operators_t;


// Na zaklade aktualniho tokenu urci, jaky operator se bude vkladat na zasobnik
int operatorToPush(Token token)
{
    switch (token.type)
        {
        // + -
        case TOKEN_PLUS:    return P_PLUS_MINUS;
        case TOKEN_MINUS:   return P_PLUS_MINUS;
        // * /
        case TOKEN_STAR:    return P_MUL_DIV;
        case TOKEN_SLASH:   return P_MUL_DIV;
        // > < >= <=
        case TOKEN_LT:      return P_COMPARE;
        case TOKEN_GT:      return P_COMPARE;
        case TOKEN_GTE:     return P_COMPARE;
        case TOKEN_LTE:     return P_COMPARE;
        // is
        case TOKEN_KEYWORD_IS:
                            return P_IS;
        // == !=
        case TOKEN_EQ:      return P_EQ;
        case TOKEN_NOT:     return P_EQ;

        // !
        // case TOKEN_NEG:     return P_NEG; // Pepa prida token negaci
        // &&
        case TOKEN_AND:     return P_AND;
        // ||
        case TOKEN_OR:      return P_OR;

        // id
        case TOKEN_IDENTIFIER:
                            return P_ID;
        case TOKEN_INTEGER:
                            return P_ID;
        case TOKEN_GLOBAL_ID:
                            return P_ID;
        case TOKEN_FLOAT:
                            return P_ID;
        // (
        case TOKEN_LPAREN:  return P_LP;
        // )
        case TOKEN_RPAREN:  return P_RP;

        // jinak je to $
        default:
            return P_DOLLAR;
        }
}


/**
 * GRAMATIKA PSA
 * 
 * 01) E :: E + E
 * 02) E :: E - E
 * 03) E :: E * E
 * 04) E :: E / E
 * 05) E :: (E)
 * 06) E :: !E
 * 07) E :: E == E
 * 08) E :: E != E
 * 09) E :: E is E
 * 10) E :: E && E
 * 11) E :: E || E
 * 12) E :: E > E
 * 13) E :: E < E
 * 14) E :: E <= E
 * 15) E :: E >= E
 * 16) E :: E ? E : E       /// ?:
 * 
 */

/**
 * @brief Funkce nalezne pravidlo z gramatiky precedenční analýzy a sestaví mini podstrom,
 * ke kterému ukazatel na minikořen hodí na stack
 * @param stack Zásobník, který se bude vyprazdňovat po symbol <
 */
void findPrecedenceRule(DynamicPrecedenceStackPtr stack)
{
    while (stackPrecedenceRead(stack).symbol != L)
    {
        
    }
    stackPrecedencePop(stack); // vyhození konečného symbolu <
}

/**
 * co mam teda vracet? Vracim ukazatel na koren stromu, ktery vytvorim z vyrazu
``tvořím si podstromečky
 */
AstNode* parseExpression(Token *token)
{
    DynamicPrecedenceStackPtr stack = stackPrecedenceConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    // je potřeba ukládat na zásobník symboly
    Token actualToken = *token;
    pushCheck(stackPrecedencePush(stack, makeTerm(P_DOLLAR))); // vložení $ na zasobnik

    while (!stackPrecedenceIsEmpty(stack))
    {

        switch (PSA_TABLE[stackPrecedenceRead(stack).symbol][operatorToPush(actualToken)])
        {
        case R: // >
            
            
            findPrecedenceRule(stack);
            
        case L: // <
            
            
        case E: // =
            
            actualToken = lexer_token_next();
        default:
            printError(INTERNAL_ERROR, "(PSA) Zadany vyraz neni syntakticky spravny");
            return NULL;
        }
    }
    /**
     * Tvorim strom, kde vracim ukazatel na koren
     * Kuba mi da do funkce jeste 0 nebo 1, pokud tam bude 1, tak zpracovavam vyraz, ktery konci zavorkou )
     */
    // new_leaf(AST_EXPR, )
    /*
    Potom  co bude výraz zpraovanej tak zavolám funkci: 
    */
    
    return stackPrecedenceRead(stack).ast_ptr;
}
// ast_new // vytvoří nový uzel a vytvoří pouze název a propojí mi ho zpětně
// ast_new(stack, kids)ů // 
/**
 * Projit kubuv kod a kouknout na fotky
 */

#endif