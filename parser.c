/**
 * Fakulta informačných technologií VUT Brno
 *
 * Súbor pre implementáciu lexikálnej analýzy
 * pre projekt do predmetu IFJ
 * @file  parser.c
 * @author Jakub Jambrozi (xjambrj00)
 * upravované: 22.10.2025
 */

#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "expressions.h"
#include "lexer.h"
#include "support_functions.h"
#include "symbol.h"

Token                 *gLookHead = NULL;
DynamicAstNodeStackPtr astStack  = NULL;
Token                  token;
AstNode               *astRoot  = NULL;
DynamicSymbolStackPtr  stack    = NULL;
bool                   function = false;
bool                   block    = false;

AstNode *parser() {
    astStack = stackAstNodeConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    stack    = stackSymbolConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    if (stack == NULL || astStack == NULL) {
        printError(INTERNAL_ERROR, "Chyba alokácie");
        memorySafeExit(INTERNAL_ERROR);
    }

    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_EOF)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_PROGRAM)));
    token = lexerTokenNext();

    Symbol stackTop;
    gLookHead = &token;

    while (!stackSymbolIsEmpty(stack)) {
        stackTop = stackSymbolRead(stack);
        // printLog("%d\n",stackTop.isReduce);
        if (stackTop.isReduce) {
            size_t n = stackTop.reduceArity;

            AstNode **kids = NULL;

            if (n > 0) {
                kids = malloc(sizeof(AstNode *) * n);
                if (kids == NULL) {
                    printError(INTERNAL_ERROR, "Chyba alokácie pamäte pre deti AST uzla\n");
                    memorySafeExit(INTERNAL_ERROR);
                }
            }

            if (n && !kids) {
                printError(INTERNAL_ERROR, "AST alloc");
                memorySafeExit(INTERNAL_ERROR);
            }

            // fprintf(stderr, "[REDUCE] %s / arity=%zu  (predpoklad detí=%zu)\n",
            // astNodeTypeToString(stackTop.reduceKind), n, n);
            //  na vrchu sú pravé deti; preto vkladáme odzadu
            for (size_t i = 0; i < n; ++i) {
                if (stackAstNodeIsEmpty(astStack)) 
                {
                    // fprintf(stderr, "[REDUCE-FAIL] %s chce %zu. Na AST zásobníku došli deti pri i=%zu.\n",
                    printError(INTERNAL_ERROR, "Vnútorná chyba: málo detí na AST zásobníku pri redukcii\n");
                    memorySafeExit(INTERNAL_ERROR);
                }
                AstNode *kid = stackAstNodeRead(astStack);
                stackAstNodePop(astStack);
                kids[n - 1 - i] = kid;
            }
            AstNode *parent = NULL;
            if (stackTop.reduceKind == AST_PARAM) {
                parent = buildParamFlat(n, kids);
            } else if (stackTop.reduceKind == AST_STATEMENT_LIST) {
                parent = buildStmtListFlat(n, kids);
            } else if (stackTop.reduceKind == AST_MAIN_CLASS_BLOCK) {
                parent = buildClassBlockFlat(n, kids);
            } else if (stackTop.reduceKind == AST_IF_CHAIN) {
                parent = buildIfChainFlat(n, kids);
            } else {
                parent = astNew(stackTop.reduceKind, n, kids);
            }
            stackSymbolPop(stack);
            free(kids);
            stackAstNodePush(astStack, parent);
            continue;
        }

        else if (stackTop.isTerminal) {
            // printLog("%d\n",token.type);
            // printLog("%d\n",stackTop.symbol);
            if ((TokenType)stackTop.symbol == token.type) {
                bool ok = true;
                if (stackTop.valueNeeded) {
                    ok = (token.value.identifier && stackTop.value.identifier &&
                          strcmp(stackTop.value.identifier, token.value.identifier) == 0);
                }

                if (!ok) {
                    printError(SYNTAX_ERROR, "Nesúlad hodnoty terminálu\n");
                    memorySafeExit(SYNTAX_ERROR);
                }

                if (stackTop.captureValue) {
                    switch (token.type) {
                        case TOKEN_STRING:
                            stackAstNodePush(astStack, newLeaf(AST_STRING_LIT, token));
                            break;
                        case TOKEN_INTEGER:
                            stackAstNodePush(astStack, newLeaf(AST_INT_LIT, token));
                            break;
                        case TOKEN_FLOAT:
                            stackAstNodePush(astStack, newLeaf(AST_FLOAT_LIT, token));
                            break;
                        case TOKEN_IDENTIFIER:
                            stackAstNodePush(astStack, newLeaf(AST_IDENTIFIER, token));
                            break;
                        case TOKEN_GLOBAL_ID:
                            stackAstNodePush(astStack, newLeaf(AST_GLOBAL_ID, token));
                            break;
                        case TOKEN_HEXA:
                            stackAstNodePush(astStack, newLeaf(AST_INT_LIT, token));
                            break;
                        case TOKEN_TRUE:
                            stackAstNodePush(astStack, newLeaf(AST_BOOL_LIT, token));
                            break;
                        case TOKEN_FALSE:
                            stackAstNodePush(astStack, newLeaf(AST_BOOL_LIT, token));
                            break;
                        default:
                            break;
                    }
                }

                // printLog("ok\n");
                tokenFree(&token);
                stackSymbolPop(stack);
                token = lexerTokenNext();
            } else {
                printError(SYNTAX_ERROR, "Nesprávne poradie tokenov\n");
                memorySafeExit(SYNTAX_ERROR);
            }
        } else if (!stackTop.isTerminal && !stackTop.isReduce) {
            switch (stackTop.symbol) {
                case N_PROGRAM:
                    programFunc(stack);
                    break;
                case N_PROLOG:
                    prologFunc(stack);
                    break;
                case N_MAIN_CLASS:
                    mainClassFunc(stack);
                    break;
                case N_MAIN_CLASS_BLOCK:
                    mainClassBlockFunc(stack, token);
                    break;
                case N_BLOCK:
                    blockFunc(stack);
                    break;
                case N_BLOCK_CONTENT:
                    blockContentFunc(stack, token);
                    break;
                case N_STMT_TAIL:
                    smthTailFunc(stack, token);
                    break;
                case N_STMT_LIST:
                    smthListFunc(stack, token);
                    break;
                case N_FUNC_DEC:
                    funcDecFunc(stack);
                    break;
                case N_FUNC_DEC_TAIL:
                    funcDecTailFunc(stack, token);
                    break;
                case N_PARAM:
                    paramFunc(stack, token);
                    break;
                case N_PARAM_LIST:
                    paramListFunc(stack, token);
                    break;
                case N_VAR:
                    varFunc(stack);
                    break;
                case N_VAR_EXT:
                    varExtFunc(stack, token);
                    break;
                case N_VAR_ARG:
                    varArgFunc(stack, token);
                    break;
                case N_VAR_ARG_LIST:
                    varArgListFunc(stack, token);
                    break;
                case N_CALL_ASSIGN:
                    callAssignFunc(stack);
                    break;
                case N_CALL_ASSIGN_LIST:
                    callAssignListFunc(stack, token);
                    break;
                case N_GLOBAL_ASSIGN:
                    globalAssignFunc(stack);
                    break;
                case N_IF:
                    ifFunc(stack);
                    break;
                case N_WHILE:
                    whileFunc(stack);
                    break;
                case N_FOR:
                    forFunc(stack);
                    break;
                case N_BREAK:
                    breakFunc(stack);
                    break;
                case N_CONTINUE:
                    continueFunc(stack);
                    break;
                case N_RETURN:
                    returnFunc(stack);
                    break;
                case N_RETURN_EXPR:
                    returnExprFunc(stack, token);
                    break;
                case N_EXPR:
                    exprFunc(stack, gLookHead, astStack, 0);
                    token = *gLookHead;
                    printLog("precedencna odovzdala token jeho type je = %d\n", token.type);
                    break;
                case N_C_EXPR:
                    exprFunc(stack, gLookHead, astStack, 1);
                    token = *gLookHead;
                    printLog("precedencna odovzdala token jeho type je = %d\n", token.type);
                    break;
                case N_L_EXPR:
                    exprFunc(stack, gLookHead, astStack, -1);
                    printLog("%d", 1);
                    token = *gLookHead;
                    printLog("precedencna odovzdala token jeho type je = %d\n", token.type);
                    break;
                case N_O_EXPR:
                    exprFunc(stack, gLookHead, astStack, -2);
                    token = *gLookHead;
                    printLog("precedencna odovzdala token jeho type je = %d\n", token.type);
                    break;

                case N_NEW_LINE_CHECK:
                    newLineFunc(stack, token);
                    break;
                case N_OPT_NEWLINES:
                    optNewlinesFunc(stack, token);
                    break;
                case N_BLOCK_CONTENT_CHECK:
                    blockContentFuncCheck(stack, token);
                    break;
                case N_CALL_IFJ:
                    callIfjfunc(stack, token);
                    break;
                case N_ELSE_CHAIN:
                    elseChainFunc(stack, token);
                    break;
                case N_ELSE_ITEM:
                    elseItemFunc(stack, token);
                    break;
                case N_CALL_IFJ_CHECK:
                    callIfjCheckfunc(stack, token);
                    break;
                case N_ASSIGN_IFJ_ID_CHECK:
                    AssignIfjIdCheckFunc(stack, token);
                    break;
                default:
                    printError(INTERNAL_ERROR, "Nespravny Neterminal\n");
                    memorySafeExit(INTERNAL_ERROR);
                    break;
            }
        }
    }

    if (stackAstNodeIsEmpty(astStack)) {
        printError(SYNTAX_ERROR, "AST je prázdny\n");
        memorySafeExit(SYNTAX_ERROR);
    }

    astRoot = stackAstNodeRead(astStack);
    stackSymbolDestruct(stack);
    stack = NULL;
    stackAstNodeDestruct(astStack);
    astStack = NULL;
    printLog("PARSER IS DONE\n");
    return astRoot;
}

void pushCheck(bool boolean) {
    if (!boolean) {
        printError(INTERNAL_ERROR, "Chyba pushovania na stack\n");
        memorySafeExit(INTERNAL_ERROR);
    }
}

void programFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_PROGRAM, 2)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_MAIN_CLASS)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_PROLOG)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
}

void prologFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_PROLOG, 0)));

    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_IFJ)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_FOR)));
    Token ifj25;
    ifj25.value.identifier = "ifj25";
    ifj25.type             = TOKEN_STRING;
    pushCheck(stackSymbolPush(stack, makeTermValue(ifj25)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_IMPORT)));
}

void mainClassFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_MAIN_CLASS, 2)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_NEW_LINE_CHECK)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_BLOCK_END)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_MAIN_CLASS_BLOCK)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_BLOCK_STAR)));
    pushCheck(stackSymbolPush(stack, makeTermValueAndCaptureIdentifier("Program")));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_CLASS)));
}

void mainClassBlockFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    if (token.type == TOKEN_KEYWORD_STATIC) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_MAIN_CLASS_BLOCK, 2)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_MAIN_CLASS_BLOCK)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_FUNC_DEC)));
        return;
    } 
    else if (token.type == TOKEN_NEW_LINE) 
    {
        pushCheck(stackSymbolPush(stack, makeNonterm(N_MAIN_CLASS_BLOCK)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    } 
    else if (token.type == TOKEN_BLOCK_END) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_MAIN_CLASS_BLOCK, 0)));
        return;
    }

    else 
    { 
        printError(SYNTAX_ERROR, "Očakávaný 'static' alebo '}' v Main_class_block");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void blockFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_BLOCK, 1)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_BLOCK_END)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK_CONTENT_CHECK)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_BLOCK_STAR)));
}

void blockContentFuncCheck(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);

    if (token.type == TOKEN_BLOCK_END) {
        // {} -> prázdny zoznam
        if (function) {
            pushCheck(stackSymbolPush(stack, makeReduce(AST_RETURN, 1)));
        }
        pushCheck(stackSymbolPush(stack, makeReduce(AST_NULL, 0)));
        function = false;
        return;
    }

    if (token.type == TOKEN_NEW_LINE) {
        pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK_CONTENT)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        function = false;
        return;
    }

    switch (token.type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_HEXA:
        case TOKEN_KEYWORD_NULL_SMALL:
        case TOKEN_FLOAT_EXP:
        case TOKEN_INTEGER_EXP:
        case TOKEN_STRING_BLOCK:
            pushCheck(stackSymbolPush(stack, makeReduce(AST_STATEMENT_LIST, 1)));

            if (function) {
                pushCheck(stackSymbolPush(stack, makeReduce(AST_RETURN, 1)));
                function = false;
            }
            pushCheck(stackSymbolPush(stack, makeNonterm(N_O_EXPR)));
            return;
        default:
            printError(SYNTAX_ERROR, "Po '{' vo funkcii očakávam NEW_LINE, '}', alebo one-liner výraz");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void blockContentFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    if (token.type == TOKEN_NEW_LINE) {
        pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK_CONTENT)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }

    if (token.type == TOKEN_BLOCK_END) {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_STATEMENT_LIST, 0)));
        return;
    }
    pushCheck(stackSymbolPush(stack, makeReduce(AST_STATEMENT_LIST, 2)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_STMT_TAIL)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_STMT_LIST)));
}

void smthTailFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    if (token.type == TOKEN_NEW_LINE) {
        pushCheck(
            stackSymbolPush(stack, makeNonterm(N_BLOCK_CONTENT)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }
    if (token.type == TOKEN_BLOCK_END) {
        printError(SYNTAX_ERROR, "Pred '}' musí byť NEW_LINE\n");
        memorySafeExit(SYNTAX_ERROR);
    }
    pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
}

void smthListFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    switch (token.type) {
        case TOKEN_KEYWORD_RETURN:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_RETURN)));
            return;
        case TOKEN_KEYWORD_VAR:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_VAR)));
            return;
        case TOKEN_KEYWORD_IF:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_IF)));
            return;
        case TOKEN_KEYWORD_WHILE:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_WHILE)));
            return;
        case TOKEN_KEYWORD_FOR:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_FOR)));
            return;
        case TOKEN_KEYWORD_BREAK:
            if (block) {
                pushCheck(stackSymbolPush(stack, makeNonterm(N_BREAK)));
            } else {
                printError(SYNTAX_ERROR, "break vo funkcii mimo while a for\n");
                memorySafeExit(SYNTAX_ERROR);
            }
            break;
        case TOKEN_KEYWORD_CONTINUE:
            if (block) {
                pushCheck(stackSymbolPush(stack, makeNonterm(N_CONTINUE)));
            } else {
                printError(SYNTAX_ERROR, "break vo funkcii mimo while a for\n");
                memorySafeExit(SYNTAX_ERROR);
            }
            break;
        case TOKEN_IDENTIFIER:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_CALL_ASSIGN)));
            return;

        case TOKEN_KEYWORD_IFJ:
            pushCheck(stackSymbolPush(stack, makeReduce(AST_IFJ_CALL, 2)));
            pushCheck(stackSymbolPush(stack, makeNonterm(N_CALL_IFJ)));
            pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_IFJ)));
            return;
        case TOKEN_GLOBAL_ID:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_GLOBAL_ASSIGN)));
            return;
        case TOKEN_BLOCK_STAR:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
            return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token na začiatku príkazu\n");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void funcDecFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeNonterm(N_FUNC_DEC_TAIL)));
    pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_IDENTIFIER)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_STATIC)));
}

void funcDecTailFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);

    if (token.type == TOKEN_LPAREN) {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_FUNC_DEC, 3)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_PARAM)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
        function = true;
    } else if (token.type == TOKEN_ASSIGN) {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_SETTER, 3)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 2)));
        pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
        pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_IDENTIFIER)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_ASSIGN)));
        function = true;
    } else if (token.type == TOKEN_BLOCK_STAR) {
        // Vetva: Block
        pushCheck(stackSymbolPush(stack, makeReduce(AST_GETTER, 3)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
        pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 1)));
        pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
        function = true;
    } else {
        printError(SYNTAX_ERROR, "Očakávané '(' alebo '=' vo Func_dec_tail\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}
void paramFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);

    if (token.type == TOKEN_IDENTIFIER) {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 2)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_PARAM_LIST)));
        pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_IDENTIFIER)));
    } else if (token.type == TOKEN_RPAREN) {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 1)));  // eps
        pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));

    } else {
        printError(SYNTAX_ERROR, "Očakávaný identifikátor alebo ')' v Param\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void paramListFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    if (token.type == TOKEN_COMMA) {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 2)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_PARAM_LIST)));
        pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_IDENTIFIER)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_COMMA)));

    } else if (token.type == TOKEN_RPAREN) {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
    } else {
        printError(SYNTAX_ERROR, "Očakávaný ',' alebo ')' v Param_list");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void varFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_VAR, 2)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_VAR_EXT)));
    pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_IDENTIFIER)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_VAR)));
}

void varExtFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    if (token.type == TOKEN_ASSIGN) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_ASSIGN, 1)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_ASSIGN_IFJ_ID_CHECK)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_ASSIGN)));
    }

    else if (token.type == TOKEN_NEW_LINE || TOKEN_BLOCK_END) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
    } else 
    {
        printError(SYNTAX_ERROR, "chyba za var\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void varArgFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    switch (token.type) 
    {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_HEXA:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_KEYWORD_NULL_SMALL:
        case TOKEN_FLOAT_EXP:
        case TOKEN_INTEGER_EXP:
        case TOKEN_STRING_BLOCK:
            pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 2)));
            pushCheck(stackSymbolPush(stack, makeNonterm(N_VAR_ARG_LIST)));
            pushCheck(stackSymbolPush(stack, makeNonterm(N_EXPR)));
            return;
        case TOKEN_LPAREN:

            pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 2)));
            pushCheck(stackSymbolPush(stack, makeNonterm(N_VAR_ARG_LIST)));
            pushCheck(stackSymbolPush(stack, makeNonterm(N_L_EXPR)));
            return;

        case TOKEN_RPAREN:
            pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 1)));
            pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
            return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token vo Var_arg\n");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void varArgListFunc(DynamicSymbolStackPtr stack, Token token) 
{
    stackSymbolPop(stack);
    if (token.type == TOKEN_COMMA) 
    {

        pushCheck(stackSymbolPush(stack, makeReduce(AST_PARAM, 2)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_VAR_ARG_LIST)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_C_EXPR)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_COMMA)));
    } else if (token.type == TOKEN_RPAREN) 
    {

        pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
    } else {
        printError(SYNTAX_ERROR, "Neočakávaný token vo var_arg_list\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void callAssignFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeNonterm(N_CALL_ASSIGN_LIST)));
    pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_IDENTIFIER)));
}

void callAssignListFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    if (token.type == TOKEN_ASSIGN) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_ASSIGN, 2)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_ASSIGN_IFJ_ID_CHECK)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_ASSIGN)));
    } else if (token.type == TOKEN_LPAREN) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_FUNC_CALL, 2))); 
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_VAR_ARG)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
    } else {
        printError(SYNTAX_ERROR, "Očakávané '=' alebo '(' v Call_assign_list");
        memorySafeExit(SYNTAX_ERROR);
    }
}
void globalAssignFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_GLOBAL_ID, 2)));
    pushCheck(stackSymbolPush(stack, makeReduce(AST_ASSIGN, 1)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_ASSIGN_IFJ_ID_CHECK)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_ASSIGN)));
    pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_GLOBAL_ID)));
}

void ifFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_IF_CHAIN, 2)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_ELSE_CHAIN)));
    pushCheck(stackSymbolPush(stack, makeReduce(AST_IF, 2)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
    pushCheck(stackSymbolPush(stack, makeReduce(AST_CONDITION, 1)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_L_EXPR)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_IF)));
}

void elseChainFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);

    if (token.type == TOKEN_KEYWORD_ELSE) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_IF_CHAIN, 2)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_ELSE_CHAIN)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_ELSE_ITEM))); 
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_ELSE)));
        return;
    }
    pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
}

void elseItemFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);

    if (token.type == TOKEN_KEYWORD_IF) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_IF, 2)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
        pushCheck(stackSymbolPush(stack, makeReduce(AST_CONDITION, 1)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_L_EXPR)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_IF)));
        return;
    } 
    else if (token.type == TOKEN_BLOCK_STAR) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_ELSE, 1)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
        return;
    }

    printError(SYNTAX_ERROR, "Po 'else' očakávam 'if' alebo '{'");
    memorySafeExit(SYNTAX_ERROR);
}

void whileFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_WHILE, 2)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
    block = true;
    pushCheck(stackSymbolPush(stack, makeReduce(AST_CONDITION, 1)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_L_EXPR)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_WHILE)));
}

void forFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_FOR, 2)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_BLOCK)));
    block = true;
    pushCheck(stackSymbolPush(stack, makeReduce(AST_CONDITION, 2)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_L_EXPR)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_IN)));
    pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_IDENTIFIER)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_FOR)));
}

void breakFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_BREAK, 0)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_BREAK)));
}

void continueFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_CONTINUE, 0)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_CONTINUE)));
}

void returnFunc(DynamicSymbolStackPtr stack) {
    stackSymbolPop(stack);
    pushCheck(stackSymbolPush(stack, makeReduce(AST_RETURN, 1)));
    pushCheck(stackSymbolPush(stack, makeNonterm(N_RETURN_EXPR)));
    pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_RETURN)));
}

void returnExprFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    switch (token.type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_HEXA:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_FLOAT_EXP:
        case TOKEN_INTEGER_EXP:
        case TOKEN_KEYWORD_NULL_SMALL:
        case TOKEN_STRING_BLOCK:
            pushCheck(stackSymbolPush(stack, makeNonterm(N_EXPR)));
            break;
        case TOKEN_LPAREN:
            pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
            pushCheck(stackSymbolPush(stack, makeNonterm(N_L_EXPR)));
            pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
            break;

        case TOKEN_NEW_LINE:
            pushCheck(stackSymbolPush(stack, makeReduce(AST_EMPTY, 0)));
            break;
        case TOKEN_RPAREN:
        default:
            printError(SYNTAX_ERROR, "Neočakávaný token v Return_expr\n");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void exprFunc(DynamicSymbolStackPtr stack, Token *gLookHead, DynamicAstNodeStackPtr astStack, int flag) {
    stackSymbolPop(stack);

    if (!gLookHead || !astStack) {
        printError(INTERNAL_ERROR, "exprFunc: chýbajú hooky\n");
        memorySafeExit(INTERNAL_ERROR);
    }

    AstNode *expr = parseExpression(gLookHead, flag);
    if (!expr) {
        printError(SYNTAX_ERROR, "Chybný výraz\n");
        memorySafeExit(SYNTAX_ERROR);
    }
    printLog("expr strom po skoncení funcke parseExpression\n");
    astDump(expr);

    pushCheck(stackAstNodePush(astStack, expr));
}

void newLineFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    if (token.type == TOKEN_NEW_LINE) {
        pushCheck(stackSymbolPush(stack, makeNonterm(N_NEW_LINE_CHECK)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    } else if (token.type == TOKEN_EOF) {
        return;
    } else {
        printError(SYNTAX_ERROR, "za koncom bloku class program chybny token\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void optNewlinesFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);
    if (token.type == TOKEN_NEW_LINE) 
    {
        pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }
}

void callIfjfunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);

    if (token.type == TOKEN_DOT) 
    {
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_VAR_ARG)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_LPAREN)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_CALL_IFJ_CHECK)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_OPT_NEWLINES)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_DOT)));
    } 
    else 
    {
        printError(SYNTAX_ERROR, "nespravne volanie vstavanej funkcie\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void callIfjCheckfunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);

    if (token.type != TOKEN_IDENTIFIER || token.value.identifier == NULL) 
    {
        printError(SYNTAX_ERROR, "Ocakavany nazov funkcie po 'ifj.'");
        memorySafeExit(SYNTAX_ERROR);
    }
    char  *words[] = {"read_str",  "read_num", "write", "floor", "str",      "length",
                      "substring", "strcmp",   "ord",   "chr",   "read_bool"};
    size_t count   = sizeof(words) / sizeof(words[0]);
    bool   allowed = false;
    for (size_t i = 0; i < count; i++) 
    {
        if (strcmp(token.value.identifier, words[i]) == 0) {
            allowed = true;
            break;
        }
    }
    if (allowed) 
    {
        pushCheck(stackSymbolPush(stack, makeTermWithValue(TOKEN_IDENTIFIER)));
        return;
    } else 
    {
        printError(SYNTAX_ERROR, "nespravne volanie vstavanej funkcie\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void AssignIfjIdCheckFunc(DynamicSymbolStackPtr stack, Token token) {
    stackSymbolPop(stack);

    if (token.type == TOKEN_KEYWORD_IFJ) 
    {
        pushCheck(stackSymbolPush(stack, makeReduce(AST_IFJ_CALL, 2)));
        pushCheck(stackSymbolPush(stack, makeNonterm(N_CALL_IFJ)));
        pushCheck(stackSymbolPush(stack, makeTerm(TOKEN_KEYWORD_IFJ)));
        return;
    }

    else 
    {
        pushCheck(stackSymbolPush(stack, makeNonterm(N_EXPR)));
    }
}

Symbol makeReduce(AstNodeType kind, size_t arity) {
    Symbol x      = {0};
    x.isReduce    = true;
    x.reduceKind  = kind;
    x.reduceArity = arity;
    return x;
}

Symbol makeNonterm(Nonterm nonTerm) {
    Symbol x = {0};
    x.symbol = nonTerm;
    return x;
}

Symbol makeTermValue(Token token) {
    Symbol x           = {0};
    x.isTerminal       = true;
    x.freeNeeded       = true;
    x.symbol           = token.type;
    x.valueNeeded      = true;
    x.value.identifier = token.value.identifier;
    return x;
}

Symbol makeTerm(TokenType tokenType) {
    Symbol x     = {0};
    x.isTerminal = true;
    x.symbol     = tokenType;
    return x;
}

Symbol makeTermWithValue(TokenType type) {
    Symbol x       = {0};
    x.freeNeeded   = true;
    x.isTerminal   = true;
    x.symbol       = type;
    x.captureValue = true;
    return x;
}

Symbol makeTermValueAndCaptureIdentifier(const char *exact) {
    Symbol x           = {0};
    x.isTerminal       = true;
    x.symbol           = TOKEN_IDENTIFIER;
    x.freeNeeded       = true;
    x.valueNeeded      = true;
    x.captureValue     = true;
    x.value.identifier = (char *)exact;
    return x;
}

void tokenFree(Token *token) {
    if (!token) {
        return;
    }

    switch (token->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_STRING:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_FLOAT_EXP:
        case TOKEN_HEXA:
        case TOKEN_INTEGER_EXP:
        case TOKEN_KEYWORD_BREAK:
        case TOKEN_KEYWORD_CLASS:
        case TOKEN_KEYWORD_CONTINUE:
        case TOKEN_KEYWORD_FOR:
        case TOKEN_KEYWORD_IF:
        case TOKEN_KEYWORD_IFJ:
        case TOKEN_KEYWORD_IMPORT:
        case TOKEN_KEYWORD_IN:
        case TOKEN_KEYWORD_IS:
        case TOKEN_KEYWORD_NULL:
        case TOKEN_KEYWORD_NULL_SMALL:
        case TOKEN_KEYWORD_NUM:
        case TOKEN_KEYWORD_STATIC:
        case TOKEN_KEYWORD_STRING:
        case TOKEN_KEYWORD_VAR:
        case TOKEN_KEYWORD_ELSE:
        case TOKEN_KEYWORD_WHILE:
        case TOKEN_KEYWORD_RETURN:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
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

void parserFree(void) {
    tokenFree(&token);
    gLookHead = NULL;
    if (astStack) 
    {
        while (!stackAstNodeIsEmpty(astStack)) 
        {
            AstNode *node = stackAstNodeRead(astStack);
            stackAstNodePop(astStack);
            astFreeWhole(node);
            node = NULL;
        }
        stackAstNodeDestruct(astStack);
        astStack = NULL;
    }
    if (astRoot) 
    {
        astFreeWhole(astRoot);
        astRoot = NULL;
    }
    if (stack) 
    {
        stackSymbolDestruct(stack);
        stack = NULL;
    }
}