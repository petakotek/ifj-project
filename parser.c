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


AstNode *parser()
{
    DynamicAstNodeStackPtr AstStack = stackAstNodeConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    DynamicSymbolStackPtr  Stack    = stackSymbolConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    if (Stack == NULL || AstStack == NULL)
    {
        printError(INTERNAL_ERROR,"Chyba alokácie");
        memorySafeExit(INTERNAL_ERROR);
    }

    push_check(stackSymbolPush(Stack,make_term(TOKEN_EOF)));
    push_check(stackSymbolPush(Stack,make_nonterm(N_Program)));
    Token token = lexer_token_next();
    Symbol Stack_top;

    Token *gLookahead = NULL;
    DynamicAstNodeStackPtr gAstStack = NULL;
    gLookahead = &token;
    gAstStack  = AstStack;




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


            size_t ast_count = 0;
            {
                // POZOR: toto je len rýchly odhad – ak máš API na zistenie veľkosti stacku, použi ho.
                // Inak si do tvojej implementácie zásobníka doplň počítadlo prvkov (najlepšie).
                DynamicAstNodeStackPtr tmp = AstStack;
                // ak nemáš priamy prístup, aspoň logni typ redukcie
            }
            fprintf(stderr, "[REDUCE] %s / arity=%zu  (predpoklad detí=%zu)\n", ast_node_type_to_string(Stack_top.reduce_kind), n, n);
            // na vrchu sú pravé deti; preto vkladáme odzadu
            for (size_t i = 0; i < n; ++i)
            {
                if (stackAstNodeIsEmpty(AstStack))
                {
                    fprintf(stderr, "[REDUCE-FAIL] %s chce %zu. Na AST zásobníku došli deti pri i=%zu.\n", ast_node_type_to_string(Stack_top.reduce_kind), n, i);
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

            printf("%d\n",token.type);
            printf("%d\n",Stack_top.symbol);
            if (Stack_top.symbol == token.type)
            {
                bool ok = true;
                if (Stack_top.value_needed)
                {
                    switch (token.type)
                    {
                        case TOKEN_STRING:
                            ok = (token.value.string && Stack_top.value.string && strcmp(Stack_top.value.string, token.value.string) == 0);
                            break;
                        case TOKEN_INTEGER:
                            ok = (Stack_top.value.number == token.value.number);
                            break;
                        case TOKEN_FLOAT:
                            ok = (Stack_top.value.number == token.value.number);
                            break;
                        case TOKEN_IDENTIFIER:
                            ok = (token.value.identifier && Stack_top.value.identifier && strcmp(Stack_top.value.identifier, token.value.identifier) == 0);
                            break;

                        default:
                            ok = true;
                    }
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
                    default:
                        // kľúčové slová, NEW_LINE, interpunkcia: zvyčajne do AST nejdú => nič nepushuj
                        break;
                    }
                }



                printf("ok\n");
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
                    Program_func(Stack, token);
                    break;
                case N_Prolog:
                    Prolog_func(Stack, token);
                    break;
                case N_Main_class:
                    Main_class_func(Stack, token);
                    break;
                case N_Main_class_block:
                    Main_class_block_func(Stack, token);
                    break;
                case N_Block:
                    Block_func(Stack, token);
                    break;
                case N_Block_content:
                    Block_content_func(Stack, token);
                    break;
                case N_smth:
                    smth_func(Stack, token);
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
                    Func_dec_func(Stack, token);
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
                    Var_func(Stack, token);
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
                    Call_assign_func(Stack, token);
                    break;
                case N_Call_assign_list:
                    Call_assign_list_func(Stack, token);
                    break;
                case N_Global_assign:
                    Global_assign_func(Stack, token);
                    break;
                case N_If:
                    If_func(Stack, token);
                    break;
                case N_Else:
                    Else_func(Stack, token);
                    break;
                case N_While:
                    While_func(Stack, token);
                    break;
                case N_For:
                    For_func(Stack, token);
                    break;
                case N_Break:
                    Break_func(Stack, token);
                    break;
                case N_Continue:
                    Continue_func(Stack, token);
                    break;
                case N_Return:
                    Return_func(Stack, token);
                    break;
                case N_Return_expr:
                    Return_expr_func(Stack, token);
                    break;
                case N_Expr:
                    Expr_func(Stack, token, gLookahead, gAstStack);
                    break;
                case N_Else_body:
                    ElseBody_func(Stack, token);
                    break;

                case N_New_line_check:
                    New_line_func(Stack,token);
                    break;
                case N_Opt_newlines:
                    Opt_newlines_func(Stack,token);
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

    return ast_root;

}

void push_check(bool boolean)
{
    if(!boolean)
    {
        printError(INTERNAL_ERROR,"Chyba pushovania na Stack\n");
        memorySafeExit(INTERNAL_ERROR);
    }
}

void Program_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    push_check(stackSymbolPush(Stack, make_reduce(AST_PROGRAM, 2)));
    push_check(stackSymbolPush(Stack,make_nonterm(N_Main_class)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Opt_newlines)));
    push_check(stackSymbolPush(Stack,make_term(TOKEN_NEW_LINE)));
    push_check(stackSymbolPush(Stack,make_nonterm(N_Prolog)));
}

void Prolog_func(DynamicSymbolStackPtr Stack, Token token)
{

    stackSymbolPop(Stack);;
    push_check(stackSymbolPush(Stack, make_reduce(AST_PROLOG,0)));
    
    push_check(stackSymbolPush(Stack,make_term(TOKEN_KEYWORD_IFJ)));
    push_check(stackSymbolPush(Stack,make_term(TOKEN_KEYWORD_FOR)));
    Token ifj25;
    ifj25.type = TOKEN_STRING;
    ifj25.value.string = "ifj25";
    push_check(stackSymbolPush(Stack,make_term_value(ifj25)));
    push_check(stackSymbolPush(Stack,make_term(TOKEN_KEYWORD_IMPORT)));
}

void Main_class_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    push_check(stackSymbolPush(Stack, make_reduce(AST_MAIN_CLASS, 2)));
    push_check(stackSymbolPush(Stack,make_nonterm(N_New_line_check)));
    push_check(stackSymbolPush(Stack,make_term(TOKEN_BLOCK_END)));

    push_check(stackSymbolPush(Stack,make_nonterm(N_Main_class_block)));
    push_check(stackSymbolPush(Stack,make_term(TOKEN_NEW_LINE)));
    push_check(stackSymbolPush(Stack,make_term(TOKEN_BLOCK_STAR)));
    push_check(stackSymbolPush(Stack, make_term_value_and_capture_identifier("Program")));
    push_check(stackSymbolPush(Stack,make_term(TOKEN_KEYWORD_CLASS)));
}

void Main_class_block_func(DynamicSymbolStackPtr Stack, Token token)
{
    //popneme  neterminál
    stackSymbolPop(Stack);;

    // FIRST(Func_dec) = TOKEN_KEYWORD_STATIC
    // Epsilon vyberáme, keď vidíme '}' (koniec bloku) => TOKEN_BLOCK_END
    if (token.type == TOKEN_KEYWORD_STATIC) {
        // Vetva: Main_class_block -> Main_class_item Main_class_block
        push_check(stackSymbolPush(Stack, make_reduce(AST_MAIN_CLASS_BLOCK, 2)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Main_class_block))); // ... Main_class_block
        push_check(stackSymbolPush(Stack, make_nonterm(N_Func_dec)));  // Main_class_item
        return;
    }
    else if (token.type == TOKEN_NEW_LINE)
    {
        push_check(stackSymbolPush(Stack, make_nonterm(N_Main_class_block)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_NEW_LINE)));
        return;
    }
    else if (token.type == TOKEN_BLOCK_END) {
        // Vetva: Main_class_block -> eps
        // vytvoríme prázdny uzol bloku.
        push_check(stackSymbolPush(Stack, make_reduce(AST_MAIN_CLASS_BLOCK, 0)));
        return;
    }

    else
    {// Nič z FIRST ani FOLLOW: syntaktická chyba
        printError(SYNTAX_ERROR, "Očakávaný 'static' alebo '}' v Main_class_block");
        memorySafeExit(SYNTAX_ERROR);
    }


}

void Block_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    // Block -> { Block_content }
    // AST_BLOCK má 1 dieťa = STATEMENT_LIST
    push_check(stackSymbolPush(Stack, make_reduce(AST_BLOCK, 1)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_NEW_LINE)));   // ak chceš vynútiť NL pred '}', nechaj
    push_check(stackSymbolPush(Stack, make_term(TOKEN_BLOCK_END)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Block_content)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_BLOCK_STAR)));
}

void Block_content_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;

    // Preskoč leading NEW_LINE* vo vnútri bloku
    if (token.type == TOKEN_NEW_LINE) {
        push_check(stackSymbolPush(Stack, make_nonterm(N_Block_content)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_NEW_LINE)));
        return;
    }

    // Koniec bloku → prázdny zoznam
    if (token.type == TOKEN_BLOCK_END) {
        push_check(stackSymbolPush(Stack, make_reduce(AST_STATEMENT_LIST, 0)));
        return;
    }

    // Inak: 1 položka + chvost → pravorekurzívny plochý zoznam
    push_check(stackSymbolPush(Stack, make_reduce(AST_STATEMENT_LIST, 2)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_smth_tail)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_smth_list)));
}
/* === smth -> smth_list smth_tail =======================================
   Keďže obidve vetvy môžu mať ε, zvolíme rodiča s aritou 2,
   a v ε vetvách budeme produkovať prázdne uzly (0), aby počet sedel.
*/
void smth_func(DynamicSymbolStackPtr Stack, Token token) {
    // smth -> smth_list smth_tail
    stackSymbolPop(Stack);;
    push_check(stackSymbolPush(Stack, make_nonterm(N_smth_tail)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_smth_list)));
}

void smth_tail_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    // Po jednej položke môžu nasledovať NEW_LINE a ďalší obsah,
    // alebo epsilon (keď hneď príde '}' alebo niečo, čo už nespadá do FIRST smth_list).
    if (token.type == TOKEN_NEW_LINE) {
        push_check(stackSymbolPush(Stack, make_nonterm(N_Block_content))); // znovu začni obsah (buď ďalšia položka, alebo })
        push_check(stackSymbolPush(Stack, make_term(TOKEN_NEW_LINE)));
    }
    // inak ε
}

void smth_list_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    // ŽIADNE AST_STATEMENT redukcie – vkladáme priamo "skutočný" uzol
    switch (token.type) {
        case TOKEN_KEYWORD_RETURN:   push_check(stackSymbolPush(Stack, make_nonterm(N_Return)));        return;
        case TOKEN_KEYWORD_VAR:      push_check(stackSymbolPush(Stack, make_nonterm(N_Var)));           return;
        case TOKEN_KEYWORD_IF:       push_check(stackSymbolPush(Stack, make_nonterm(N_If)));            return;
        case TOKEN_KEYWORD_WHILE:    push_check(stackSymbolPush(Stack, make_nonterm(N_While)));         return;
        case TOKEN_KEYWORD_FOR:      push_check(stackSymbolPush(Stack, make_nonterm(N_For)));           return;
        case TOKEN_KEYWORD_BREAK:    push_check(stackSymbolPush(Stack, make_nonterm(N_Break)));         return;
        case TOKEN_KEYWORD_CONTINUE: push_check(stackSymbolPush(Stack, make_nonterm(N_Continue)));      return;
        case TOKEN_IDENTIFIER:       push_check(stackSymbolPush(Stack, make_nonterm(N_Call_assign)));   return;
        case TOKEN_GLOBAL_ID:        push_check(stackSymbolPush(Stack, make_nonterm(N_Global_assign))); return;

        // ak nechceš holé výrazové statementy, tieto nechaj vypnuté:
        // case TOKEN_INTEGER:
        // case TOKEN_FLOAT:
        // case TOKEN_STRING:
        // case TOKEN_LPAREN:
        //     push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
        //     return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token na začiatku príkazu");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void Oneline_expr_opt_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    switch (token.type)
    {
        // FIRST(Expr): všetky tokeny, ktorými môže začínať výraz
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
            push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
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

void Func_dec_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    push_check(stackSymbolPush(Stack, make_nonterm(N_Func_dec_tail)));
    push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_STATIC)));
}

void Func_dec_tail_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;

    if (token.type == TOKEN_LPAREN)
    {
        // Vetva: ( Param ) Block
        push_check(stackSymbolPush(Stack, make_reduce(AST_FUNC_DEC, 3)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Block)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Param)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_LPAREN)));
    }
    else if (token.type == TOKEN_ASSIGN)
    {
        // Vetva: = ( identifier ) Block (getter/setter)
        push_check(stackSymbolPush(Stack, make_reduce(AST_FUNC_DEC, 3)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Block)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_LPAREN)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_ASSIGN)));
    }
    else if (token.type == TOKEN_BLOCK_STAR)
    {
        // Vetva: Block
        push_check(stackSymbolPush(Stack, make_reduce(AST_FUNC_DEC, 3)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Block)));
        push_check(stackSymbolPush(Stack, make_reduce(AST_PARAM, 0)));
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávané '(' alebo '=' vo Func_dec_tail\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}
void Param_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;

    if (token.type == TOKEN_IDENTIFIER)
    {
        push_check(stackSymbolPush(Stack, make_reduce(AST_PARAM, 2)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Param_list)));
        push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
    }
    else if (token.type == TOKEN_RPAREN)
    {
        push_check(stackSymbolPush(Stack, make_reduce(AST_PARAM, 0))); // eps
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávaný identifikátor alebo ')' v Param\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void Param_list_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    if (token.type == TOKEN_COMMA)
    {
        push_check(stackSymbolPush(Stack, make_reduce(AST_PARAM, 2))); 
        push_check(stackSymbolPush(Stack, make_nonterm(N_Param_list)));
        push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_COMMA)));
    }
    else if (token.type == TOKEN_RPAREN)
    {
        push_check(stackSymbolPush(Stack, make_reduce(AST_EMPTY, 0)));
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávaný ',' alebo ')' v Param_list");
        memorySafeExit(SYNTAX_ERROR);
    }
}
void Func_call_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;

    if (token.type == TOKEN_IDENTIFIER) {
        // IDENT ( Var_arg )
        // redukujeme na AST_FUNC_CALL s 1 dieťaťom (Var_arg)
        push_check(stackSymbolPush(Stack, make_reduce(AST_FUNC_CALL, 2)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Var_arg)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_LPAREN)));
        push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
        return;
    }
    else if (token.type == TOKEN_KEYWORD_IFJ) {
        // KEYWORD_IFJ . IDENT ( Var_arg )
        push_check(stackSymbolPush(Stack, make_reduce(AST_FUNC_CALL, 2)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Var_arg)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_LPAREN)));
        push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_DOT)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_IFJ)));
        return;
    }

    printError(SYNTAX_ERROR, "Očakávaný identifikátor alebo 'ifj' na začiatku Func_call");
    memorySafeExit(SYNTAX_ERROR);
}

void Var_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    push_check(stackSymbolPush(Stack, make_reduce(AST_VAR, 2)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Var_EXT)));
    push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_VAR)));
}

void Var_EXT_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    if (token.type == TOKEN_ASSIGN)
    {
        push_check(stackSymbolPush(Stack, make_reduce(AST_ASSIGN, 1)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_ASSIGN)));
    }

    // epsilon vetva (koniec deklarácie bez priradenia)
    else if (token.type == TOKEN_NEW_LINE || token.type == TOKEN_BLOCK_END || token.type == TOKEN_RPAREN)
    {
        push_check(stackSymbolPush(Stack, make_reduce(AST_EMPTY, 0)));
    }
    else
    {
        printError(SYNTAX_ERROR,"chyba za var");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void Var_arg_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    switch (token.type) {
        // FIRST(Expr)
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
            // Expr var_arg_list  => uložíme ako AST_PARAM (zoznam argumentov) => arity = 2
            push_check(stackSymbolPush(Stack, make_reduce(AST_PARAM, 2)));
            push_check(stackSymbolPush(Stack, make_nonterm(N_Var_arg_list)));
            push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
            return;

        // eps (žiadne argumenty)
        case TOKEN_RPAREN:
            push_check(stackSymbolPush(Stack, make_reduce(AST_PARAM, 0)));
            return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token vo Var_arg");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void var_arg_list_func(DynamicSymbolStackPtr Stack, Token token)
{
     stackSymbolPop(Stack);;
     if (token.type == TOKEN_COMMA) {
        // , Expr var_arg_list  => Expr (1) + tail (1) => arity = 2
        push_check(stackSymbolPush(Stack, make_reduce(AST_PARAM, 2)));
         push_check(stackSymbolPush(Stack, make_nonterm(N_Var_arg_list)));
         push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
         push_check(stackSymbolPush(Stack, make_term(TOKEN_COMMA)));
    } else if (token.type == TOKEN_RPAREN) {
         // eps
        push_check(stackSymbolPush(Stack, make_reduce(AST_EMPTY, 0)));
    } else {
         printError(SYNTAX_ERROR, "Neočakávaný token vo var_arg_list");
         memorySafeExit(SYNTAX_ERROR);
    }
}

void Call_assign_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    push_check(stackSymbolPush(Stack, make_nonterm(N_Call_assign_list)));
    push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
}

void Call_assign_list_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    if (token.type == TOKEN_ASSIGN) {
        push_check(stackSymbolPush(Stack, make_reduce(AST_ASSIGN, 2))); // IDENT, EXPR
        push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_ASSIGN)));
    } else if (token.type == TOKEN_LPAREN) {
        push_check(stackSymbolPush(Stack, make_reduce(AST_FUNC_CALL, 2))); // IDENT, PARAM(args)
        push_check(stackSymbolPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackSymbolPush(Stack, make_nonterm(N_Var_arg)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_LPAREN)));
    } else {
        printError(SYNTAX_ERROR, "Očakávané '=' alebo '(' v Call_assign_list");
        memorySafeExit(SYNTAX_ERROR);
    }
}void Global_assign_func(DynamicSymbolStackPtr Stack, Token token)
{
    // Global_assign -> GLOBAL_ID = Expr
    stackSymbolPop(Stack);;
    // AST_ASSIGN(Expr) => arity = 1
    push_check(stackSymbolPush(Stack, make_reduce(AST_ASSIGN, 2)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_ASSIGN)));
    push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_GLOBAL_ID)));
}

void If_func(DynamicSymbolStackPtr Stack, Token token)
{
    // If -> if ( Expr ) Block Else
    stackSymbolPop(Stack);;
    // deti: Expr, Block, Else => arity = 3
    push_check(stackSymbolPush(Stack, make_reduce(AST_IF, 3)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Else)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Block)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_RPAREN)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_LPAREN)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_IF)));
}

void Else_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;

    if (token.type == TOKEN_KEYWORD_ELSE) {
        // else Block | else If  => 1 dieťa (Block alebo If)
        push_check(stackSymbolPush(Stack, make_reduce(AST_IF, 1)));
        // Rozhodnutie necháme na ďalší krok (podľa lookaheadu po spárovaní ELSE)
        push_check(stackSymbolPush(Stack, make_nonterm(N_Else_body)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_ELSE)));
    } else {
        // eps
        push_check(stackSymbolPush(Stack, make_reduce(AST_IF, 0)));
    }
}

void ElseBody_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    if (token.type == TOKEN_KEYWORD_IF) {
        // ElseBody -> If
        push_check(stackSymbolPush(Stack, make_nonterm(N_If)));
    } else if (token.type == TOKEN_BLOCK_STAR) {
        // ElseBody -> Block
        push_check(stackSymbolPush(Stack, make_nonterm(N_Block)));
    } else {
        printError(SYNTAX_ERROR, "Po 'else' očakávam 'if' alebo '{'\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void While_func(DynamicSymbolStackPtr Stack, Token token)
{
    // While -> while ( Expr ) Block
    stackSymbolPop(Stack);;
    // deti: Expr, Block => arity = 2
    push_check(stackSymbolPush(Stack, make_reduce(AST_WHILE, 2)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Block)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_RPAREN)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_LPAREN)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_WHILE)));
}

void For_func(DynamicSymbolStackPtr Stack, Token token)
{
    // For -> for ( IDENT in Expr ) Block
    stackSymbolPop(Stack);;
    // deti: Expr, Block => arity = 2  (IDENT v AST neukladáme)
    push_check(stackSymbolPush(Stack, make_reduce(AST_FOR, 3)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Block)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_RPAREN)));
    push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_IN)));
    push_check(stackSymbolPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_LPAREN)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_FOR)));
}

void Break_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    // jednoduchý statement (bez detí); zabalíme ho ako AST_STATEMENT (0)
    push_check(stackSymbolPush(Stack, make_reduce(AST_BREAK, 0)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_BREAK)));
}

void Continue_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    // jednoduchý statement (bez detí)
    push_check(stackSymbolPush(Stack, make_reduce(AST_CONTINUE, 0)));
    push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_CONTINUE)));
}

void Return_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    /* Striktné určenie arity podľa lookaheadu:
       return <expr>  => AST_RETURN(1)
       return         => AST_RETURN(0)
    */
   if (token.type==TOKEN_IDENTIFIER || token.type==TOKEN_GLOBAL_ID ||
       token.type==TOKEN_INTEGER   || token.type==TOKEN_FLOAT     ||
       token.type==TOKEN_STRING    || token.type==TOKEN_LPAREN)
    {
       push_check(stackSymbolPush(Stack, make_reduce(AST_RETURN, 1)));
       push_check(stackSymbolPush(Stack, make_nonterm(N_Return_expr)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_RETURN)));
    } else {
        push_check(stackSymbolPush(Stack, make_reduce(AST_RETURN, 0)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_KEYWORD_RETURN)));
   }
}

void Return_expr_func(DynamicSymbolStackPtr Stack, Token token)
{
    stackSymbolPop(Stack);;
    switch (token.type)
    {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
            push_check(stackSymbolPush(Stack, make_nonterm(N_Expr)));
            return;

        case TOKEN_NEW_LINE:
        case TOKEN_BLOCK_END:
        case TOKEN_RPAREN:
            return; // ε – žiadny uzol, aritu už rozhodol Return_func

         default:
            printError(SYNTAX_ERROR, "Neočakávaný token v Return_expr\n");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void Expr_func(DynamicSymbolStackPtr Stack, Token token, Token *gLookahead, DynamicAstNodeStackPtr gAstStack)
{
    // odstráň neterminál
    stackSymbolPop(Stack);;

    if (!gLookahead || !gAstStack) {
        printError(INTERNAL_ERROR, "Expr_func: chýbajú hooky\n");
        memorySafeExit(INTERNAL_ERROR);
    }

    AstNode *expr = parseExpression(gLookahead);
    if (!expr)
    {
        printError(SYNTAX_ERROR, "Chybný výraz\n");
        memorySafeExit(SYNTAX_ERROR);
    }

    // výsledný uzol výrazu dáme na AST zásobník
    push_check(stackAstNodePush(gAstStack, expr));
}

void  New_line_func(DynamicSymbolStackPtr Stack, Token token)
{

    stackSymbolPop(Stack);;
    if (token.type == TOKEN_NEW_LINE)
    {
        push_check(stackSymbolPush(Stack,make_nonterm(N_New_line_check)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_NEW_LINE)));
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
    stackSymbolPop(Stack);;
    if (token.type == TOKEN_NEW_LINE)
    {
        // Reprezentuje "NEW_LINE Opt_newlines"
        push_check(stackSymbolPush(Stack, make_nonterm(N_Opt_newlines)));
        push_check(stackSymbolPush(Stack, make_term(TOKEN_NEW_LINE)));
        return;
    }
    // ε (nič - keď už nepríde NEW_LINE)
}

Symbol make_reduce(AstNodeType kind, size_t arity)
{
    Symbol x = {0};
    x.is_terminal = false;
    x.is_reduce = true;
    x.reduce_kind = kind;
    x.reduce_arity = arity;
    return x;
}

Symbol make_nonterm(Nonterm N)
{
    Symbol x = {0};
    x.is_terminal = false;
    x.is_reduce = false;
    x.symbol = N;
    return x;
}

Symbol make_term_value(Token token)
{
    Symbol x = {0};
    x.is_terminal = true;
    x.symbol = token.type;
    x.value_needed = true;
    x.is_reduce = false;
    x.capture_value = false;
    switch (token.type)
    {
        case TOKEN_INTEGER:
            x.value.number = token.value.number;
            break;
        case TOKEN_FLOAT:
            x.value.number = token.value.number;
            break;
        case TOKEN_STRING:
            x.value.string = token.value.string;
            break;
        case TOKEN_IDENTIFIER:
            x.value.identifier = token.value.identifier;
            break;
        default:
            x.value_needed = false;
            break;
    }
    return x;
}

Symbol make_term(TokenType T)
{
    Symbol x = {0};
    x.is_terminal = true;
    x.symbol = T;
    x.value_needed = false;
    x.capture_value = false;
    return x;
}


Symbol make_term_with_value(TokenType type)
{
    Symbol x = {0};
    x.is_terminal = true;
    x.symbol = type;
    x.value_needed = false;
    x.capture_value = true;
    return x;

}

Symbol make_term_value_and_capture_identifier(const char *exact) {
    Symbol x = {0};
    x.is_terminal = true;
    x.symbol = TOKEN_IDENTIFIER;
    x.value_needed = true;
    x.capture_value = true;
    x.value.identifier = (char*)exact;
    return x;
}
