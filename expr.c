/**
 * Fakulta informačních technologií VUT Brno
 *
 * Implementace precedeční analýzy pro překladač jazyka Wren pro projekt do předmětu IFJ 2025
 * @file expr.c
 * @author Petr Kotek (xkotekp00)
 */

#ifndef EXPR_C
#define EXPR_C

#include "expr.h"
#include "dynamic_stack.h"
#include "support_functions.h"


typedef enum {
    R, // >
    L, // <
    E, // =
    N, // prázdné místo

} precedentRules;

int PSA_TABLE[15][15] = {
//   0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
    {R, R, R, R, R, L, R, R, L, R, L, R, N, R, R}, // + -
    {R, R, R, R, R, L, R, R, L, R, L, R, N, N, R}, // * /
    {L, L, R, R, R, L, R, R, L, R, L, R, N, R, R}, // < > >= <=
    {L, L, L, R, R, L, R, R, L, R, L, R, N, N, N}, // is
    {L, L, L, L, R, L, R, R, L, R, L, R, N, R, N}, // == !=
    {R, R, R, R, R, R, R, R, L, R, L, R, N, N, N}, // negace !
    {L, L, L, L, L, L, R, R, L, R, L, R, N, N, N}, // &&
    {L, L, L, L, L, L, L, R, L, R, L, R, N, N, N}, // ||
    {R, R, R, R, R, N, R, R, N, R, N, R, R, R, R}, // id
    {L, L, L, L, L, L, L, L, L, N, L, N, L, L, L}, // dolar
    {L, L, L, L, L, L, L, L, L, N, L, E, N, N, N}, // (
    {R, R, R, R, R, N, R, R, N, R, N, R, R, R, N}, // )
    {N, N, N, N, N, L, N, N, L, N, L, R, R, N, N}, // ../...
    {L, L, L, N, N, N, N, N, L, N, L, N, N, N, E}, // ?
    {L, L, L, N, N, N, N, N, L, N, L, N, N, N, L}, // :
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
    INDEX_DOTS,         // 12   ../...
    INDEX_QUESTION_MARK,
    INDEX_COLON,
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
        case TOKEN_NOT_EQUAL:     return INDEX_EQ;

        // !
        case TOKEN_NOT:     return INDEX_NEG; // Pepa prida token negaci
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
        case TOKEN_STRING:
                            return INDEX_ID;
        case TOKEN_TRUE:    return INDEX_ID;
        case TOKEN_FALSE:    return INDEX_ID;
        case TOKEN_KEYWORD_STRING: return INDEX_ID;
        case TOKEN_KEYWORD_NUM:    return INDEX_ID;
        case TOKEN_KEYWORD_NULL_SMALL: return INDEX_ID;
        case TOKEN_KEYWORD_NULL: return INDEX_ID;


        case TOKEN_EXPRESSION:  // nemuzu pocitat ze tam je identifikator kdyz tam je jenom expression abych a pridal <
                            return INDEX_DOLLAR;


        // (
        case TOKEN_LPAREN:  return INDEX_LP;
        // )
        case TOKEN_RPAREN:  return INDEX_RP;

        case TOKEN_DOUBLE_DOT: return INDEX_DOTS;
        case TOKEN_TRIPLE_DOT: return INDEX_DOTS;

        case TOKEN_QUESTION_MARK:   return INDEX_QUESTION_MARK; // ?
        case TOKEN_COLON:           return INDEX_COLON; // :


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
    case TOKEN_PLUS:            return AST_PLUS;    // +
    case TOKEN_MINUS:           return AST_MINUS;   // -
    case TOKEN_STAR:            return AST_MUL;     // *
    case TOKEN_SLASH:           return AST_DIV;     // /
    case TOKEN_IDENTIFIER:      return AST_IDENTIFIER; // identifikator
    case TOKEN_INTEGER:         return AST_INT_LIT;   // integer literal
    case TOKEN_FLOAT:           return AST_FLOAT_LIT; // float literal
    case TOKEN_GLOBAL_ID:       return AST_GLOBAL_ID; // global identifier
    case TOKEN_EQ:              return AST_EQ;      // ==
    case TOKEN_NOT_EQUAL:             return AST_NEQ;     // !=
    case TOKEN_LT:              return AST_LT;      // <
    case TOKEN_GT:              return AST_GT;      // >
    case TOKEN_AND:             return AST_AND;     // &&
    case TOKEN_OR:              return AST_OR;      // ||
    case TOKEN_KEYWORD_IS:      return AST_IS;      // is
    case TOKEN_GTE:             return AST_GTE;     // >=
    case TOKEN_LTE:             return AST_LTE;     // <=
    case TOKEN_LPAREN:          return AST_LPAREN;     // (
    case TOKEN_RPAREN:          return AST_RPAREN;     // )
    case TOKEN_NOT:          return AST_NOT;     // !
    case TOKEN_STRING:          return AST_STRING_LIT; // string
    case TOKEN_DOUBLE_DOT:      return AST_DOUBLE_DOT;
    case TOKEN_TRIPLE_DOT:      return AST_TRIPLE_DOT;
    case TOKEN_TRUE:            return AST_BOOL_LIT;
    case TOKEN_FALSE:           return AST_BOOL_LIT;
    case TOKEN_KEYWORD_NUM:     return AST_KEYWORD_NUM;
    case TOKEN_KEYWORD_STRING:  return AST_KEYWORD_STRING;
    case TOKEN_KEYWORD_NULL_SMALL: return AST_SMALL_NULL;
    case TOKEN_KEYWORD_NULL: return AST_NULL;

    default:
        printError(SYNTAX_ERROR, "(PSA) Nelze prevést TokenType na AstNodeType, neznámý token\n");
        memorySafeExit(SYNTAX_ERROR);
        return AST_RETURN;
        break;
    }
}

/**
 * @brief Funkce uvolňuje nevyužívaný token
 * @param token ukazatel na strukturu tokenu, který má být uvolněn
 * @return Funkce nemá žádnou návratovou hodnotu
 */
void tokenPrecedenceFree(Token *token)
{
    if (!token)
    {
        return;
    }

    switch (token->type)
    {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_STRING:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            if (token->value.identifier)
            {
                free(token->value.identifier);
                token->value.identifier = NULL;
            }
            break;

        default:
            break;
    }
    *token = (Token){0};
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
    DynamicPrecedenceStackPtr stackTmp;
    stackTmp = stackPrecedenceConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    while (stackPrecedenceRead(stack).type != TOKEN_STOP)
    {
        Token stackTop = stackPrecedenceRead(stack);
        stackPrecedencePop(stack); // vyberu z puvodniho ale dam si na jiny zasobnik, potom tam vse vratim
        stackPrecedencePush(stackTmp, stackTop);
        count++;
    }
    // hodim na zasobnik vse zpet
    while (!stackPrecedenceIsEmpty(stackTmp))
    {
        Token stackTop = stackPrecedenceRead(stackTmp);
        stackPrecedencePop(stackTmp);
        stackPrecedencePush(stack, stackTop);
    }
    stackPrecedenceDestruct(stackTmp); // uvolnim nakonec nepouzivany zasobnik


    printf("\t\t\tparam count %d\n", count);
    switch (count)
    {

        case 1: // E -> i
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack); // vyhodim vyraz
            stackPrecedencePop(stack); // vyhodim <
            AstNode* newNode = NULL;
            newNode = new_leaf(getAstNodeTypeFromTokenType(p1.type), p1);

            tokenPrecedenceFree(&p1); // uvolneni nepotrebneho tokenu

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
            if (p1.type == TOKEN_EXPRESSION && p2.type == TOKEN_NOT)
            {
                AstNode **kids = NULL;
                kids = malloc(sizeof(AstNode*) * 1);
                // vyberu si ze zasobniku 2 identifikatory
                AstNode *firstChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                kids[0] = firstChild; // Levý uzel (i když by u unárních bylo přesnější dát pravý)
                AstNode *newRoot = ast_new(getAstNodeTypeFromTokenType(p2.type), 1, kids); // vytvoření stromu s jedním dítětem, který bude ten výraz
                stackAstNodePush(stackA, newRoot);

                p2.type = TOKEN_EXPRESSION; // vratim na klasicky zasobnik jenom expression
                stackPrecedencePush(stack, p2);
            }
            // E -> -E
            if (p1.type == TOKEN_EXPRESSION && p2.type == TOKEN_MINUS)
            {
                AstNode **kids = NULL;
                kids = malloc(sizeof(AstNode*) * 1);
                // vyberu si ze zasobniku 2 identifikatory
                AstNode *firstChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                kids[0] = firstChild; // Levý uzel (i když by u unárních bylo přesnější dát pravý)
                AstNode *newRoot = ast_new(AST_MINUS, 1, kids); // vytvoření stromu s jedním dítětem, který bude ten výraz
                stackAstNodePush(stackA, newRoot);

                p2.type = TOKEN_EXPRESSION; // vratim na klasicky zasobnik jenom expression
                stackPrecedencePush(stack, p2);
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
                kids[1] = firstChild; // Pravý uzel
                kids[0] = secondChild; // Levý uzel
                AstNode *newRoot = ast_new(getAstNodeTypeFromTokenType(p2.type), 2, kids);
                free(kids); // uvolneni deti, nebudou uz potreba
                stackAstNodePush(stackA, newRoot);

                p2.type = TOKEN_EXPRESSION; // vratim na klasicky zasobnik jenom expression
                stackPrecedencePush(stack, p2);

            }else if (p1.type == TOKEN_RPAREN && p3.type == TOKEN_LPAREN &&  //  E -> (E)
                p2.type == TOKEN_EXPRESSION)
            {
                stackPrecedencePush(stack, p2);
            }
        break;

        // E -> E ? E : E
        case 5:
            Token p4;
            Token p5;
            p1 = stackPrecedenceRead(stack); // E
            stackPrecedencePop(stack); // levy vyraz
            p2 = stackPrecedenceRead(stack); // :
            stackPrecedencePop(stack); // nejaky operator
            p3 = stackPrecedenceRead(stack); // E
            stackPrecedencePop(stack); // pravy vyraz
            p4 = stackPrecedenceRead(stack); // ?
            stackPrecedencePop(stack); // pravy vyraz
            p5 = stackPrecedenceRead(stack); // E
            stackPrecedencePop(stack);
            stackPrecedencePop(stack); // vyhodim <

            // projde pravidlo E ? E : E
            if (p1.type == TOKEN_EXPRESSION && p2.type == TOKEN_COLON && p3.type == TOKEN_EXPRESSION
                && p4.type == TOKEN_QUESTION_MARK && p5.type == TOKEN_EXPRESSION)
            {

                AstNode **kids = NULL;
                kids = malloc(sizeof(AstNode*) * 3);
                // vyberu si ze zasobniku 2 identifikatory
                AstNode *elseNode = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                AstNode *ifNode = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                AstNode *contidionNode = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                kids[2] = elseNode; // jinak
                kids[1] = ifNode; // pokud je to pravda
                kids[0] = contidionNode; // podmínka

                AstNode *newRoot = ast_new(AST_TERNARY, 3, kids);
                free(kids); // uvolneni deti, nebudou uz potreba
                stackAstNodePush(stackA, newRoot);

                p2.type = TOKEN_EXPRESSION; // vratim na klasicky zasobnik jenom expression
                stackPrecedencePush(stack, p2);
            }

        break;

        default:
            printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
            memorySafeExit(SYNTAX_ERROR);
            break;

    }
}

/**
 * @brief Funkce zkotroluje první přijatý token, pokud aktuální token nesmí být prvním, funkce ukončí program
 * @param actualToken Token, který se zkotroluje
 * @return Funkce vrací hodnotu true v případě povoleného token na vstupu, jinak ukončuje program
 */
bool checkFirstTokenType(Token actualToken)
{
    switch (actualToken.type)
    {
    case TOKEN_IDENTIFIER:
    case TOKEN_GLOBAL_ID:
    case TOKEN_INTEGER:
    case TOKEN_STRING:
    case TOKEN_FLOAT:
    case TOKEN_NOT:
    case TOKEN_LPAREN:
    case TOKEN_TRUE:
    case TOKEN_FALSE:
    case TOKEN_MINUS:
    case TOKEN_KEYWORD_NULL_SMALL:
        return true;

    default:
        printError(SYNTAX_ERROR, "(PSA) prvni odeslany token neni identifikator, dostal jsem %s\n", token_type_to_string(actualToken.type));
        memorySafeExit(SYNTAX_ERROR);
        break;
    }
    return false;
}

/**
 * @brief Funkce zkotroluje, že na zásobníku je jenom výraz a dolar
 * @param stack zásobník precedenční analýzy
 * @return Návratovou hodnotou je true v přípradě, že na zásobníku je pouze výraz a dolar, jinak false
 */
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

// pomocná funkce pro výpis zásobníku, bude odstraněno
void printStack(DynamicPrecedenceStackPtr stack)
{
    DynamicPrecedenceStackPtr stackTmp;
    stackTmp = stackPrecedenceConstruct(DEFAULT_INIT_ELEMENT_COUNT); // zkopiruji si stack a spocitam si pocet prvku, ktere na zasobniku jsou
    while (!stackPrecedenceIsEmpty(stack))
    {
        Token tokenRead = stackPrecedenceRead(stack);
        stackPrecedencePop(stack);
        printf("%s ", token_type_to_string(tokenRead.type));
        stackPrecedencePush(stackTmp, tokenRead);
    }
    while (!stackPrecedenceIsEmpty(stackTmp))
    {
         Token topToken = stackPrecedenceRead(stackTmp);
         stackPrecedencePop(stackTmp);
         stackPrecedencePush(stack, topToken);
    }
    stackPrecedenceDestruct(stackTmp); // uvolnim docasny zasobnik

    printf("\n");
}


void functionCalled(DynamicPrecedenceStackPtr stackP, DynamicAstNodeStackPtr stackA, Token *actualToken, int *endFlag)
{
    findPrecedenceRule(stackP, stackA); // zpracuji identifikator, co je na vrcholu zasobniku

    *actualToken = lexer_token_next();
    if (actualToken->type == TOKEN_RPAREN) // Funkce nemá žádný parametr
    {
        AstNode **kids = NULL;
        kids = malloc(sizeof(AstNode*) * 2);
        AstNode *identif = stackAstNodeRead(stackA);
        stackAstNodePop(stackA);
        AstNode *param = NULL;

        kids[0] = identif;
        kids[1] = param;

        AstNode *functionCallRoot = ast_new(AST_FUNC_CALL, 2, kids);
        free(kids);
        stackAstNodePush(stackA, functionCallRoot);
        
        
        if (actualToken->type != TOKEN_NEW_LINE)
        {
            *actualToken = lexer_token_next();
        }
        if (actualToken->type == TOKEN_NEW_LINE)
        {
            *endFlag = TOKEN_PSA_END;
        }

    }else{
        int paramCount = 0;
        while (actualToken->type != TOKEN_RPAREN)
        {
            AstNode *param = parseExpression(actualToken, 0);
            printf("\t\t\t\treturned\n");
            paramCount++;
            stackAstNodePush(stackA, param);
            if (actualToken->type == TOKEN_COMMA)
            {
                *actualToken = lexer_token_next();
                if (actualToken->type == TOKEN_RPAREN) // za čárkou je hned zavorka -> syntaktická chyba
                {
                    printError(SYNTAX_ERROR, "(PSA): Za oddělovačem parametru funckce následuje pravá závorka [(,)]\n");
                    memorySafeExit(SYNTAX_ERROR);
                }
                
            }
        }

        AstNode **paramKids = NULL;
        paramKids = malloc(sizeof(AstNode*) * paramCount);
        for (int i = 0; i < paramCount; ++i) // Cyklem ze zásobníku seberu napočítaná počet parametrů
        {
            if (stackAstNodeIsEmpty(stackA)) // kontrola, zda zásobník, ze kterého beru parametry funce není prázdný
            {
                //fprintf(stderr, "[REDUCE-FAIL] %s chce %zu. Na AST zásobníku došli deti pri i=%zu.\n", ast_node_type_to_string(Stack_top.reduce_kind), n, i);
                printError(SYNTAX_ERROR, "(PSA): málo detí na AST zásobníku pri redukcii\n");
                memorySafeExit(SYNTAX_ERROR);
            }
            AstNode *kid = stackAstNodeRead(stackA);
            stackAstNodePop(stackA);
            paramKids[paramCount - 1 - i] = kid; // tady to ma Kuba implementovane
        }
        AstNode* paramNode = ast_new(AST_PARAM, paramCount, paramKids);
        free(paramKids);
        stackAstNodePush(stackA, paramNode); // na zasobniku by mela byt ted jen identif a paramNode

        AstNode **functionKids = NULL;
        functionKids = malloc(sizeof(AstNode*) * 2);
        AstNode* params = stackAstNodeRead(stackA);
        stackAstNodePop(stackA);
        AstNode* identif = stackAstNodeRead(stackA);
        stackAstNodePop(stackA);

        functionKids[0] = identif;
        functionKids[1] = params;

        AstNode* functionRoot = ast_new(AST_FUNC_CALL, 2, functionKids);
        free(functionKids);
        stackAstNodePush(stackA, functionRoot);

        
        if (actualToken->type != TOKEN_NEW_LINE)
        {
            *actualToken = lexer_token_next();
        }
        if (actualToken->type == TOKEN_NEW_LINE)
        {
            *endFlag = TOKEN_PSA_END;
        }
        printf("ZASOBNIK PO ZPRACOVANI FUNKCE: ");
        printStack(stackP);

        // Token expression;
        // expression.type = TOKEN_EXPRESSION;
        // stackPrecedencePush(stackP, expression);
    }
}

/**
 * @brief Funkce provede precedenční analýzu nad výrazem, v případě, že je výraz správný, vrací ukazatel na kořen stromu, který vytvoří
 * @param token aktuální token, který přišel od překladače
 * @param flag Celočíselná hodnota udávající, kde byla funkce volána (výraz ve v podmínce, ...)
 */
AstNode* parseExpression(Token *token, int flag)
{
    printf("%d FLAG\n", flag);
    DynamicPrecedenceStackPtr stackP = stackPrecedenceConstruct(DEFAULT_INIT_ELEMENT_COUNT); // zásobník pro precedeční analýzu
    DynamicAstNodeStackPtr stackA = stackAstNodeConstruct(DEFAULT_INIT_ELEMENT_COUNT); // Zásobník uzlů, na které budou odkládány zpracované výrazy

    Token actualToken = *token; // aktuální token na vstupu, se kterým pracuji
    int endFlag = 0;
    Token dollar; // $
    dollar.type = TOKEN_DOLLAR;
    Token stop; // >
    stop.type = TOKEN_STOP;

    int poradiCount = 1; // pomocná proměnná pro kontrolu pořadí operací. Potom SMAZAT

    stackPrecedencePush(stackP, dollar); // vložení $ na zasobnik
    Token StackTop;

    bool firstOK = false; // Pomocná proměnná pro kontrolu povoleného prvního tokenu
    bool checkEmptyIf = false; // Pomocn8 proměnná pro kontrolu prázdnosti podmínky
    bool expectingNewLineAfterOperator = true; // Kontrolní proměnná pro implementaci možnosti nového řádku za operátorem
    bool expectingRPAREN = false; // Pomocná proměnná pro implementaci zajištění kontroly ukočení výrazu pravou závorkou
    bool functionCall = false; // Pomocná proměnná pro zpracování funkce ve výrazu
    bool checkIs = false;
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

        if (actualToken.type == TOKEN_LPAREN)
        {
            expectingRPAREN = true;
        }
        
        // ošetření, že za klíčovým slovem is není blbost
        if (checkIs)
        {
            printf("actualToken %s\n", token_type_to_string(actualToken.type));
            if (operatorToPush(StackTop) == INDEX_IS)
            {
                if (actualToken.type != TOKEN_KEYWORD_STRING  && actualToken.type != TOKEN_KEYWORD_NUM && actualToken.type != TOKEN_KEYWORD_NULL) // Doplnit popripade
                {
                    printError(SYNTAX_ERROR, "(PSA) Za klicovym slovem Is nasleduje nezname klicove slovo: %s\n ", token_type_to_string(actualToken.type));
                    memorySafeExit(SYNTAX_ERROR);
                    break;
                }
            }
            checkIs = false; // Kontrola proběhla, resetuji hodnotu
        }
        
        if (actualToken.type == TOKEN_KEYWORD_IS) // nastavení hodnoty pro kontrolu aktuálního tokenu v další iteraci programu
        {
            checkIs = true;
        }
        
        if (actualToken.type == TOKEN_EOF)
        {
            printError(SYNTAX_ERROR, "(PSA) Narazilo na konec souboru, nemelo se tak stat\n");
            stackAstNodeDestruct(stackA); // uvolneni zásobníku uzlů
            stackPrecedenceDestruct(stackP); // uvolnění zásobníku výrazů
            memorySafeExit(SYNTAX_ERROR);
            break;
        }
        

        if (!checkEmptyIf) // voláno pouze jednou při prvním volání sestavení výrazu v if bloku
        {
            checkEmptyIf = true;
            // prevence prazdneho if statementu
            if (flag == -1 && actualToken.type == TOKEN_RPAREN)
            {
                printError(SYNTAX_ERROR, "(PSA) Volano nad prazdnym cyklem, podminkou...\n");
                return NULL;
            }
        }
        if (functionCall)
        {
            printf("FUNC true\n");
        }else{
            printf("FUNC false\n");
        }
        
        if (expectingNewLineAfterOperator && endFlag != TOKEN_PSA_END) // očekávám znak nového řádku za operátorem
        {
            if (operatorToPush(StackTop) != INDEX_ID && operatorToPush(StackTop) != INDEX_DOLLAR && operatorToPush(StackTop) != INDEX_RP)
            {
                if (actualToken.type == TOKEN_NEW_LINE)
                {
                    actualToken = lexer_token_next(); // posouvám se na další token
                    if (operatorToPush(actualToken) != INDEX_ID) // dalsi token neni identifikator, cislo, ...
                    {
                        printError(SYNTAX_ERROR, "(PSA) PSA ocekavalo identifikator za operatorem, ale nedostalo jej\n");
                        memorySafeExit(SYNTAX_ERROR);
                    }
                    
                    expectingNewLineAfterOperator = false; // už jej znovu neočekávám
                }
            }
        }
        if (expectingNewLineAfterOperator == false && actualToken.type == TOKEN_NEW_LINE && (operatorToPush(StackTop) == INDEX_ID || operatorToPush(StackTop) == INDEX_DOLLAR))
        {
            endFlag = TOKEN_PSA_END;
        }

        if (operatorToPush(actualToken) != INDEX_ID && (operatorToPush(StackTop) == INDEX_ID || operatorToPush(StackTop) == INDEX_DOLLAR))
        {
            expectingNewLineAfterOperator = true;
        }else{
            expectingNewLineAfterOperator = false;
        }
        

        // Kontrola povoleného typu prvního tokenu
        if (!firstOK)
        {
            firstOK = checkFirstTokenType(actualToken);
        }

        
        if (!functionCall)
        {
            if (actualToken.type == TOKEN_LPAREN && StackTop.type == TOKEN_IDENTIFIER)
            {
                firstOK = true;
                functionCall = true;
                functionCalled(stackP, stackA, &actualToken, &endFlag); // zpracování celé funkce
                functionCall = false;
                continue; // pokud 
            }
        }
        
        // podminky, ktere mohou nastat pri ukoncovani vyrazu, kazda je unikatni
        if ((flag == -1 && actualToken.type == TOKEN_RPAREN && StackTop.type == TOKEN_DOLLAR)
            || (actualToken.type == TOKEN_NEW_LINE && StackTop.type == TOKEN_DOLLAR) // ukonceni pouze na novy radek
            || (actualToken.type == TOKEN_COMMA && StackTop.type == TOKEN_DOLLAR) // оddělení výrazů ve funkce čárkou
            || (actualToken.type == TOKEN_RPAREN && StackTop.type == TOKEN_DOLLAR && flag == 1) // ukončení funkce pravou závorkou
            || (actualToken.type == TOKEN_BLOCK_END && flag == -2 && StackTop.type == TOKEN_DOLLAR) // ukonceni volani vyrazu v jednoradkove funkci
            || (actualToken.type == TOKEN_RPAREN && flag == 0 && StackTop.type == TOKEN_DOLLAR && expectingRPAREN == false)) // volani funkce pouze s jednim parametrem
        {   // abych mohl koncit potrebuji mit na vstupu token new line a u zasobniku ukazovat jen na $
            printf("splnila se podminka\n");
            printStack(stackP);
            endFlag = TOKEN_PSA_END;
        }

        printf("%d Aktualni token %s a stacktop: %s [%d, %d]\n",poradiCount++, token_type_to_string(actualToken.type), token_type_to_string(stackPrecedenceRead(stackP).type), operatorToPush(StackTop), operatorToPush(actualToken));
        printStack(stackP);

        if (endFlag == TOKEN_PSA_END)
        {
            while (!checkDollarExpression(stackP)) // cyklus probíhá dokud není na zásobníku pouze dolar a výraz
            {
                findPrecedenceRule(stackP, stackA); // upravuji výraz dokud nedostanu na zásobníku pouze dolar a výraz
                printf("Final etapa: ");
                printStack(stackP);
            }

            AstNode* root = stackAstNodeRead(stackA);
            printf("string %s\n", ast_node_type_to_string(root->kind));
            printf("Final stack: ");
            printStack(stackP);

            stackAstNodeDestruct(stackA); // uvolneni zásobníku uzlů
            stackPrecedenceDestruct(stackP); // uvolnění zásobníku výrazů
            *token = actualToken;
            return root; // koren, ktery vracim Kubovi na cely vyraz
        }
        // ošetření toho, že první token musí být nějaký identifikátor, nelze začít operátorem


        // radky a sloupce
        switch (PSA_TABLE[operatorToPush(StackTop)][operatorToPush(actualToken)])
        {

            case R: // >

                printf("\tOperace >\n");
                findPrecedenceRule(stackP, stackA); // najde pravidlo z gramatiky a na precedence zasobnik vrati E
                printf("\t\t\t%d > (%d, %d) ",poradiCount++, operatorToPush(StackTop), operatorToPush(actualToken));
                printStack(stackP);

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
                printError(SYNTAX_ERROR, "(PSA) Zadany vyraz neni syntakticky spravny (neocekavany znak nebo samostatne stojici operator)\n");
                memorySafeExit(SYNTAX_ERROR);
                
                stackAstNodeDestruct(stackA); // uvolneni zásobníku uzlů
                stackPrecedenceDestruct(stackP); // uvolnění zásobníku výrazů
                return NULL;
        }
    }
    return NULL; // navraceni ukazatele na koren vyrazu
}

#endif

/* Konec souboru expr.c */
