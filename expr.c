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
//   0  1  2  3  4  5  6  7  8  9  10 11
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
    INDEX_PLUS_MINUS,   // 0    + -
    INDEX_MUL_DIV,      // 1    * /
    INDEX_COMPARE,      // 2    > < >= <=
    INDEX_IS,           // 3    is
    INDEX_EQ,           // 4    == !=
    INDEX_NEG,          // 5    !
    INDEX_AND,          // 6    &&
    INDEX_OR,           // 7    ||
    INDEX_ID,           // 8    identifikator
    INDEX_DOLLAR,       // 9    dollar
    INDEX_LP,           // 10   (
    INDEX_RP,           // 11   )
} operators_t;


// Na zaklade aktualniho tokenu urci, jaky operator se bude vkladat na zasobnik
int operatorToPush(Token token)
{
    switch (token.type)
        {
        // + -
        case TOKEN_PLUS:    return INDEX_PLUS_MINUS;
        case TOKEN_MINUS:   return INDEX_PLUS_MINUS;
        // * /
        case TOKEN_STAR:    return INDEX_MUL_DIV;
        case TOKEN_SLASH:   return INDEX_MUL_DIV;
        // > < >= <=
        case TOKEN_LT:      return INDEX_COMPARE;
        case TOKEN_GT:      return INDEX_COMPARE;
        case TOKEN_GTE:     return INDEX_COMPARE;
        case TOKEN_LTE:     return INDEX_COMPARE;
        // is
        case TOKEN_KEYWORD_IS:
                            return INDEX_IS;
        // == !=
        case TOKEN_EQ:      return INDEX_EQ;
        case TOKEN_NOT:     return INDEX_EQ;

        // !
        // case TOKEN_NEG:     return P_NEG; // Pepa prida token negaci
        // &&
        case TOKEN_AND:     return INDEX_AND;
        // ||
        case TOKEN_OR:      return INDEX_OR;

        // id
        case TOKEN_IDENTIFIER:
                            return INDEX_ID;
        case TOKEN_INTEGER:
                            return INDEX_ID;
        case TOKEN_GLOBAL_ID:
                            return INDEX_ID;
        case TOKEN_FLOAT:
                            return INDEX_ID;
        // (
        case TOKEN_LPAREN:  return INDEX_LP;
        // )
        case TOKEN_RPAREN:  return INDEX_RP;

        // jinak je to $
        default:
            return INDEX_DOLLAR;
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
 * @brief Funkce převede TokenType na AstNodeType
 * 
 * @param type Typ tokenu
 * @return AstNodeType Převedený typ uzlu AST
 */
AstNodeType getAstNodeTypeFromTokenType(TokenType type)
{
    switch (type)
    {
    case TOKEN_PLUS:        return AST_PLUS;    // +
    case TOKEN_MINUS:       return AST_MINUS;   // -
    case TOKEN_STAR:        return AST_MUL;     // *
    case TOKEN_SLASH:       return AST_DIV;     // /
    case TOKEN_IDENTIFIER:  return AST_IDENTIFIER; // identifikator
    case TOKEN_INTEGER:     return AST_INT_LIT;   // integer literal
    case TOKEN_FLOAT:       return AST_FLOAT_LIT; // float literal
    case TOKEN_GLOBAL_ID:   return AST_GLOBAL_ID; // global identifier
    case TOKEN_EQ:          return AST_EQ;      // ==
    case TOKEN_LT:          return AST_LT;      // <
    case TOKEN_GT:          return AST_GT;      // >
    case TOKEN_NOT:         return AST_NOT;     // !
    case TOKEN_AND:         return AST_AND;     // &&
    case TOKEN_OR:          return AST_OR;      // ||
    case TOKEN_KEYWORD_IS:  return AST_IS;      // is
    case TOKEN_GTE:         return AST_GTE;     // >=
    case TOKEN_LTE:         return AST_LTE;     // <=


    default:
        printError(SYNTAX_ERROR, "(PSA) Nelze prevést TokenType na AstNodeType, neznámý token\n");
        memorySafeExit(SYNTAX_ERROR);
        break;
    }
}


/**
 * @brief Funkce nalezne pravidlo z gramatiky precedenční analýzy a sestaví mini podstrom,
 * ke kterému ukazatel na minikořen hodí na stack
 * @param stack Zásobník, který se bude vyprazdňovat po symbol <
 */
void findPrecedenceRule(DynamicPrecedenceStackPtr stack, DynamicAstNodeStackPtr stackA)
{
    Token p1; // parametr 1
    Token p2; // parametr 2
    Token p3; // parametr 3

    int count = 0;

    DynamicPrecedenceStack tmp = *stack; // zkopiruji si stack a spocitam si pocet prvku, ktere na zasobniku jsou
    while (stackPrecedenceRead(&tmp).type != TOKEN_STOP)
    {
        stackPrecedencePop(&tmp);
        count++;
    }

    switch (count)
    {

        case 1: // E -> i
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // vyhodim vyraz
            stackPrecedencePop(stack); // vyhodim <
            AstNode* newNode = NULL;
            newNode = new_leaf(getAstNodeTypeFromTokenType(p1.type), p1);
            stackAstNodePush(stackA, newNode); // hodim na zasobnik uzel
        break;

        case 2: // E -> !E
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // vyhodim vyraz
            p2 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // vyhodim vyraz
            switch (p2.type)
            {
            case TOKEN_NOT:
                {
                    AstNode **kids = NULL;
                    // vyberu si ze zasobniku 1 identifikator
                    AstNode *child = stackAstNodeRead(stackA);
                    stackAstNodePop(stackA);
                    kids[0] = child;
                    AstNode *newRoot = ast_new(AST_NOT, 1, kids);
                    stackAstNodePush(stackA, newRoot);
                }
                break;
            default:
                printError(SYNTAX_ERROR, "(PSA) Nenalezene pravidlo pro precedencni analyzu\n");
                memorySafeExit(SYNTAX_ERROR);
                break;
            }
        break;

        case 3: // E -> E op E
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // levy vyraz
            p2 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // nejaky operator
            p3 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // pravy vyraz
            

            // kontrola, ze p1 a p3 jsou nejake identifikatory (cislo, promenna, globalni promenna, ...)
            // a ze p2 je nejaky operator (+, -, *, /, atd.)
            if (operatorToPush(p1) == INDEX_ID && operatorToPush(p3) == INDEX_ID &&
                operatorToPush(p2) != INDEX_ID && operatorToPush(p2) != INDEX_DOLLAR)
            {
                AstNode **kids = NULL;
                // vyberu si ze zasobniku 2 identifikatory
                AstNode *firstChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                AstNode *secondChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                kids[-1] = firstChild;
                kids[0] = secondChild;
                AstNode *newRoot = ast_new(getAstNodeTypeFromTokenType(p2.type), 2, kids);
                stackAstNodePush(stackA, newRoot);
            }


        default:
            break;

    }
}

void printStack(DynamicPrecedenceStackPtr stack)
{

    while (!stackPrecedenceIsEmpty(stack))
    {
        Token tokenRead = stackPrecedenceRead(stack);
        stackPrecedencePop(stack);
        printf("%d ", tokenRead.type);
    }

    printf("\n");
}
/**
 * co mam teda vracet? Vracim ukazatel na koren stromu, ktery vytvorim z vyrazu
   tvořím si podstromečky
 */
AstNode* parseExpression(Token *token)
{
    DynamicPrecedenceStackPtr stackP = stackPrecedenceConstruct(DEFAULT_INIT_ELEMENT_COUNT); // zásobník pro precedeční analýzu
    DynamicAstNodeStackPtr stackA = stackAstNodeConstruct(DEFAULT_INIT_ELEMENT_COUNT); // Zásobník uzlů, na které budou odkládány zpracované výrazy

    Token actualToken = *token; // aktuální token na vstupu, se kterým pracuji

    Token dollar; // $
    dollar.type = TOKEN_DOLLAR;
    Token stop; // >
    stop.type = TOKEN_STOP;

    push_check(stackPrecedencePush(stackP, dollar)); // vložení $ na zasobnik
    
    Token StackTop;

    bool firstOK = false; // kontrola, ze prvni token, ktery prijimam je nejaky identifikator

    int krokCount = 1; // debug 

    while (!stackPrecedenceIsEmpty(stackP))
    {
        StackTop = stackPrecedenceRead(stackP);
        int topOp = operatorToPush(StackTop);
        int inputOp = operatorToPush(actualToken);
        // ošetření toho, že první token musí být nějaký identifikátor, nelze začít operátorem
        if (!firstOK)
        {
            switch (actualToken.type)
            {
            case TOKEN_IDENTIFIER:
            case TOKEN_GLOBAL_ID:
            case TOKEN_INTEGER:
            case TOKEN_STRING:
            case TOKEN_FLOAT:
                firstOK = true;
                break;

            default:
                printError(SYNTAX_ERROR, "(PSA) prvni odeslany token neni identifikator\n");
                memorySafeExit(SYNTAX_ERROR);
                break;
            }
        }
        
        // ošetřit konec výrazu, pokud dostanu NL
        // if (((operatorToPush(StackTop) != P_ID) && (operatorToPush(StackTop) != P_DOLLAR)) && actualToken.type == TOKEN_NEW_LINE)
        // {
        //     Token tmp = lexer_token_next();
        //     if (tmp.type == TOKEN_NEW_LINE)
        //     {
        //         break;
        //     }   
        // }
        printf(" (%d, %d) \n", operatorToPush(StackTop), operatorToPush(actualToken));

        switch (PSA_TABLE[topOp][inputOp])
        {

        case R: // >
            findPrecedenceRule(stackP, stackA); // najde pravidlo z gramatiky a na precedence zasobnik vrati E
            break;

        case L: // <
            if (stackPrecedenceRead(stackP).type == TOKEN_DOLLAR)
            { // stack push kdyz tam je jenom dollar
                printf("tady jsem\n");
                stackPrecedencePush(stackP, stop); // <
                stackPrecedencePush(stackP, actualToken);
            }else{ // stack push after top terminal
                Token topTmp = stackPrecedenceRead(stackP);
                stackPrecedencePop(stackP);
                stackPrecedencePush(stackP, stop); // <
                stackPrecedencePush(stackP, topTmp); // to co tam bylo
                stackPrecedencePush(stackP, actualToken); // aktualni token
            }
            actualToken = lexer_token_next();

        case E: // =
            stackPrecedencePush(stackP, actualToken);
            actualToken = lexer_token_next();
            break;

        default: // stav ktery nesmi nastat
            printError(SYNTAX_ERROR, "(PSA) Zadany vyraz neni syntakticky spravny\n");
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
    
    return NULL;
}
// ast_new // vytvoří nový uzel a vytvoří pouze název a propojí mi ho zpětně
// ast_new(stack, kids)ů // 
/**
 * Projit kubuv kod a kouknout na fotky
 */

#endif