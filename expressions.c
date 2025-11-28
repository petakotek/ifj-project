/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Implementace precedenční analýzy pro překladač jazyka Wren pro projekt do předmětu IFJ 2025
 * @file expressions.c
 * @author Petr Kotek (xkotekp00)
 */

#ifndef EXPR_C
#define EXPR_C

#include "expressions.h"

#include "ast.h"
#include "dynamic_stack.h"
#include "lexer.h"
#include "support_functions.h"

#ifdef ENABLE_DEVELOPMENT_LOGGING
#define DEBUG true
#else
#define DEBUG false
#endif

typedef enum {
    R,  // >
    L,  // <
    E,  // =
    N,  // prázdné místo

} precedentRules;

int PSA_TABLE[15][15] = {
    //   0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
    {R, L, R, R, R, L, R, R, L, R, L, R, N, R, R},  // + -       0
    {R, R, R, R, R, L, R, R, L, R, L, R, N, N, R},  // * /       1
    {L, L, R, R, R, L, R, R, L, R, L, R, N, R, R},  // < > >= <= 2
    {L, L, L, R, R, L, R, R, L, R, L, R, N, N, N},  // is        3
    {L, L, L, L, R, L, R, R, L, R, L, R, N, R, N},  // == !=     4
    {R, R, R, R, R, R, R, R, L, R, L, R, N, N, N},  // negace !  5
    {L, L, L, L, L, L, R, R, L, R, L, R, N, N, N},  // &&        6
    {L, L, L, L, L, L, L, R, L, R, L, R, N, N, N},  // ||        7
    {R, R, R, R, R, N, R, R, N, R, N, R, R, R, R},  // id        8
    {L, L, L, L, L, L, L, L, L, N, L, N, L, L, L},  // dolar     9
    {L, L, L, L, L, L, L, L, L, N, L, E, N, N, N},  // (         10
    {R, R, R, R, R, N, R, R, N, R, N, R, R, R, R},  // )         11
    {N, N, N, N, N, L, N, N, L, N, L, R, R, N, N},  // ../...    12
    {L, L, L, N, N, N, N, N, L, N, L, N, N, N, E},  // ?         13
    {L, L, L, N, N, N, N, N, L, R, L, N, N, N, L},  // :         14
};

// vsechny operatory, ktere se mohou vyskytnout ve vyrazu
typedef enum {
    INDEX_PLUS_MINUS,     // 0    + -
    INDEX_MUL_DIV,        // 1    * /
    INDEX_COMPARE,        // 2    > < >= <=
    INDEX_IS,             // 3    is
    INDEX_EQ,             // 4    == !=
    INDEX_NEG,            // 5    !
    INDEX_AND,            // 6    &&
    INDEX_OR,             // 7    ||
    INDEX_ID,             // 8    identifikator
    INDEX_DOLLAR,         // 9    dollar
    INDEX_LP,             // 10   (
    INDEX_RP,             // 11   )
    INDEX_DOTS,           // 12   ../...
    INDEX_QUESTION_MARK,  // ?
    INDEX_COLON,          // :
} operators_t;

/**
 * @brief Funkce na základě aktuálního tokenu určí, jaký operátor se bude vkládat na zásobník
 * @param token Aktuální načtený token
 * @return Funkce navrací index výčtového typu operators_t
 */
int operatorToPush(Token token) {
    switch (token.type) {
        // + -
        case TOKEN_PLUS:
            return INDEX_PLUS_MINUS;
        case TOKEN_MINUS:
            return INDEX_PLUS_MINUS;
        // * /
        case TOKEN_STAR:
            return INDEX_MUL_DIV;
        case TOKEN_SLASH:
            return INDEX_MUL_DIV;
        // > < >= <=
        case TOKEN_LT:
            return INDEX_COMPARE;
        case TOKEN_GT:
            return INDEX_COMPARE;
        case TOKEN_GTE:
            return INDEX_COMPARE;
        case TOKEN_LTE:
            return INDEX_COMPARE;
        // is
        case TOKEN_KEYWORD_IS:
            return INDEX_IS;
        // == !=
        case TOKEN_EQ:
            return INDEX_EQ;
        case TOKEN_NOT_EQUAL:
            return INDEX_EQ;
        // !
        case TOKEN_NOT:
            return INDEX_NEG;
        // &&
        case TOKEN_AND:
            return INDEX_AND;
        // ||
        case TOKEN_OR:
            return INDEX_OR;
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
        case TOKEN_TRUE:
            return INDEX_ID;
        case TOKEN_FALSE:
            return INDEX_ID;
        case TOKEN_KEYWORD_STRING:
            return INDEX_ID;
        case TOKEN_KEYWORD_NUM:
            return INDEX_ID;
        case TOKEN_KEYWORD_NULL_SMALL:
            return INDEX_ID;
        case TOKEN_KEYWORD_NULL:
            return INDEX_ID;
        case TOKEN_STRING_BLOCK:
            return INDEX_ID;
        case TOKEN_HEXA:
            return INDEX_ID;
        case TOKEN_INTEGER_EXP:
            return INDEX_ID;
        case TOKEN_FLOAT_EXP:
            return INDEX_ID;
        // Pokud je na zásobníku výraz, beru jej jako dolar
        case TOKEN_EXPRESSION:
            return INDEX_DOLLAR;
        // (
        case TOKEN_LPAREN:
            return INDEX_LP;
        // )
        case TOKEN_RPAREN:
            return INDEX_RP;

        case TOKEN_DOUBLE_DOT:
            return INDEX_DOTS;
        case TOKEN_TRIPLE_DOT:
            return INDEX_DOTS;
        // ?
        case TOKEN_QUESTION_MARK:
            return INDEX_QUESTION_MARK;
        // :
        case TOKEN_COLON:
            return INDEX_COLON;

        // v ostatních případech $
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
 * 16) E :: E ? E : E
 *
 */

/**
 * @brief Funkce převede TokenType na AstNodeType
 *
 * @param type Typ tokenu
 * @return AstNodeType Převedený typ uzlu AST
 */
AstNodeType getAstNodeTypeFromTokenType(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:
            return AST_PLUS;                // +
        case TOKEN_MINUS:
            return AST_MINUS;               // -
        case TOKEN_STAR:
            return AST_MUL;                 // *
        case TOKEN_SLASH:
            return AST_DIV;                 // /
        case TOKEN_IDENTIFIER:
            return AST_IDENTIFIER;          // identifikátor
        case TOKEN_INTEGER:
            return AST_INT_LIT;             // integer literál
        case TOKEN_FLOAT:
            return AST_FLOAT_LIT;           // float literál
        case TOKEN_GLOBAL_ID:
            return AST_GLOBAL_ID;           // globální identifikátor
        case TOKEN_EQ:
            return AST_EQ;                  // ==
        case TOKEN_NOT_EQUAL:
            return AST_NEQ;                 // !=
        case TOKEN_LT:
            return AST_LT;                  // <
        case TOKEN_GT:
            return AST_GT;                  // >
        case TOKEN_AND:
            return AST_AND;                 // &&
        case TOKEN_OR:
            return AST_OR;                  // ||
        case TOKEN_KEYWORD_IS:
            return AST_IS;                  // is
        case TOKEN_GTE:
            return AST_GTE;                 // >=
        case TOKEN_LTE:
            return AST_LTE;                 // <=
        case TOKEN_LPAREN:
            return AST_LPAREN;              // (
        case TOKEN_RPAREN:
            return AST_RPAREN;              // )
        case TOKEN_NOT:
            return AST_NOT;                 // !
        case TOKEN_STRING:
            return AST_STRING_LIT;          // string
        case TOKEN_DOUBLE_DOT:              // ..
            return AST_DOUBLE_DOT;
        case TOKEN_TRIPLE_DOT:              // ...
            return AST_TRIPLE_DOT;
        case TOKEN_TRUE:                    // true
            return AST_BOOL_LIT;
        case TOKEN_FALSE:                   // false
            return AST_BOOL_LIT;
        case TOKEN_KEYWORD_NUM:             // Num
            return AST_KEYWORD_NUM;
        case TOKEN_KEYWORD_STRING:          // String
            return AST_KEYWORD_STRING;
        case TOKEN_KEYWORD_NULL_SMALL:      // null
            return AST_SMALL_NULL;  
        case TOKEN_KEYWORD_NULL:            // Null
            return AST_NULL;    
        case TOKEN_STRING_BLOCK:            // """ x """
            return AST_STRING_BLOCK;
        case TOKEN_HEXA:                    // 0x..
            return AST_INT_LIT;
        case TOKEN_INTEGER_EXP:             // e..
            return AST_INT_LIT_EXP;
        case TOKEN_FLOAT_EXP:               // x.x
            return AST_FLOAT_LIT_EXP;

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
void tokenPrecedenceFree(Token *token) {
    if (!token) {
        return;
    }

    switch (token->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_STRING:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_KEYWORD_IS:
        case TOKEN_KEYWORD_NUM:
        case TOKEN_KEYWORD_STRING:
        case TOKEN_HEXA:
        case TOKEN_KEYWORD_NULL_SMALL:
        case TOKEN_FLOAT_EXP:
        case TOKEN_INTEGER_EXP:
        case TOKEN_KEYWORD_NULL:
        case TOKEN_STRING_BLOCK:
            if (token->value.identifier) {
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
 * @brief Funkce uvolní zásobník uzlů AST stromu
 * @param stack Zásobník datového typu DynamicAstNodeStackPtr
 */
void freeAstStack(DynamicAstNodeStackPtr stack) {
    while (!stackAstNodeIsEmpty(stack)) {
        AstNode *tmp = stackAstNodeRead(stack);
        stackAstNodePop(stack);
        astFreeWhole(tmp);
        tmp = NULL;
    }
    stackAstNodeDestruct(stack);
}
/**
 * @brief Funkce uvolní zásobník tokenů
 * @param stack Zásobník datového typu DynamicPrecedenceStackPtr
 */
void freePrecedenceStack(DynamicPrecedenceStackPtr stack) {
    while (!stackPrecedenceIsEmpty(stack)) {
        Token tmp = stackPrecedenceRead(stack);
        stackPrecedencePop(stack);
        tokenPrecedenceFree(&tmp);
    }
    stackPrecedenceDestruct(stack);
}
/**
 * @brief Funkce spočítá počet položek v zásobníku tokenů
 * @param stack Zásobník tokenů
 * @return Fuknce navrací počet položek
 */
int countParameters(DynamicPrecedenceStackPtr stack){
    int count = 0;

    DynamicPrecedenceStackPtr stackTmp;
    stackTmp = stackPrecedenceConstruct(DEFAULT_INIT_ELEMENT_COUNT);

    while (stackPrecedenceRead(stack).type != TOKEN_STOP) {
        Token stackTop = stackPrecedenceRead(stack);
        // vyberu z původního a dám si na jiný zasobnik, potom vše vrátím zpět
        stackPrecedencePop(stack);
        stackPrecedencePush(stackTmp, stackTop);
        count++;
    }

    // hodim na zasobnik vse zpet
    while (!stackPrecedenceIsEmpty(stackTmp)) {

        Token stackTop = stackPrecedenceRead(stackTmp);
        stackPrecedencePop(stackTmp);
        stackPrecedencePush(stack, stackTop);
    }

    freePrecedenceStack(stackTmp);  // uvolnění nepoužívaného zásobníku

    return count;
}

/**
 * @brief Funkce nalezne pravidlo z gramatiky precedenční analýzy a sestaví mini podstrom,
 * ke kterému ukazatel na minikořen hodí na stack
 * @param stack Zásobník, který se bude vyprazdňovat po symbol <
 */
void findPrecedenceRule(DynamicPrecedenceStackPtr stack, DynamicAstNodeStackPtr stackA) {
    Token p1;  // parametr 1
    Token p2;  // parametr 2
    Token p3;  // parametr 3
    Token p4;  // parametr 4 (pouze u ternárního operátoru)
    Token p5;  // parametr 5 (pouze u ternárního operátoru)

    
    if (stack == NULL) {
        printError(SYNTAX_ERROR, "(PSA) stack je ve findPrecedenceRule NULL\n");
        freeAstStack(stackA);
        freePrecedenceStack(stack);
        memorySafeExit(SYNTAX_ERROR);
    }

    // výpočet poštu položek na zásobníku
    int   count = countParameters(stack);
    

    if (DEBUG)
        printLog("\t\t\tparam count %d\n", count);

    switch (count) {
        case 1:  // E -> i
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack);  // vyhodim vyraz
            stackPrecedencePop(stack);  // vyhodím znak značící konec výrazu <
            AstNode *newNode = NULL;
            newNode          = newLeaf(getAstNodeTypeFromTokenType(p1.type), p1);

            tokenPrecedenceFree(&p1);  // uvolneni nepotrebneho tokenu

            stackAstNodePush(stackA, newNode);  // uložení uzlu na zásobník
            p1.type = TOKEN_EXPRESSION; // na zásobník tokenů je vracen token expression
            stackPrecedencePush(stack, p1);
            break;

        case 2:  // E -> !E
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack);  // vyhodim vyraz
            p2 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack);  // vyhodim vyraz
            stackPrecedencePop(stack);  // vyhodím znak značící konec výrazu <
            if (p1.type == TOKEN_EXPRESSION && p2.type == TOKEN_NOT) {
                AstNode **kids = malloc(sizeof(AstNode *) * 1);

                if (kids == NULL)
                {
                    printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
                    freeAstStack(stackA);
                    freePrecedenceStack(stack);
                    memorySafeExit(SYNTAX_ERROR);
                    break;
                }
                
                AstNode *firstChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                // Levý uzel (i když by u unárních bylo přesnější dát pravý)
                kids[0] = firstChild;
                // vytvoření stromu s jedním dítětem, který bude ten výraz
                AstNode *newRoot = astNew(getAstNodeTypeFromTokenType(p2.type), 1, kids);
                stackAstNodePush(stackA, newRoot);
                free(kids); // uvolnění dětí, ukazatelé na ně jsou již zajištěny
                p2.type = TOKEN_EXPRESSION;  // vratim na klasicky zasobnik jenom expression
                stackPrecedencePush(stack, p2);
                break;
            }
            // E -> -E
            if (p1.type == TOKEN_EXPRESSION && p2.type == TOKEN_MINUS) {

                AstNode **kids = malloc(sizeof(AstNode *) * 1);

                if (kids == NULL)
                {
                    printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
                    freeAstStack(stackA);
                    freePrecedenceStack(stack);
                    memorySafeExit(SYNTAX_ERROR);
                    break;
                }

                // vyberu si ze zasobniku 2 identifikatory
                AstNode *firstChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                // Levý uzel (i když by u unárních bylo přesnější dát pravý)
                kids[0] = firstChild;
                // vytvoření stromu s jedním dítětem, který bude výraz
                AstNode *newRoot = astNew(AST_MINUS, 1, kids);
                stackAstNodePush(stackA, newRoot);
                free(kids); // uvolnění dětí, ukazatelé na ně jsou již zajištěny
                p2.type = TOKEN_EXPRESSION;  // na zásobník tokenů je vracen token expression
                stackPrecedencePush(stack, p2);
                break;
            }
            printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
            freeAstStack(stackA);
            freePrecedenceStack(stack);
            memorySafeExit(SYNTAX_ERROR);
            break;

        case 3:  // E -> E op E
            p1 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack);  // levý vyraz
            p2 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack);  // operátor
            p3 = stackPrecedenceRead(stack);
            stackPrecedencePop(stack);  // pravý vyraz
            stackPrecedencePop(stack);  // vyhodím znak značící konec výrazu <

            // kontrola: p1 a p3 jsou identifikátory, p2 je nějaký operator (+, -, *, /, atd.)
            if (p1.type == TOKEN_EXPRESSION && p3.type == TOKEN_EXPRESSION && operatorToPush(p2) != INDEX_ID &&
                operatorToPush(p2) != INDEX_DOLLAR) {
                
                AstNode **kids = malloc(sizeof(AstNode *) * 2);

                if (kids == NULL)
                {
                    printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
                    freeAstStack(stackA);
                    freePrecedenceStack(stack);
                    memorySafeExit(SYNTAX_ERROR);
                    break;
                }

                // vyberu si ze zasobniku 2 identifikatory
                AstNode *firstChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                AstNode *secondChild = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                kids[1]          = firstChild;   // Pravý uzel
                kids[0]          = secondChild;  // Levý uzel
                AstNode *newRoot = astNew(getAstNodeTypeFromTokenType(p2.type), 2, kids);
                free(kids);  // uvolnění dětí, ukazatelé na ně jsou již zajištěny
                stackAstNodePush(stackA, newRoot);

                tokenPrecedenceFree(&p2);
                p2.type = TOKEN_EXPRESSION;  // na zásobník tokenů je vracen token expression
                stackPrecedencePush(stack, p2);

            } else if (p1.type == TOKEN_RPAREN && p3.type == TOKEN_LPAREN &&  //  E -> (E)
                       p2.type == TOKEN_EXPRESSION) {
                stackPrecedencePush(stack, p2);
            }
            break;

        // E -> E ? E : E, ternární operátor
        case 5:

            p1 = stackPrecedenceRead(stack);  // E
            stackPrecedencePop(stack);        // levy výraz
            p2 = stackPrecedenceRead(stack);  // :
            stackPrecedencePop(stack);        // operátor
            p3 = stackPrecedenceRead(stack);  // E
            stackPrecedencePop(stack);        // pravý výraz
            p4 = stackPrecedenceRead(stack);  // ?
            stackPrecedencePop(stack);        // pravý výraz
            p5 = stackPrecedenceRead(stack);  // E
            stackPrecedencePop(stack);
            stackPrecedencePop(stack);  // vyhodím znak značící konec výrazu <

            // pravidlo E ? E : E
            if (p1.type == TOKEN_EXPRESSION && p2.type == TOKEN_COLON && p3.type == TOKEN_EXPRESSION &&
                p4.type == TOKEN_QUESTION_MARK && p5.type == TOKEN_EXPRESSION) {

                AstNode **kids = malloc(sizeof(AstNode *) * 3);

                if (kids == NULL)
                {
                    printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
                    freeAstStack(stackA);
                    freePrecedenceStack(stack);
                    memorySafeExit(SYNTAX_ERROR);
                    break;
                }
                // vyberu si ze zasobniku 2 identifikatory
                AstNode *elseNode = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                AstNode *ifNode = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                AstNode *contidionNode = stackAstNodeRead(stackA);
                stackAstNodePop(stackA);
                kids[2] = elseNode;       // else
                kids[1] = ifNode;         // true
                kids[0] = contidionNode;  // podmínka

                AstNode *newRoot = astNew(AST_TERNARY, 3, kids);
                free(kids);  // uvolnění dětí, ukazatelé na ně jsou již zajištěny
                stackAstNodePush(stackA, newRoot);

                p2.type = TOKEN_EXPRESSION;  // na zásobník tokenů je vracen token expression
                stackPrecedencePush(stack, p2);
            }

            break;

        default:
            printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
            freeAstStack(stackA);
            freePrecedenceStack(stack);
            memorySafeExit(SYNTAX_ERROR);
            break;
    }
}

/**
 * @brief Funkce zkotroluje první přijatý token,
 *        pokud aktuální token nesmí být prvním, funkce ukončí program
 *
 * @param actualToken Token, který se zkotroluje
 * @return Funkce vrací hodnotu true v případě povoleného token na vstupu, jinak ukončuje program
 */
bool checkFirstTokenType(Token actualToken) {
    switch (actualToken.type) {
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
        case TOKEN_STRING_BLOCK:
        case TOKEN_HEXA:
        case TOKEN_INTEGER_EXP:
        case TOKEN_FLOAT_EXP:
            return true;

        default:
            printError(SYNTAX_ERROR, "(PSA) prvni odeslany token neni identifikator, dostal jsem %s\n",
                       tokenTypeToString(actualToken.type));
            memorySafeExit(SYNTAX_ERROR);
            break;
    }
    return false;
}

/**
 * @brief Funkce zkotroluje, že na zásobníku je jenom výraz a dolar
 * @param stack zásobník precedenční analýzy
 * @return Návratovou hodnotou je true v přípradě,
 *         že na zásobníku je pouze výraz a dolar, jinak false
 */
bool checkDollarExpression(DynamicPrecedenceStackPtr stack) {
    if (stack == NULL || stackPrecedenceIsEmpty(stack)) {
        return false;
    }

    DynamicPrecedenceStack tmp = *stack;

    Token exp = stackPrecedenceRead(&tmp);
    if (exp.type != TOKEN_EXPRESSION) {
        return false;
    }
    stackPrecedencePop(&tmp);

    Token dol = stackPrecedenceRead(&tmp);
    if (dol.type != TOKEN_DOLLAR) {
        return false;
    }

    return true;
}

/**
 * @brief Pomocná funkce pro vypsání aktuální zásobíku, slouží pro ladící účely
 * @param stack Zásobník tokenů
 */
void printStack(DynamicPrecedenceStackPtr stack) {
    DynamicPrecedenceStackPtr stackTmp;
    // zkopiruji si stack a spocitam si pocet prvku, ktere na zasobniku jsou
    stackTmp = stackPrecedenceConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    while (!stackPrecedenceIsEmpty(stack)) {
        Token tokenRead = stackPrecedenceRead(stack);
        stackPrecedencePop(stack);
        if (DEBUG)
            printLog("%s ", tokenTypeToString(tokenRead.type));

        stackPrecedencePush(stackTmp, tokenRead);
    }
    while (!stackPrecedenceIsEmpty(stackTmp)) {
        Token topToken = stackPrecedenceRead(stackTmp);
        stackPrecedencePop(stackTmp);
        stackPrecedencePush(stack, topToken);
    }
    freePrecedenceStack(stackTmp);  // uvolnim docasny zasobnik

    if (DEBUG)
        printLog("\n");
}
/**
 * @brief Funkce zpracovávající volání funkce
 * @param stackP Zásobník tokenů
 * @param stackA Zásobník AST uzlů
 * @param actualToken Ukazatel na aktuální načtený token
 * @param endFlag Ukazatel na pomocnou proměnnou ukočující PSA
 */
void functionCalled(DynamicPrecedenceStackPtr stackP, DynamicAstNodeStackPtr stackA, Token *actualToken, int *endFlag) {
    findPrecedenceRule(stackP, stackA);  // zpracuji identifikator, co je na vrcholu zasobniku

    *actualToken = lexerTokenNext();
    if (actualToken->type == TOKEN_RPAREN)  // Funkce nemá žádný parametr
    {
        AstNode **paramKids = NULL;
        paramKids           = malloc(sizeof(AstNode *));

        // Přidání prázdného parametru jako poslední dítě uzlu parametrů
        Token    tokenEmpty = {.type = 0};
        AstNode *empty      = NULL;
        empty               = newLeaf(AST_EMPTY, tokenEmpty);

        paramKids[0] = empty;

        AstNode *paramNode = astNew(AST_PARAM, 1, paramKids);
        free(paramKids);
        stackAstNodePush(stackA, paramNode);

        AstNode *params = stackAstNodeRead(stackA);
        stackAstNodePop(stackA);
        AstNode *identif = stackAstNodeRead(stackA);
        stackAstNodePop(stackA);

        AstNode **kids = malloc(sizeof(AstNode *) * 2);
        if (kids == NULL)
        {
            printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
            freeAstStack(stackA);
            freePrecedenceStack(stackP);
            memorySafeExit(SYNTAX_ERROR);
        }

        kids[0] = identif;
        kids[1] = params;

        // Vytvoření kořene volání prázdné funkce
        AstNode *functionCallRoot = astNew(AST_FUNC_CALL, 2, kids);
        free(kids);
        stackAstNodePush(stackA, functionCallRoot);

        if (actualToken->type != TOKEN_NEW_LINE) {
            // pokračuji dále, mohu očekávat například sčítání
            *actualToken = lexerTokenNext();
        }
        if (actualToken->type == TOKEN_NEW_LINE) {
            *endFlag = TOKEN_PSA_END;
        }

    } else { // Funkce s jedním a více parametry

        int paramCount = 0;
        while (actualToken->type != TOKEN_RPAREN) {
            // zpracování výrazu parametru funkce
            AstNode *param = parseExpression(actualToken, 0);
            if (DEBUG)
                printLog("\t\t\t\tParam executed\n");

            paramCount++;
            // vložení pamametru na zásobník uzlů AST stromu
            stackAstNodePush(stackA, param);
            // Pokud je načtena čárka, je načten další parametr
            if (actualToken->type == TOKEN_COMMA) {
                *actualToken = lexerTokenNext();
                if (actualToken->type == TOKEN_RPAREN)  // za čárkou je hned zavorka -> syntaktická chyba
                {
                    printError(SYNTAX_ERROR, "(PSA): Za oddělovačem parametru funkce následuje pravá závorka [(,)]\n");
                    memorySafeExit(SYNTAX_ERROR);
                }
            }
        }

        AstNode **paramKids = NULL;
        paramCount++;  // zvyšuji o 1 protože potřebuji místo i pro uzel EMTPY
        paramKids = malloc(sizeof(AstNode *) * (paramCount));

        if (paramKids == NULL)
        {
            printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
            freeAstStack(stackA);
            freePrecedenceStack(stackP);
            memorySafeExit(SYNTAX_ERROR);
        }

        // Přidání prázdného parametru jako poslední dítě uzlu parametrů
        Token    tokenEmpty = {.type = 0};
        AstNode *empty      = NULL;
        empty               = newLeaf(AST_EMPTY, tokenEmpty);
        stackAstNodePush(stackA, empty);

        // Cyklem ze zásobníku seberu napočítaný počet parametrů
        for (int i = 0; i < paramCount; ++i) {
            // kontrola, zda zásobník, ze kterého beru parametry funce není prázdný
            if (stackAstNodeIsEmpty(stackA)) {
                printError(SYNTAX_ERROR, "(PSA): málo detí na AST zásobníku pri redukcii\n");
                memorySafeExit(SYNTAX_ERROR);
            }
            AstNode *kid = stackAstNodeRead(stackA);
            stackAstNodePop(stackA);
            paramKids[paramCount - 1 - i] = kid; // vkládám odzadu
        }
        AstNode *paramNode = astNew(AST_PARAM, paramCount, paramKids);
        free(paramKids);
        // na zásobníku zůstává identifikátor a uzel parametrů
        stackAstNodePush(stackA, paramNode);

        AstNode **functionKids = malloc(sizeof(AstNode *) * 2);

        if (functionKids == NULL)
        {
            printError(SYNTAX_ERROR, "(PSA) pravidlo gramatiky nebylo nalezeno\n");
            freeAstStack(stackA);
            freePrecedenceStack(stackP);
            memorySafeExit(SYNTAX_ERROR);
        }
        // načtení parametrů
        AstNode *params        = stackAstNodeRead(stackA);
        stackAstNodePop(stackA);
        // načtení idetifikátoru funkce (jejího názvu)
        AstNode *identif = stackAstNodeRead(stackA);
        stackAstNodePop(stackA);

        functionKids[0] = identif; // levý uzel
        functionKids[1] = params; // pravý uzel

        AstNode *functionRoot = astNew(AST_FUNC_CALL, 2, functionKids);
        free(functionKids);
        stackAstNodePush(stackA, functionRoot);

        if (actualToken->type != TOKEN_NEW_LINE) {
            *actualToken = lexerTokenNext();
        }
        if (actualToken->type == TOKEN_NEW_LINE) {
            *endFlag = TOKEN_PSA_END;
        }
        // Debug
        if (DEBUG) {
            printLog("ZASOBNIK PO ZPRACOVANI FUNKCE: ");
            printStack(stackP);
        }

        // Token expression;
        // expression.type = TOKEN_EXPRESSION;
        // stackPrecedencePush(stackP, expression);
    }
}

/**
 * @brief Funkce provede precedenční analýzu nad výrazem, v případě,
 *         že je výraz správný, vrací ukazatel na kořen stromu, který vytvoří
 * @param token aktuální token, který přišel od překladače
 * @param flag Celočíselná hodnota udávající, kde byla funkce volána (výraz ve v podmínce, ...)
 * @return Funkce navrací ukazatel na uzel stromu vytvoření precedeční analýzou
 */
AstNode *parseExpression(Token *token, int flag) {
    if (DEBUG)
        printLog("%d FLAG\n", flag);

    // zásobník pro precedeční analýzu
    DynamicPrecedenceStackPtr stackP = stackPrecedenceConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    // Zásobník uzlů, na které budou odkládány zpracované výrazy
    DynamicAstNodeStackPtr stackA = stackAstNodeConstruct(DEFAULT_INIT_ELEMENT_COUNT);

    Token actualToken = *token;  // aktuální token na vstupu, se kterým pracuji
    switch (actualToken.type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_STRING:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_KEYWORD_IS:
        case TOKEN_KEYWORD_NUM:
        case TOKEN_KEYWORD_STRING:
        case TOKEN_KEYWORD_NULL:
        case TOKEN_STRING_BLOCK:
        case TOKEN_HEXA:
        case TOKEN_KEYWORD_NULL_SMALL:
        case TOKEN_FLOAT_EXP:
        case TOKEN_INTEGER_EXP:
            token->value.identifier = NULL;  // Překladač oevzdá vlastnictví PSA
            break;

        default:
            break;
    }
    int   endFlag = 0;
    Token dollar;  // $
    dollar.type = TOKEN_DOLLAR;
    Token stop;  // >
    stop.type = TOKEN_STOP;


    int poradiCount = 1;  // pomocná proměnná pro kontrolu pořadí operací, slouží pro DEBUG

    stackPrecedencePush(stackP, dollar);  // vložení $ na zasobnik
    Token StackTop;

    // Pomocná proměnná pro kontrolu povoleného prvního tokenu
    bool firstOK = false;
    // Pomocna proměnná pro kontrolu prázdnosti podmínky
    bool checkEmptyIf = false;
    // Kontrolní proměnná pro implementaci možnosti nového řádku za operátorem
    bool expectingNewLineAfterOperator = true;
    // Pomocná proměnná pro implementaci zajištění kontroly ukočení výrazu pravou závorkou
    bool expectingRPAREN = false;
    // Pomocná proměnná pro zpracování funkce ve výrazu
    bool functionCall = false;
    bool checkIs      = false;
    // dokud neni na zasobniku jenom $E && nemam na vstupu token, ktery mi vše ukoncuje
    while (actualToken.type != TOKEN_PSA_END)  //
    {
        StackTop = stackPrecedenceRead(stackP);
        // pokud je stacktop expression, presunu ukazatel na vrchol a pozici zpet
        if (StackTop.type == TOKEN_EXPRESSION) {
            Token tmpTop = stackPrecedenceRead(stackP);
            // potrebuji si prvne vyhodit prvek a potom jej vlozit zpet
            stackPrecedencePop(stackP);
            StackTop = stackPrecedenceRead(stackP);
            stackPrecedencePush(stackP, tmpTop);
        }

        if (actualToken.type == TOKEN_LPAREN) {
            expectingRPAREN = true;
        }

        // ošetření, že za klíčovým slovem is není nevalidní token
        if (checkIs) {
            if (DEBUG)
                printLog("actualToken %s\n", tokenTypeToString(actualToken.type));

            if (operatorToPush(StackTop) == INDEX_IS) {
                // Možné případy pravé strany u klíčového slova Is
                if (actualToken.type != TOKEN_KEYWORD_STRING && actualToken.type != TOKEN_KEYWORD_NUM &&
                    actualToken.type != TOKEN_KEYWORD_NULL && actualToken.type != TOKEN_LPAREN)  // Doplnit popripade
                {
                    printError(SYNTAX_ERROR, "(PSA) Za klicovym slovem Is nasleduje nezname klicove slovo: %s\n ",
                               tokenTypeToString(actualToken.type));
                    freePrecedenceStack(stackP);
                    freeAstStack(stackA);
                    tokenPrecedenceFree(&actualToken);
                    token = NULL;
                    memorySafeExit(SYNTAX_ERROR);
                    break;
                }
            }
            checkIs = false;  // Kontrola proběhla, resetuji hodnotu
        }

        // nastavení hodnoty pro kontrolu aktuálního tokenu v další iteraci programu
        if (actualToken.type == TOKEN_KEYWORD_IS) {
            checkIs = true;
        }

        if (actualToken.type == TOKEN_EOF) {
            printError(SYNTAX_ERROR, "(PSA) Narazilo na konec souboru, nemelo se tak stat\n");
            freeAstStack(stackA);         // uvolneni zásobníku uzlů
            freePrecedenceStack(stackP);  // uvolnění zásobníku výrazů
            memorySafeExit(SYNTAX_ERROR);
            break;
        }

        if (!checkEmptyIf)  // voláno pouze jednou při prvním volání sestavení výrazu v if bloku
        {
            checkEmptyIf = true;
            // prevence prazdneho if statementu
            if (flag == -1 && actualToken.type == TOKEN_RPAREN) {
                printError(SYNTAX_ERROR, "(PSA) Volano nad prazdnym cyklem, podminkou...\n");
                freeAstStack(stackA);
                freePrecedenceStack(stackP);
                memorySafeExit(SYNTAX_ERROR);
                break;
            }
        }
        // Kontrola pro možnost vložení nového řádku za operátorem
        if (expectingNewLineAfterOperator && endFlag != TOKEN_PSA_END)  // očekávám znak nového řádku za operátorem
        {
            if (operatorToPush(StackTop) != INDEX_ID && operatorToPush(StackTop) != INDEX_DOLLAR &&
                operatorToPush(StackTop) != INDEX_RP) {
                if (actualToken.type == TOKEN_NEW_LINE) {
                    actualToken = lexerTokenNext();               // posouvám se na další token
                    if (operatorToPush(actualToken) != INDEX_ID)  // dalsi token neni identifikator, cislo, ...
                    {
                        printError(SYNTAX_ERROR,
                                   "(PSA) PSA ocekavalo identifikator za operatorem, ale nedostalo jej\n");
                        freeAstStack(stackA);
                        freePrecedenceStack(stackP);
                        memorySafeExit(SYNTAX_ERROR);
                        break;
                    }

                    expectingNewLineAfterOperator = false;  // už jej znovu neočekávám
                }
            }
        }
        // Kontrola pro možnost vložení nového řádku za operátorem
        if (expectingNewLineAfterOperator == false && actualToken.type == TOKEN_NEW_LINE &&
            (operatorToPush(StackTop) == INDEX_ID || operatorToPush(StackTop) == INDEX_DOLLAR)) {
            endFlag = TOKEN_PSA_END;
        }
        // Nastavení pomocné proměnné pro možnost vložení nového řádku za operátorem
        if (operatorToPush(actualToken) != INDEX_ID &&
            (operatorToPush(StackTop) == INDEX_ID || operatorToPush(StackTop) == INDEX_DOLLAR)) {
            expectingNewLineAfterOperator = true;
        } else {
            expectingNewLineAfterOperator = false;
        }

        // Kontrola povoleného typu prvního tokenu
        if (!firstOK) {
            firstOK = checkFirstTokenType(actualToken);
        }

        // Spuštění PSA nad funkcí, která je součástí výrazu
        if (!functionCall) {
            if (actualToken.type == TOKEN_LPAREN && StackTop.type == TOKEN_IDENTIFIER) {
                firstOK      = true;
                functionCall = true;
                functionCalled(stackP, stackA, &actualToken, &endFlag);  // zpracování celé funkce
                functionCall = false;
                continue;  // pokud
            }
        }

        // Podmínky, které mohou nastat při ukočování výrazu
        if ((flag == -1 && actualToken.type == TOKEN_RPAREN && StackTop.type == TOKEN_DOLLAR)
            // ukonceni pouze na novy radek
            || (actualToken.type == TOKEN_NEW_LINE && StackTop.type == TOKEN_DOLLAR)
            // оddělení výrazů ve funkce čárkou
            || (actualToken.type == TOKEN_COMMA && StackTop.type == TOKEN_DOLLAR)
            // ukončení funkce pravou závorkou
            || (actualToken.type == TOKEN_RPAREN && StackTop.type == TOKEN_DOLLAR && flag == 1)
            // ukonceni volani vyrazu v jednoradkove funkci
            || (actualToken.type == TOKEN_BLOCK_END && flag == -2 && StackTop.type == TOKEN_DOLLAR)
            // volani funkce pouze s jednim parametrem
            ||
            (actualToken.type == TOKEN_RPAREN && flag == 0 && StackTop.type == TOKEN_DOLLAR &&
             expectingRPAREN ==
                 false)) {  // abych mohl koncit potrebuji mit na vstupu token new line a u zasobniku ukazovat jen na $
            if (DEBUG) {
                printLog("splnila se podminka\n");
                printStack(stackP);
            }

            endFlag = TOKEN_PSA_END;
        }

        if (DEBUG) {
            printLog("%d Aktualni token %s a stacktop: %s [%d, %d]\n", poradiCount++,
                     tokenTypeToString(actualToken.type), tokenTypeToString(stackPrecedenceRead(stackP).type),
                     operatorToPush(StackTop), operatorToPush(actualToken));
            printStack(stackP);
        }

        if (endFlag == TOKEN_PSA_END) {
            // cyklus probíhá dokud není na zásobníku pouze dolar a výraz
            while (!checkDollarExpression(stackP)) {
                // upravuji výraz dokud nedostanu na zásobníku pouze dolar a výraz
                findPrecedenceRule(stackP, stackA);
                if (DEBUG) {
                    printLog("Final: ");
                    printStack(stackP);
                }
            }

            AstNode *root = stackAstNodeRead(stackA);
            if (DEBUG) {
                printLog("string %s\n", astNodeTypeToString(root->kind));
                printLog("Final stack: ");
                printStack(stackP);
            }

            stackAstNodeDestruct(stackA);  // uvolneni zásobníku uzlů
            freePrecedenceStack(stackP);   // uvolnění zásobníku výrazů
            *token = actualToken;
            return root;  // koren, ktery vracim Kubovi na cely vyraz
        }

        // řádky a sloupce
        switch (PSA_TABLE[operatorToPush(StackTop)][operatorToPush(actualToken)]) {
            case R:  // >
                if (DEBUG)
                    printLog("\tOperace >\n");
                // najde pravidlo z gramatiky a na zásobník AST uzlů vloží výraz
                findPrecedenceRule(stackP, stackA);  
                if (DEBUG) {
                    printLog("\t\t\t%d > (%d, %d) ", poradiCount++, operatorToPush(StackTop),
                             operatorToPush(actualToken));
                    printStack(stackP);
                }

                break;

            case L:  // <
                
                if (DEBUG){
                    printLog(" < (%d, %d) ", operatorToPush(StackTop), operatorToPush(actualToken));
                    printLog("\tOperace <\n");
                }
                // vložení zarážky před aktuálně vkládaný id/operátor
                if (stackPrecedenceRead(stackP).type == TOKEN_DOLLAR ||
                    stackPrecedenceRead(stackP).type != TOKEN_EXPRESSION)
                {
                    if (DEBUG)
                        printLog("\t\t%d budu hazet na stack klasicky\n", poradiCount++);

                    stackPrecedencePush(stackP, stop);  // <
                    stackPrecedencePush(stackP, actualToken);

                } else {  // vložení ukočovací zarážky ještě před aktuální ID, které je na zásobníku
                    if (DEBUG)
                        printLog("\t\t%d budu hazet na stack slozite\n", poradiCount++);

                    Token topTmp = stackPrecedenceRead(stackP);
                    stackPrecedencePop(stackP);
                    stackPrecedencePush(stackP, stop);         // <
                    stackPrecedencePush(stackP, topTmp);       // to co tam bylo
                    stackPrecedencePush(stackP, actualToken);  // aktualni token
                }
                if (DEBUG) {
                    printLog("\t\t\t%d < (%d, %d) ", poradiCount++, operatorToPush(StackTop),
                             operatorToPush(actualToken));
                    printStack(stackP);
                }
                actualToken = lexerTokenNext();
                break;

            case E:  // =
                stackPrecedencePush(stackP, actualToken);
                if (DEBUG) {
                    printLog("\t\t\t%d = (%d, %d) ", poradiCount++, operatorToPush(StackTop),
                             operatorToPush(actualToken));
                    printStack(stackP);
                }
                // načtení dalšího tokenu
                actualToken = lexerTokenNext();
                break;

            default:  // stav ktery nesmi nastat
                printError(SYNTAX_ERROR,
                           "(PSA) Zadany vyraz neni syntakticky spravny (neocekavany znak nebo samostatne stojici "
                           "operator)\n");
                freeAstStack(stackA);
                freePrecedenceStack(stackP);
                memorySafeExit(SYNTAX_ERROR);
                break;
        }
    }
    return NULL;  // Nastala chyba
}

#endif

/* Konec souboru expression.c */