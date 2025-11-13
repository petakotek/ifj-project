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

#include "lexer.h"
#include "support_functions.h"
#include "symbol.h"
#include "ast_strom.h"
#include "expr.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

AstNode *parser()
{
    DynamicAstNodeStackPtr AstStack = stackAstNodeConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    DynamicSymbolStackPtr  Stack    = stackSymbolConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    if (Stack == NULL || AstStack == NULL)
    {
        printError(INTERNAL_ERROR,"Chyba alokácie");
        memorySafeExit(INTERNAL_ERROR);
    }

    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_EOF)));
    pushCheck(stackSymbolPush(Stack,makeNonterm(N_Program)));
    Token token = lexer_token_next();
    Symbol Stack_top;

    Token *gLookahead = NULL;
    gLookahead = &token;




    while (!stackSymbolIsEmpty(Stack))
    {
        Stack_top = stackSymbolRead(Stack);
        //printf("%d\n",Stack_top.is_reduce);
        if (Stack_top.is_reduce)
        {
            size_t n = Stack_top.reduce_arity;


            AstNode **kids = NULL;

            if (n > 0)
            {
                kids = malloc(sizeof(AstNode*) * n);
                if (kids == NULL) {
                    printError(INTERNAL_ERROR, "Chyba alokácie pamäte pre deti AST uzla\n");
                    memorySafeExit(INTERNAL_ERROR);
                }
            }

            if (n && !kids)
            {
                printError(INTERNAL_ERROR, "AST alloc");
                memorySafeExit(INTERNAL_ERROR);
            }

            //fprintf(stderr, "[REDUCE] %s / arity=%zu  (predpoklad detí=%zu)\n", ast_node_type_to_string(Stack_top.reduce_kind), n, n);
            // na vrchu sú pravé deti; preto vkladáme odzadu
            for (size_t i = 0; i < n; ++i)
            {
                if (stackAstNodeIsEmpty(AstStack))
                {
                    //fprintf(stderr, "[REDUCE-FAIL] %s chce %zu. Na AST zásobníku došli deti pri i=%zu.\n", ast_node_type_to_string(Stack_top.reduce_kind), n, i);
                    printError(SYNTAX_ERROR, "Vnútorná chyba: málo detí na AST zásobníku pri redukcii\n");
                    memorySafeExit(SYNTAX_ERROR);
                }
                AstNode *kid = stackAstNodeRead(AstStack);
                stackAstNodePop(AstStack);
                kids[n - 1 - i] = kid;
            }
            AstNode *parent = NULL;
            if (Stack_top.reduce_kind == AST_PARAM)
            {
                parent = build_param_flat(n, kids);      // <<— PLOCHÝ ZOZNAM
            }
            else if (Stack_top.reduce_kind == AST_STATEMENT_LIST)
            {
                parent = build_stmtlist_flat(n, kids);
            }
            else if (Stack_top.reduce_kind == AST_MAIN_CLASS_BLOCK)
            {
                parent = build_classblock_flat(n, kids);
            }
            else if (Stack_top.reduce_kind == AST_IF_CHAIN) {
            parent = build_ifchain_flat(n, kids);   // <<< PRIDANÉ
            }
            else
            {
                parent = ast_new(Stack_top.reduce_kind, n, kids);
            }
            stackSymbolPop(Stack);
            free(kids);
            stackAstNodePush(AstStack,parent);
            continue;
        }

        else if(Stack_top.is_terminal)
        {

            //printf("%d\n",token.type);
            //printf("%d\n",Stack_top.symbol);
            if ((TokenType)Stack_top.symbol == token.type)
            {
                bool ok = true;
                if (Stack_top.value_needed)
                {
                    ok = (token.value.identifier && Stack_top.value.identifier && strcmp(Stack_top.value.identifier, token.value.identifier) == 0);
                }


                if (!ok) {
                    printError(SYNTAX_ERROR, "Nesúlad hodnoty terminálu\n");
                    memorySafeExit(SYNTAX_ERROR);
                }

                if (Stack_top.capture_value)
                {
                    switch (token.type) {
                    case TOKEN_STRING:
                        stackAstNodePush(AstStack, new_leaf(AST_STRING_LIT, token));
                        break;
                    case TOKEN_INTEGER:
                        stackAstNodePush(AstStack, new_leaf(AST_INT_LIT, token));
                        break;
                    case TOKEN_FLOAT:
                        stackAstNodePush(AstStack, new_leaf(AST_FLOAT_LIT, token));
                        break;
                    case TOKEN_IDENTIFIER:
                        stackAstNodePush(AstStack,new_leaf(AST_IDENTIFIER,token));
                        break;
                    case TOKEN_GLOBAL_ID:
                        stackAstNodePush(AstStack,new_leaf(AST_GLOBAL_ID,token));
                        break;
                    case TOKEN_TRUE:
                        stackAstNodePush(AstStack,new_leaf(AST_BOOL_LIT,token));
                        break;
                    case TOKEN_FALSE:
                        stackAstNodePush(AstStack,new_leaf(AST_BOOL_LIT,token));
                        break;
                    default:
                        // kľúčové slová, NEW_LINE, interpunkcia: zvyčajne do AST nejdú => nič nepushuj
                        break;
                    }
                }



                //printf("ok\n");
                tokenFree(&token);
                stackSymbolPop(Stack);
                token = lexer_token_next(); 
            }
            else
            {
                printError(SYNTAX_ERROR,"Nesprávne poradie tokenov\n");
                memorySafeExit(SYNTAX_ERROR);
            }
        }
        else if(!Stack_top.is_terminal && !Stack_top.is_reduce)
        {
            switch (Stack_top.symbol)
            {
                case N_Program:
                    Program_func(Stack);
                    break;
                case N_Prolog:
                    Prolog_func(Stack);
                    break;
                case N_Main_class:
                    Main_class_func(Stack);
                    break;
                case N_Main_class_block:
                    Main_class_block_func(Stack, token);
                    break;
                case N_Block:
                    Block_func(Stack);
                    break;
                case N_Block_content:
                    Block_content_func(Stack, token);
                    break;
                case N_smth:
                    smth_func(Stack);
                    break;
                case N_smth_tail:
                    smth_tail_func(Stack, token);
                    break;
                case N_smth_list:
                    smth_list_func(Stack, token);
                    break;
                case N_Oneline_expr_opt:
                    Oneline_expr_opt_func(Stack, token);
                    break;
                case N_Func_dec:
                    Func_dec_func(Stack);
                    break;
                case N_Func_dec_tail:
                    Func_dec_tail_func(Stack, token);
                    break;
                case N_Param:
                    Param_func(Stack, token);
                    break;
                case N_Param_list:
                    Param_list_func(Stack, token);
                    break;
                case N_Func_call:
                    Func_call_func(Stack, token);
                    break;
                case N_Var:
                    Var_func(Stack);
                    break;
                case N_Var_EXT:
                    Var_EXT_func(Stack, token);
                    break;
                case N_Var_arg:
                    Var_arg_func(Stack, token);
                    break;
                case N_Var_arg_list:
                    var_arg_list_func(Stack, token);
                    break;
                case N_Call_assign:
                    Call_assign_func(Stack);
                    break;
                case N_Call_assign_list:
                    Call_assign_list_func(Stack, token);
                    break;
                case N_Global_assign:
                    Global_assign_func(Stack);
                    break;
                case N_If:
                    If_func(Stack);
                    break;
                case N_While:
                    While_func(Stack);
                    break;
                case N_For:
                    For_func(Stack);
                    break;
                case N_Break:
                    Break_func(Stack);
                    break;
                case N_Continue:
                    Continue_func(Stack);
                    break;
                case N_Return:
                    Return_func(Stack, token);
                    break;
                case N_Return_expr:
                    Return_expr_func(Stack, token);
                    break;
                case N_Expr:
                    Expr_func(Stack, gLookahead, AstStack, 0);
                    token = *gLookahead;
                    printf("precedencna odovzdala token jeho type je = %d\n",token.type);
                    break;
                case N_cExpr:
                    Expr_func(Stack, gLookahead, AstStack, 1);
                    token = *gLookahead;
                    printf("precedencna odovzdala token jeho type je = %d\n",token.type);
                    break;
                case N_lExpr:
                    Expr_func(Stack, gLookahead, AstStack, -1);
                    printf("%d",1);
                    token = *gLookahead;
                    printf("precedencna odovzdala token jeho type je = %d\n",token.type);
                    break;
                case N_oExpr:
                    Expr_func(Stack, gLookahead, AstStack, -2);
                    token = *gLookahead;
                    printf("precedencna odovzdala token jeho type je = %d\n",token.type);
                    break;

                case N_New_line_check:
                    New_line_func(Stack,token);
                    break;
                case N_Opt_newlines:
                    Opt_newlines_func(Stack,token);
                    break;
                case N_Block_content_check:
                    Block_content_func_check(Stack,token);
                    break;
                case N_callIfj:
                    callIfjfunc(Stack,token);
                    break;
                case N_Else_chain:
                    Else_chain_func(Stack, token);
                    break;
                case N_Else_item:
                    Else_item_func(Stack, token);
                    break;
                case N_callIfjCheck:
                    callIfjCheckfunc(Stack,token);
                    break;
                case N_AssignIfjIdCheck:
                    AssignIfjIdCheckFunc(Stack,token);
                    break;
                case N_SMALL_NULL_CHECK:
                    smallNullCheckFunc(Stack,token);
                    break;
                case N_SMALL_NULL_CHECK_TYPE:
                    smallNullCheckTypeFunc(Stack,token);
                    break;
                default:
                        printError(INTERNAL_ERROR, "Nespravny Neterminal\n");
                        memorySafeExit(INTERNAL_ERROR);
                        break;
            }
        }

    }

    if (stackAstNodeIsEmpty(AstStack))
    {
        printError(SYNTAX_ERROR, "AST je prázdny\n");
        memorySafeExit(SYNTAX_ERROR);
    }
    
    
    AstNode *ast_root = stackAstNodeRead(AstStack);
    stackSymbolDestruct(Stack);
    stackAstNodeDestruct(AstStack);
    fprintf(stderr,"PARSER IS DONE\n");
    return ast_root;

}

void pushCheck(bool boolean)
{
    if(!boolean)
    {
        printError(INTERNAL_ERROR,"Chyba pushovania na Stack\n");
        memorySafeExit(INTERNAL_ERROR);
    }
}

void Program_func(DynamicSymbolStackPtr Stack)
{
    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_PROGRAM, 2)));
    pushCheck(stackSymbolPush(Stack,makeNonterm(N_Main_class)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_NEW_LINE)));
    pushCheck(stackSymbolPush(Stack,makeNonterm(N_Prolog)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
}

void Prolog_func(DynamicSymbolStackPtr Stack)
{

    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_PROLOG,0)));
    
    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_KEYWORD_IFJ)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_KEYWORD_FOR)));
    Token ifj25;
    ifj25.value.identifier = "ifj25";
    ifj25.type = TOKEN_STRING;
    pushCheck(stackSymbolPush(Stack,makeTermValue(ifj25)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_KEYWORD_IMPORT)));
}

void Main_class_func(DynamicSymbolStackPtr Stack)
{
    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_MAIN_CLASS, 2)));
    pushCheck(stackSymbolPush(Stack,makeNonterm(N_New_line_check)));
    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_BLOCK_END)));
    pushCheck(stackSymbolPush(Stack,makeNonterm(N_Main_class_block)));
    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_NEW_LINE)));
    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_BLOCK_STAR)));
    pushCheck(stackSymbolPush(Stack, makeTermValueAndCaptureIdentifier("Program")));
    pushCheck(stackSymbolPush(Stack,makeTerm(TOKEN_KEYWORD_CLASS)));
}

void Main_class_block_func(DynamicSymbolStackPtr Stack, Token token)
{
    //popneme  neterminál
    stackSymbolPop(Stack);

    // FIRST(Func_dec) = TOKEN_KEYWORD_STATIC
    // Epsilon vyberáme, keď vidíme '}' (koniec bloku) => TOKEN_BLOCK_END
    if (token.type == TOKEN_KEYWORD_STATIC) {
        // Vetva: Main_class_block -> Main_class_item Main_class_block
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_MAIN_CLASS_BLOCK, 2)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Main_class_block))); // ... Main_class_block
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Func_dec)));  // Main_class_item
        return;
    }
    else if (token.type == TOKEN_NEW_LINE)
    {
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Main_class_block)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }
    else if (token.type == TOKEN_BLOCK_END) {
        // Vetva: Main_class_block -> eps
        // vytvoríme prázdny uzol bloku.
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_MAIN_CLASS_BLOCK, 0)));
        return;
    }

    else
    {// Nič z FIRST ani FOLLOW: syntaktická chyba
        printError(SYNTAX_ERROR, "Očakávaný 'static' alebo '}' v Main_class_block");
        memorySafeExit(SYNTAX_ERROR);
    }


}

void Block_func(DynamicSymbolStackPtr Stack)
{
    stackSymbolPop(Stack);
    // Block -> { Block_content }
    // AST_BLOCK má 1 dieťa = STATEMENT_LIST
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_BLOCK, 1)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_BLOCK_END)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block_content_check)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_BLOCK_STAR)));
}

void Block_content_func_check(DynamicSymbolStackPtr Stack, Token token)
{ 
    stackSymbolPop(Stack);

    if (token.type == TOKEN_BLOCK_END) {
        // {} -> prázdny zoznam
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_NULL, 0)));
        return;
    }

    if (token.type == TOKEN_NEW_LINE) {
        // { NEW_LINE Block_content }
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block_content)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }

    // One-liner VO FUNKCII: presne jeden VÝRAZ (napr. { 5+5 })
    switch (token.type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            pushCheck(stackSymbolPush(Stack, makeReduce(AST_STATEMENT_LIST, 1)));
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_oExpr)));
            return;
        default:
            printError(SYNTAX_ERROR, "Po '{' vo funkcii očakávam NEW_LINE, '}', alebo one-liner výraz");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void Block_content_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    // Preskoč leading NEW_LINE* vo vnútri bloku
    if (token.type == TOKEN_NEW_LINE) {
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block_content)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }

    // Koniec bloku → prázdny zoznam
    if (token.type == TOKEN_BLOCK_END) {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_STATEMENT_LIST, 0)));
        return;
    }
    // Inak: 1 položka + chvost → pravorekurzívny plochý zoznam
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_STATEMENT_LIST, 2)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_smth_tail)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_smth_list)));
}
/* === smth -> smth_list smth_tail =======================================
   Keďže obidve vetvy môžu mať ε, zvolíme rodiča s aritou 2,
   a v ε vetvách budeme produkovať prázdne uzly (0), aby počet sedel.
*/
void smth_func(DynamicSymbolStackPtr Stack) {
    // smth -> smth_list smth_tail
    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_smth_tail)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_smth_list)));
}

void smth_tail_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    // Po jednej položke môžu nasledovať NEW_LINE a ďalší obsah,
    // alebo epsilon (keď hneď príde '}' alebo niečo, čo už nespadá do FIRST smth_list).
    if (token.type == TOKEN_NEW_LINE) {
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block_content))); // znovu začni obsah (buď ďalšia položka, alebo })
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }
    if (token.type == TOKEN_BLOCK_END) {
        // tu nesmie byť ε – chceme aspoň jeden NEW_LINE pred '}'
        printError(SYNTAX_ERROR, "Pred '}' musí byť NEW_LINE\n");
        memorySafeExit(SYNTAX_ERROR);
    }
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
}

void smth_list_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    switch (token.type) {
        case TOKEN_KEYWORD_RETURN:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Return)));        
            return;
        case TOKEN_KEYWORD_VAR:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var)));           
            return;
        case TOKEN_KEYWORD_IF:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_If)));         
            return;
        case TOKEN_KEYWORD_WHILE:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_While)));         
            return;
        case TOKEN_KEYWORD_FOR:   
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_For)));           
            return;
        case TOKEN_KEYWORD_BREAK:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Break)));         
            return;
        case TOKEN_KEYWORD_CONTINUE:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Continue)));      
            return;
        case TOKEN_IDENTIFIER:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Call_assign)));   
            return;
            
        case TOKEN_KEYWORD_IFJ:
            pushCheck(stackSymbolPush(Stack, makeReduce(AST_IFJ_CALL, 2)));
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_callIfj)));
            pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_IFJ)));
            return;
        case TOKEN_GLOBAL_ID: 
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Global_assign))); return;
        case TOKEN_BLOCK_STAR:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block))); 
            return;

        // ak nechceš holé výrazové statementy, tieto nechaj vypnuté:
        // case TOKEN_INTEGER:
        // case TOKEN_FLOAT:
        // case TOKEN_STRING:
        // case TOKEN_LPAREN:
        //     pushCheck(stackSymbolPush(Stack, makeNonterm(N_Expr)));
        //     return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token na začiatku príkazu\n");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void Oneline_expr_opt_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    switch (token.type)
    {
        // FIRST(Expr): všetky tokeny, ktorými môže začínať výraz
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_oExpr)));
            return;

        // Vetva: Oneline_expr_opt -> eps
        case TOKEN_NEW_LINE:
        case TOKEN_BLOCK_END:
        case TOKEN_RPAREN:
            return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token v Oneline_expr_opt\n");
            memorySafeExit(SYNTAX_ERROR);
    }

}

void Func_dec_func(DynamicSymbolStackPtr Stack)
{
    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Func_dec_tail)));
    pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_STATIC)));
}

void Func_dec_tail_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);

    if (token.type == TOKEN_LPAREN)
    {
        // Vetva: ( Param ) Block
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_FUNC_DEC, 3)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Param)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
    }
    else if (token.type == TOKEN_ASSIGN)
    {
        // Vetva: = ( identifier ) Block (getter/setter)
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_SETTER, 3)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 2)));
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
        pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_ASSIGN)));
    }
    else if (token.type == TOKEN_BLOCK_STAR)
    {
        // Vetva: Block
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_GETTER, 3)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block)));
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 1)));
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávané '(' alebo '=' vo Func_dec_tail\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}
void Param_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);

    if (token.type == TOKEN_IDENTIFIER)
    {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 2)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Param_list)));
        pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
    }
    else if (token.type == TOKEN_RPAREN)
    {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 1))); // eps
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
        
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávaný identifikátor alebo ')' v Param\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void Param_list_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    if (token.type == TOKEN_COMMA)
    {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 2))); 
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Param_list)));
        pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_COMMA)));
        
    }
    else if (token.type == TOKEN_RPAREN)
    {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávaný ',' alebo ')' v Param_list");
        memorySafeExit(SYNTAX_ERROR);
    }
}
void Func_call_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);

    if (token.type == TOKEN_IDENTIFIER) {
        // IDENT ( Var_arg )
        // redukujeme na AST_FUNC_CALL s 1 dieťaťom (Var_arg)
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_FUNC_CALL, 2)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var_arg)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
        pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
        return;
    }
    else if (token.type == TOKEN_KEYWORD_IFJ) {
        // KEYWORD_IFJ . IDENT ( Var_arg )
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_FUNC_CALL, 2)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var_arg)));
        pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_DOT)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_IFJ)));
        return;
    }

    printError(SYNTAX_ERROR, "Očakávaný identifikátor alebo 'ifj' na začiatku Func_call");
    memorySafeExit(SYNTAX_ERROR);
}

void Var_func(DynamicSymbolStackPtr Stack)
{
    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_VAR, 2)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var_EXT)));
    pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_VAR)));
}

void Var_EXT_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    if (token.type == TOKEN_ASSIGN)
    {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_ASSIGN, 1)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_AssignIfjIdCheck)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_ASSIGN)));
    }

    // epsilon vetva (koniec deklarácie bez priradenia)
    else if (token.type == TOKEN_NEW_LINE || TOKEN_BLOCK_END)
    {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
    }
    else
    {
        printError(SYNTAX_ERROR,"chyba za var\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void Var_arg_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    switch (token.type) {
        // FIRST(Expr)
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
            pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 2)));
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var_arg_list)));
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Expr)));
            return;
        case TOKEN_LPAREN:
            // Expr var_arg_list  => uložíme ako AST_PARAM (zoznam argumentov) => arity = 2
            pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 2)));
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var_arg_list)));
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_lExpr)));
            return;

        // eps (žiadne argumenty)
        case TOKEN_RPAREN:
            pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 1)));
            pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
            return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token vo Var_arg\n");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void var_arg_list_func(DynamicSymbolStackPtr Stack, Token token)
{
     stackSymbolPop(Stack);
     if (token.type == TOKEN_COMMA) {
        // , Expr var_arg_list  => Expr (1) + tail (1) => arity = 2
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_PARAM, 2)));
         pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var_arg_list)));
         pushCheck(stackSymbolPush(Stack, makeNonterm(N_cExpr)));
         pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
         pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_COMMA)));
    } else if (token.type == TOKEN_RPAREN) {
         // eps
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
    } else {
         printError(SYNTAX_ERROR, "Neočakávaný token vo var_arg_list\n");
         memorySafeExit(SYNTAX_ERROR);
    }
}

void Call_assign_func(DynamicSymbolStackPtr Stack)
{
    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Call_assign_list)));
    pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));

}

void Call_assign_list_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    if (token.type == TOKEN_ASSIGN) {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_ASSIGN, 2))); // IDENT, EXPR
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_AssignIfjIdCheck)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_ASSIGN)));
    } else if (token.type == TOKEN_LPAREN) {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_FUNC_CALL, 2))); // IDENT, PARAM(args)
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var_arg)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
    }
    else {
        printError(SYNTAX_ERROR, "Očakávané '=' alebo '(' v Call_assign_list");
        memorySafeExit(SYNTAX_ERROR);
    }
}void Global_assign_func(DynamicSymbolStackPtr Stack)
{
    // Global_assign -> GLOBAL_ID = Expr
    stackSymbolPop(Stack);
    // AST_ASSIGN(Expr) => arity = 1
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_GLOBAL_ID, 2)));
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_ASSIGN,1)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_AssignIfjIdCheck)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_ASSIGN)));
    pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_GLOBAL_ID)));
}

void If_func(DynamicSymbolStackPtr Stack)
{
    // if ( lExpr ) NEW_LINE Block Else_chain
    stackSymbolPop(Stack);

    // Finálna redukcia: IF_CHAIN(2) = [ BRANCH(cond,block) , Else_chain ]
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_IF_CHAIN, 2)));

    // 2) zvyšok (zoznam ďalších vetiev)
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Else_chain)));

    // 1) prvá vetva ako BRANCH(cond, block)
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_IF, 2)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block)));
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_CONDITION, 1)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_lExpr)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_IF)));
}

void Else_chain_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);

    if (token.type == TOKEN_KEYWORD_ELSE) {
        // Else_chain -> Else_item Else_chain
        // Skladáme dvojice do IF_CHAIN(2); builder to sploští
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_IF_CHAIN, 2)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Else_chain))); // tail
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Else_item)));  // head
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_ELSE)));
        return;
    }
    // ε
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
}

void Else_item_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);

    if (token.type == TOKEN_KEYWORD_IF) {
        // else if ( lExpr ) NEW_LINE Block   => premeníme na BRANCH(cond, block)
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_IF, 2)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block)));
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_CONDITION, 1)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_lExpr)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_IF)));
        return;
    }
    else if (token.type == TOKEN_BLOCK_STAR) {
        // else { ... }  => necháme AST_ELSE(1)
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_ELSE, 1)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block)));
        return;
    }

    printError(SYNTAX_ERROR, "Po 'else' očakávam 'if' alebo '{'");
    memorySafeExit(SYNTAX_ERROR);
}

void While_func(DynamicSymbolStackPtr Stack)
{
    // While -> while ( Expr ) Block
    stackSymbolPop(Stack);
    // deti: Expr, Block => arity = 2
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_WHILE, 2)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block)));
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_CONDITION, 1)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_lExpr)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_WHILE)));
}

void For_func(DynamicSymbolStackPtr Stack)
{
    // For -> for ( IDENT in Expr ) Block
    stackSymbolPop(Stack);
    // deti: Expr, Block => arity = 2  (IDENT v AST neukladáme)
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_FOR, 2)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Block)));
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_CONDITION, 2)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_lExpr)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_IN)));
    pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_FOR)));
}

void Break_func(DynamicSymbolStackPtr Stack)
{
    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_BREAK, 0)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_BREAK)));
}

void Continue_func(DynamicSymbolStackPtr Stack)
{
    stackSymbolPop(Stack);
    // jednoduchý statement (bez detí)
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_CONTINUE, 0)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_CONTINUE)));
}

void Return_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_RETURN, 1)));
    pushCheck(stackSymbolPush(Stack, makeNonterm(N_Return_expr)));
    pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_RETURN)));
}

void Return_expr_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    switch (token.type)
    {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_Expr)));
            break;
        case TOKEN_LPAREN:
            pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
            pushCheck(stackSymbolPush(Stack, makeNonterm(N_lExpr)));
            pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
            break;

        case TOKEN_NEW_LINE:
            pushCheck(stackSymbolPush(Stack, makeReduce(AST_EMPTY, 0)));
            break; // ε – žiadny uzol, aritu už rozhodol Return_func
        case TOKEN_RPAREN:
        default:
            printError(SYNTAX_ERROR, "Neočakávaný token v Return_expr\n");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void Expr_func(DynamicSymbolStackPtr Stack, Token *gLookahead, DynamicAstNodeStackPtr AstStack,int flag)
{
    // odstráň neterminál
    stackSymbolPop(Stack);

    if (!gLookahead || ! AstStack) {
        printError(INTERNAL_ERROR, "Expr_func: chýbajú hooky\n");
        memorySafeExit(INTERNAL_ERROR);
    }

    AstNode *expr = parseExpression(gLookahead,flag);
    if (!expr)
    {
        printError(SYNTAX_ERROR, "Chybný výraz\n");
        memorySafeExit(SYNTAX_ERROR);
    }
    fprintf(stderr,"expr strom po skoncení funcke parseExpression\n");
    ast_dump(expr);

    // výsledný uzol výrazu dáme na AST zásobník
    pushCheck(stackAstNodePush(AstStack, expr));
    
}

void  New_line_func(DynamicSymbolStackPtr Stack, Token token)
{

    stackSymbolPop(Stack);
    if (token.type == TOKEN_NEW_LINE)
    {
        pushCheck(stackSymbolPush(Stack,makeNonterm(N_New_line_check)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }
    else if (token.type == TOKEN_EOF)
    {
        return;
    }
    else
    {
        printError(SYNTAX_ERROR, "za koncom bloku class program chybny token\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void Opt_newlines_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    if (token.type == TOKEN_NEW_LINE)
    {
        // Reprezentuje "NEW_LINE Opt_newlines"
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_NEW_LINE)));
        return;
    }
    // ε (nič - keď už nepríde NEW_LINE)
}

void callIfjfunc(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);

    if (token.type == TOKEN_DOT){
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_RPAREN)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Var_arg)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_LPAREN)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_callIfjCheck)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Opt_newlines)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_DOT)));
    }
    else
    {
        printError(SYNTAX_ERROR,"nespravne volanie vstavanej funkcie\n");
        memorySafeExit(SYNTAX_ERROR);
    }


}

void callIfjCheckfunc(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);

    if (token.type != TOKEN_IDENTIFIER || token.value.identifier == NULL) 
    {
        printError(SYNTAX_ERROR, "Ocakavany nazov funkcie po 'ifj.'");
        memorySafeExit(SYNTAX_ERROR);
    }
    char *words[] = {"read_str","read_num","write", "floor","str","length","substring","strcmp","ord","chr","read_bool"};
    size_t count = sizeof(words) / sizeof(words[0]);
    bool allowed = false;
    for (size_t i = 0; i < count; i++)
    {

        if (strcmp(token.value.identifier, words[i]) == 0)
        {
            allowed = true;
            break;
        }
    }
    if (allowed)
    {
        pushCheck(stackSymbolPush(Stack, makeTermWithValue(TOKEN_IDENTIFIER)));
        return;
    }
    else
    {
        printError(SYNTAX_ERROR,"nespravne volanie vstavanej funkcie\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void AssignIfjIdCheckFunc(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);

    if (token.type == TOKEN_KEYWORD_IFJ)
    {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_IFJ_CALL, 2)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_callIfj)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_IFJ)));
        return;
    }
    
    else if (token.type == TOKEN_KEYWORD_NULL_SMALL)
    {
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_SMALL_NULL_CHECK)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_NULL_SMALL)));
    }
    else
    {
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_Expr)));
    }
}

void smallNullCheckFunc(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    if (token.type == TOKEN_KEYWORD_IS)
    {
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_IS, 2)));
        pushCheck(stackSymbolPush(Stack, makeNonterm(N_SMALL_NULL_CHECK_TYPE)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_IS)));
    }
    pushCheck(stackSymbolPush(Stack, makeReduce(AST_SMALL_NULL,0)));
}
void smallNullCheckTypeFunc(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);
    switch (token.type)
    {
    case TOKEN_KEYWORD_NULL:
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_NULL, 0)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_NULL)));
        break;
    case TOKEN_KEYWORD_STRING:
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_KEYWORD_STRING, 0)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_STRING)));
        break;
    case TOKEN_KEYWORD_NUM:
        pushCheck(stackSymbolPush(Stack, makeReduce(AST_KEYWORD_NUM, 0)));
        pushCheck(stackSymbolPush(Stack, makeTerm(TOKEN_KEYWORD_NUM)));
        break;
    
    default:
        printError(SYNTAX_ERROR,"po is musi nasledovat datovy typ");
        memorySafeExit(SYNTAX_ERROR);
        break;
    }
}


Symbol makeReduce(AstNodeType kind, size_t arity)
{
    Symbol x = {0};
    x.is_reduce = true;
    x.reduce_kind = kind;
    x.reduce_arity = arity;
    return x;
}

Symbol makeNonterm(Nonterm N)
{
    Symbol x = {0};
    x.symbol = N;
    return x;
}

Symbol makeTermValue(Token token)
{
    Symbol x = {0};
    x.is_terminal = true;
    x.freeNeeded =  true;
    x.symbol = token.type;
    x.value_needed = true;
    x.value.identifier = token.value.identifier;
    return x;
}

Symbol makeTerm(TokenType tokenType)
{
    Symbol x = {0};
    x.is_terminal = true;
    x.symbol = tokenType;
    return x;
}


Symbol makeTermWithValue(TokenType type)
{
    Symbol x = {0};
    x.freeNeeded =  true;
    x.is_terminal = true;
    x.symbol = type;
    x.capture_value = true;
    return x;

}

Symbol makeTermValueAndCaptureIdentifier(const char *exact) {
    Symbol x = {0};
    x.is_terminal = true;
    x.symbol = TOKEN_IDENTIFIER;
    x.freeNeeded = true;
    x.value_needed = true;
    x.capture_value = true;
    x.value.identifier = (char*)exact;
    return x;
}

void tokenFree(Token *token)
{
    if (!token) 
    {
        return;
    }

    switch (token->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_STRING:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
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
