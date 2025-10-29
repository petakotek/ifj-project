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
    {R, R, R, R, R, N, R, R, N, R, N, R}, // id
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
        case TOKEN_NOT:     return INDEX_NEG;

        // !
        case TOKEN_NEGACE:     return INDEX_NEG; // Pepa prida token negaci
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



        case TOKEN_EXPRESSION:  // nemuzu pocitat ze tam je identifikator kdyz tam je jenom expression abych a pridal <
                            return INDEX_DOLLAR;


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
    case TOKEN_LPAREN:      return AST_LPAREN;     // (
    case TOKEN_RPAREN:      return AST_RPAREN;     // )

    default:
        printError(SYNTAX_ERROR, "(PSA) Nelze prevést TokenType na AstNodeType, neznámý token\n");
        memorySafeExit(SYNTAX_ERROR);
        return AST_RETURN;
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
    if (stack == NULL)
    {
        printError(SYNTAX_ERROR, "(PSA) stack je ve findPrecedenceRule NULL\n");
        memorySafeExit(SYNTAX_ERROR);
    }
    
    DynamicPrecedenceStack tmp = *stack; // zkopiruji si stack a spocitam si pocet prvku, ktere na zasobniku jsou
    while (stackPrecedenceRead(&tmp).type != TOKEN_STOP)
    {
        stackPrecedencePop(&tmp);
        count++;
    }
    printf("\t\t\tparam count %d\n", count);
    switch (count)
    {

        case 1: // E -> i
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // vyhodim vyraz
            stackPrecedencePop(stack); // vyhodim <
            AstNode* newNode = NULL;
            newNode = new_leaf(getAstNodeTypeFromTokenType(p1.type), p1);
            stackAstNodePush(stackA, newNode); // hodim na zasobnik uzel
            p1.type = TOKEN_EXPRESSION;
            stackPrecedencePush(stack, p1);
        break;

        case 2: // E -> !E
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // vyhodim vyraz
            p2 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // vyhodim vyraz
            stackPrecedencePop(stack); // vyhodim <
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
            stackPrecedencePop(stack); // vyhodim <
            

            // kontrola, ze p1 a p3 jsou nejake identifikatory (cislo, promenna, globalni promenna, ...)
            // a ze p2 je nejaky operator (+, -, *, /, atd.)
            if (p1.type == TOKEN_EXPRESSION && p3.type == TOKEN_EXPRESSION &&
                operatorToPush(p2) != INDEX_ID && operatorToPush(p2) != INDEX_DOLLAR)
            {
                AstNode **kids = NULL;
                kids = malloc(sizeof(AstNode*) * 2);
                // vyberu si ze zasobniku 2 identifikatory
                AstNode *firstChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                AstNode *secondChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                kids[0] = firstChild;
                kids[1] = secondChild;
                AstNode *newRoot = ast_new(getAstNodeTypeFromTokenType(p2.type), 2, kids);
                stackAstNodePush(stackA, newRoot);
                
                p2.type = TOKEN_EXPRESSION; // vratim na klasicky zasobnik jenom expression
                stackPrecedencePush(stack, p2);

            }else if (p1.type == TOKEN_RPAREN && p3.type == TOKEN_LPAREN &&         //  E -> (E)
                p2.type == TOKEN_EXPRESSION)
            {
                stackPrecedencePush(stack, p2);
            }
        break;
            
            


        default:
            break;

    }
}

bool checkDollarExpression(DynamicPrecedenceStackPtr stack)
{
    if (stack == NULL || stackPrecedenceIsEmpty(stack)) {
        return false;
    }
    
    DynamicPrecedenceStack tmp = *stack;
    
    Token exp = stackPrecedenceRead(&tmp);
    if (exp.type != TOKEN_EXPRESSION)
    {
        return false;
    }
    stackPrecedencePop(&tmp);

    Token dol = stackPrecedenceRead(&tmp);
    if (dol.type != TOKEN_DOLLAR)
    {
        return false;
    }
     
    return true;
}

void printStack(DynamicPrecedenceStackPtr stack)
{
    DynamicPrecedenceStack tmp = *stack; // zkopiruji si stack a spocitam si pocet prvku, ktere na zasobniku jsou
    while (!stackPrecedenceIsEmpty(&tmp))
    {
        Token tokenRead = stackPrecedenceRead(&tmp);
        stackPrecedencePop(&tmp);
        printf("%s ", token_type_to_string(tokenRead.type));
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

    int poradiCount = 1;
    
    stackPrecedencePush(stackP, dollar); // vložení $ na zasobnik
    Token StackTop;

    bool firstOK = false; // kontrola, ze prvni token, ktery prijimam je nejaky identifikator

    // dokud neni na zasobniku jenom $E && nemam na vstupu token, ktery mi vsechno ukoncuje
    while (actualToken.type != TOKEN_PSA_END) // 
    {
        StackTop = stackPrecedenceRead(stackP);
        if (StackTop.type == TOKEN_EXPRESSION) // pokud je stacktop expression, presunu ukazatel na vrchol a pozici zpet
        {
            Token tmpTop = stackPrecedenceRead(stackP);
            stackPrecedencePop(stackP); // potrebuji si prvne vyhodit prvek a potom jej vlozit zpet
            StackTop = stackPrecedenceRead(stackP);
            stackPrecedencePush(stackP, tmpTop);
        }

        if (actualToken.type == TOKEN_NEW_LINE && StackTop.type == TOKEN_DOLLAR) 
        {   // abych mohl koncit potrebuji mit na vstupu token new line a u zasobniku ukazovat jen na $
            actualToken.type = TOKEN_PSA_END;
        }
        
        printf("%d Aktualni token %s a stacktop: %s [%d, %d]\n",poradiCount++, token_type_to_string(actualToken.type), token_type_to_string(stackPrecedenceRead(stackP).type), operatorToPush(StackTop), operatorToPush(actualToken));

        
        // ošetření toho, že první token musí být nějaký identifikátor, nelze začít operátorem
        // if (!firstOK)
        // {
        //     switch (actualToken.type)
        //     {
        //     case TOKEN_IDENTIFIER:
        //     case TOKEN_GLOBAL_ID:
        //     case TOKEN_INTEGER:
        //     case TOKEN_STRING:
        //     case TOKEN_FLOAT:
        //     case TOKEN_NEGACE:
        //         firstOK = true;
        //         break;
            
        //     default:
        //         printError(SYNTAX_ERROR, "(PSA) prvni odeslany token neni identifikator\n");
        //         memorySafeExit(SYNTAX_ERROR);
        //         break;
        //     }
        // }
        
        // radky a sloupce
        switch (PSA_TABLE[operatorToPush(StackTop)][operatorToPush(actualToken)])
        {

            case R: // >
                printf("\tOperace >\n");
                findPrecedenceRule(stackP, stackA); // najde pravidlo z gramatiky a na precedence zasobnik vrati E

                printf("\t\t\t%d > (%d, %d) ",poradiCount++, operatorToPush(StackTop), operatorToPush(actualToken));
                printStack(stackP);

                if (actualToken.type == TOKEN_NEW_LINE)
                {
                    while (!checkDollarExpression(stackP))
                    {
                        findPrecedenceRule(stackP, stackA);
                        printf("Final etapa pico: ");
                        printStack(stackP);
                    }
                    
                    AstNode* root = stackAstNodeRead(stackA);
                    printf("string %s\n", ast_node_type_to_string(root->kind));
                    stackAstNodeDestruct(stackA);
                    stackPrecedenceDestruct(stackP);
                    *token = actualToken;
                    return root; // koren, ktery vracim Kubovi na cely vyraz
                }

            break;

            case L: // <
                // printf(" < (%d, %d) ", operatorToPush(StackTop), operatorToPush(actualToken));
                printf("\tOperace <\n");
                if (stackPrecedenceRead(stackP).type == TOKEN_DOLLAR || stackPrecedenceRead(stackP).type != TOKEN_EXPRESSION) // pokud je na za zasobniku pouze dollar, tak tam vlozim  < i
                // struktura bude tedy  $ < i
                { 
                    printf("\t\t%d budu hazet na stack klasicky\n",poradiCount++);
                    stackPrecedencePush(stackP, stop); // <
                    stackPrecedencePush(stackP, actualToken);
                    
                }
                else{ // vlozim na zasobnik pred prvni terminal stop znak, abych vedel, kde potom koncit
                    printf("\t\t%d budu hazet na stack slozite\n",poradiCount++);
                    Token topTmp = stackPrecedenceRead(stackP);
                    stackPrecedencePop(stackP);
                    stackPrecedencePush(stackP, stop); // <
                    stackPrecedencePush(stackP, topTmp); // to co tam bylo
                    stackPrecedencePush(stackP, actualToken); // aktualni token
                    
                }

                printf("\t\t\t%d < (%d, %d) ",poradiCount++, operatorToPush(StackTop), operatorToPush(actualToken));
                printStack(stackP);
                
                actualToken = lexer_token_next();
            break;

            case E: // =
                stackPrecedencePush(stackP, actualToken);
                printf("\t\t\t%d = (%d, %d) ",poradiCount++, operatorToPush(StackTop), operatorToPush(actualToken));
                printStack(stackP);
                actualToken = lexer_token_next();
            break;

            default: // stav ktery nesmi nastat
                printError(SYNTAX_ERROR, "(PSA) Zadany vyraz neni syntakticky spravny\n");
                return NULL;
        }
    }
    return NULL; // navraceni ukazatele na koren vyrazu
}

#endif