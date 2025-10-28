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
    DynamicStackPtr AstStack = stackConstruct(DEFAULT_INIT_SIZE);
    DynamicStackPtr Stack = stackConstruct(DEFAULT_INIT_SIZE);
    if (Stack == NULL || AstStack == NULL)
    {
        printError(INTERNAL_ERROR,"Chyba alokácie");
        memorySafeExit(INTERNAL_ERROR);
    }

    bool push;
    push = stackPush(Stack,make_term(TOKEN_EOF));
    push_check(push);
    push = stackPush(Stack,make_nonterm(N_Program));
    push_check(push);
    Token token = lexer_token_next();
    Symbol Stack_top;

    Token *gLookahead = NULL;
    DynamicStackPtr gAstStack = NULL;
    gLookahead = &token;
    gAstStack  = AstStack;




    while (!stackIsEmpty(Stack))
    {
        Stack_top = stackRead(Stack);
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

            // na vrchu sú pravé deti; preto vkladáme odzadu
            for (size_t i = 0; i < n; ++i)
            {
                if (stackIsEmpty(AstStack))
                {
                    printError(SYNTAX_ERROR, "Vnútorná chyba: málo detí na AST zásobníku pri redukcii\n");
                    memorySafeExit(SYNTAX_ERROR);
                }
                AstNode *kid = aststack_read(AstStack);
                stackPop(AstStack);
                kids[n - 1 - i] = kid;
            }
            AstNode *parent = ast_new(Stack_top.reduce_kind, n, kids);
            stackPop(Stack);
            free(kids);
            aststack_push(AstStack,parent);
            continue;
        }

        else if(Stack_top.is_terminal)
        {

            printf("%d\n",token.type);
            printf("%d\n",Stack_top.symbol);
            if (Stack_top.symbol == token.type)
            {
                bool ok = true;
                bool concrete_value = false;
                if (Stack_top.value_needed)
                {
                    switch (token.type)
                    {
                        case TOKEN_STRING:
                            ok = (token.value.string && Stack_top.value.string && strcmp(Stack_top.value.string, token.value.string) == 0);
                            concrete_value = true;
                            break;
                        case TOKEN_INTEGER:
                            ok = (Stack_top.value.number == token.value.number);
                            concrete_value = true;
                            break;
                        case TOKEN_FLOAT:
                            ok = (Stack_top.value.number == token.value.number);
                            concrete_value = true;
                            break;
                        case TOKEN_IDENTIFIER:
                            ok = (token.value.identifier && Stack_top.value.identifier && strcmp(Stack_top.value.identifier, token.value.identifier) == 0);
                            concrete_value = true;
                            break;
                        
                        default:
                            ok = true;
                    }
                }


                if (!ok) {
                    printError(SYNTAX_ERROR, "Nesúlad hodnoty terminálu\n");
                    memorySafeExit(SYNTAX_ERROR);
                }

                if (concrete_value)
                {
                    Stack_top.capture_value = true;
                }

                if (Stack_top.capture_value)
                {
                    switch (token.type) {
                    case TOKEN_STRING:
                        aststack_push(AstStack, new_leaf(AST_STRING_LIT, token));
                        break;
                    case TOKEN_INTEGER:
                        aststack_push(AstStack, new_leaf(AST_INT_LIT, token));
                        break;
                    case TOKEN_FLOAT:
                        aststack_push(AstStack, new_leaf(AST_FLOAT_LIT, token));
                        break;
                    case TOKEN_IDENTIFIER:
                        //aststack_push(AstStack,new_leaf(AST_IDENTIFIER,token));
                        break;
                    case TOKEN_GLOBAL_ID:
                        //aststack_push(AstStack,new_leaf(AST_GLOBAL_ID,token));
                        break;
                    default:
                        // kľúčové slová, NEW_LINE, interpunkcia: zvyčajne do AST nejdú => nič nepushuj
                        break;
                    }
                }



                printf("ok\n");
                stackPop(Stack);
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

    if (stackIsEmpty(AstStack))
    {
        printError(SYNTAX_ERROR, "AST je prázdny\n");
        memorySafeExit(SYNTAX_ERROR);
    }


    AstNode *ast_root = aststack_read(AstStack);
    stackDestruct(Stack);
    stackDestruct(AstStack);

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

void Program_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    push_check(stackPush(Stack, make_reduce(AST_PROGRAM, 2)));
    push_check(stackPush(Stack,make_nonterm(N_Main_class)));
    push_check(stackPush(Stack, make_nonterm(N_Opt_newlines)));
    push_check(stackPush(Stack,make_term(TOKEN_NEW_LINE)));
    push_check(stackPush(Stack,make_nonterm(N_Prolog)));
}

void Prolog_func(DynamicStackPtr Stack, Token token)
{

    stackPop(Stack);
    push_check(stackPush(Stack, make_reduce(AST_PROLOG,0)));

    push_check(stackPush(Stack,make_term(TOKEN_KEYWORD_IFJ)));
    push_check(stackPush(Stack,make_term(TOKEN_KEYWORD_FOR)));
    Token ifj25;
    ifj25.type = TOKEN_STRING;
    ifj25.value.string = "ifj25";
    push_check(stackPush(Stack,make_term_value(ifj25)));
    push_check(stackPush(Stack,make_term(TOKEN_KEYWORD_IMPORT)));
}

void Main_class_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    push_check(stackPush(Stack, make_reduce(AST_MAIN_CLASS, 1)));
    push_check(stackPush(Stack,make_nonterm(N_New_line_check)));
    push_check(stackPush(Stack,make_term(TOKEN_BLOCK_END)));

    push_check(stackPush(Stack,make_nonterm(N_Main_class_block)));
    push_check(stackPush(Stack,make_term(TOKEN_NEW_LINE)));
    push_check(stackPush(Stack,make_term(TOKEN_BLOCK_STAR)));
    Token Program_class;
    Program_class.type = TOKEN_IDENTIFIER;
    Program_class.value.identifier = "Program";
    push_check(stackPush(Stack,make_term_value(Program_class)));
    push_check(stackPush(Stack,make_term(TOKEN_KEYWORD_CLASS)));
}

void Main_class_block_func(DynamicStackPtr Stack, Token token)
{
    //popneme  neterminál
    stackPop(Stack);

    // FIRST(Func_dec) = TOKEN_KEYWORD_STATIC
    // Epsilon vyberáme, keď vidíme '}' (koniec bloku) => TOKEN_BLOCK_END
    if (token.type == TOKEN_KEYWORD_STATIC) {
        // Vetva: Main_class_block -> Main_class_item Main_class_block
        push_check(stackPush(Stack, make_reduce(AST_MAIN_CLASS_BLOCK, 2)));
        push_check(stackPush(Stack, make_nonterm(N_Main_class_block))); // ... Main_class_block
        push_check(stackPush(Stack, make_nonterm(N_Func_dec)));  // Main_class_item
        return;
    }
    else if (token.type == TOKEN_NEW_LINE)
    {
        push_check(stackPush(Stack, make_nonterm(N_Main_class_block)));
        push_check(stackPush(Stack, make_term(TOKEN_NEW_LINE)));
        return;
    }
    else if (token.type == TOKEN_BLOCK_END) {
        // Vetva: Main_class_block -> eps
        // vytvoríme prázdny uzol bloku.
        push_check(stackPush(Stack, make_reduce(AST_MAIN_CLASS_BLOCK, 0)));
        return;
    }

    else
    {// Nič z FIRST ani FOLLOW: syntaktická chyba
        printError(SYNTAX_ERROR, "Očakávaný 'static' alebo '}' v Main_class_block");
        memorySafeExit(SYNTAX_ERROR);
    }


}

void Block_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    // Block má práve 1 dieťa = Block_content (zabalíme ho do jedného uzla)
    push_check(stackPush(Stack, make_reduce(AST_BLOCK, 1)));
    push_check(stackPush(Stack, make_term(TOKEN_NEW_LINE)));
    push_check(stackPush(Stack, make_term(TOKEN_BLOCK_END)));
    push_check(stackPush(Stack, make_nonterm(N_Block_content)));
    push_check(stackPush(Stack, make_term(TOKEN_BLOCK_STAR)));
}

/* === Block_content -> NEW_LINE smth | Oneline_expr_opt =================
   Dôležité: Block_content MUSÍ vyprodukovať 1 uzol (aby AST_BLOCK(1) nepadol).
   Preto ho vždy zabalíme do "obalového" uzla s aritou 1.
*/
void Block_content_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);

    if (token.type == TOKEN_NEW_LINE) {
        // NEW_LINE smth  → jeden statement na riadku
        push_check(stackPush(Stack, make_reduce(AST_STATEMENT, 1)));
        push_check(stackPush(Stack, make_nonterm(N_smth)));
        push_check(stackPush(Stack, make_term(TOKEN_NEW_LINE)));
        return;
    }

    // jednoriadkové vyjadrenie alebo ε
    switch (token.type) {
        // FIRST(Expr)
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
            // máme výraz → 1 dieťa
            push_check(stackPush(Stack, make_reduce(AST_STATEMENT, 1)));
            push_check(stackPush(Stack, make_nonterm(N_Expr)));
            return;

        // FOLLOW → ε (žiadny statement tu)
        case TOKEN_BLOCK_END:
        case TOKEN_RPAREN:
        case TOKEN_NEW_LINE:
            push_check(stackPush(Stack, make_reduce(AST_STATEMENT, 0)));
            return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token v Block_content");
            memorySafeExit(SYNTAX_ERROR);
    }
}

/* === smth -> smth_list smth_tail =======================================
   Keďže obidve vetvy môžu mať ε, zvolíme rodiča s aritou 2,
   a v ε vetvách budeme produkovať prázdne uzly (0), aby počet sedel.
*/
void smth_func(DynamicStackPtr Stack, Token token) {
    // smth -> smth_list smth_tail
    stackPop(Stack);
    push_check(stackPush(Stack, make_nonterm(N_smth_tail)));
    push_check(stackPush(Stack, make_nonterm(N_smth_list)));
}

void smth_tail_func(DynamicStackPtr Stack, Token token) {
    // smth_tail -> NEW_LINE smth | eps
    stackPop(Stack);
    if (token.type == TOKEN_NEW_LINE) {
        push_check(stackPush(Stack, make_nonterm(N_smth)));
        push_check(stackPush(Stack, make_term(TOKEN_NEW_LINE)));
    } // eps = nič nepúšťam
}

void smth_list_func(DynamicStackPtr Stack, Token token) {
    // smth_list -> Return | Var | If | While | For | Break | Continue | Call_assign | Global_assign | eps
    stackPop(Stack);

    switch (token.type) {
        case TOKEN_KEYWORD_RETURN:   push_check(stackPush(Stack, make_nonterm(N_Return)));        return;
        case TOKEN_KEYWORD_VAR:      push_check(stackPush(Stack, make_nonterm(N_Var)));           return;
        case TOKEN_KEYWORD_IF:       push_check(stackPush(Stack, make_nonterm(N_If)));            return;
        case TOKEN_KEYWORD_WHILE:    push_check(stackPush(Stack, make_nonterm(N_While)));         return;
        case TOKEN_KEYWORD_FOR:      push_check(stackPush(Stack, make_nonterm(N_For)));           return;
        case TOKEN_KEYWORD_BREAK:    push_check(stackPush(Stack, make_nonterm(N_Break)));         return;
        case TOKEN_KEYWORD_CONTINUE: push_check(stackPush(Stack, make_nonterm(N_Continue)));      return;
        case TOKEN_IDENTIFIER:       push_check(stackPush(Stack, make_nonterm(N_Call_assign)));   return;
        case TOKEN_GLOBAL_ID:        push_check(stackPush(Stack, make_nonterm(N_Global_assign))); return;
        case TOKEN_BLOCK_END:
        case TOKEN_NEW_LINE:
            push_check(stackPush(Stack, make_reduce(AST_STATEMENT_LIST, 0)));
           return; // eps
        default:
            printError(SYNTAX_ERROR, "Neočakávaný token na začiatku statementu");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void Oneline_expr_opt_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    switch (token.type)
    {
        // FIRST(Expr): všetky tokeny, ktorými môže začínať výraz
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
            push_check(stackPush(Stack, make_nonterm(N_Expr)));
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

void Func_dec_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    push_check(stackPush(Stack, make_nonterm(N_Func_dec_tail)));
    push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
    push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_STATIC)));
}

void Func_dec_tail_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);

    if (token.type == TOKEN_LPAREN)
    {
        // Vetva: ( Param ) Block
        push_check(stackPush(Stack, make_reduce(AST_FUNC_DEC, 2)));
        push_check(stackPush(Stack, make_nonterm(N_Block)));
        push_check(stackPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackPush(Stack, make_nonterm(N_Param)));
        push_check(stackPush(Stack, make_term(TOKEN_LPAREN)));
    }
    else if (token.type == TOKEN_ASSIGN)
    {
        // Vetva: = ( identifier ) Block (getter/setter)
        push_check(stackPush(Stack, make_reduce(AST_FUNC_DEC, 2)));
        push_check(stackPush(Stack, make_nonterm(N_Block)));
        push_check(stackPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
        push_check(stackPush(Stack, make_term(TOKEN_LPAREN)));
        push_check(stackPush(Stack, make_term(TOKEN_ASSIGN)));
    }
    else if (token.type == TOKEN_BLOCK_STAR)
    {
        // Vetva: Block
        push_check(stackPush(Stack, make_reduce(AST_FUNC_DEC, 1)));
        push_check(stackPush(Stack, make_nonterm(N_Block)));
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávané '(' alebo '=' vo Func_dec_tail\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}
void Param_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);

    if (token.type == TOKEN_IDENTIFIER)
    {
        push_check(stackPush(Stack, make_reduce(AST_PARAM, 2)));
        push_check(stackPush(Stack, make_nonterm(N_Param_list)));
        push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
    }
    else if (token.type == TOKEN_RPAREN)
    {
        push_check(stackPush(Stack, make_reduce(AST_PARAM, 0))); // eps
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávaný identifikátor alebo ')' v Param\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void Param_list_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    if (token.type == TOKEN_COMMA)
    {
        push_check(stackPush(Stack, make_reduce(AST_PARAM, 2)));
        push_check(stackPush(Stack, make_nonterm(N_Param_list)));
        push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
        push_check(stackPush(Stack, make_term(TOKEN_COMMA)));
    }
    else if (token.type == TOKEN_RPAREN)
    {
        push_check(stackPush(Stack, make_reduce(AST_PARAM, 0))); // eps
    }
    else
    {
        printError(SYNTAX_ERROR, "Očakávaný ',' alebo ')' v Param_list");
        memorySafeExit(SYNTAX_ERROR);
    }
}
void Func_call_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);

    if (token.type == TOKEN_IDENTIFIER) {
        // IDENT ( Var_arg )
        // redukujeme na AST_FUNC_CALL s 1 dieťaťom (Var_arg)
        push_check(stackPush(Stack, make_reduce(AST_FUNC_CALL, 2)));
        push_check(stackPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackPush(Stack, make_nonterm(N_Var_arg)));
        push_check(stackPush(Stack, make_term(TOKEN_LPAREN)));
        push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
        return;
    }
    else if (token.type == TOKEN_KEYWORD_IFJ) {
        // KEYWORD_IFJ . IDENT ( Var_arg )
        push_check(stackPush(Stack, make_reduce(AST_FUNC_CALL, 2)));
        push_check(stackPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackPush(Stack, make_nonterm(N_Var_arg)));
        push_check(stackPush(Stack, make_term(TOKEN_LPAREN)));
        push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
        push_check(stackPush(Stack, make_term(TOKEN_DOT)));
        push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_IFJ)));
        return;
    }

    printError(SYNTAX_ERROR, "Očakávaný identifikátor alebo 'ifj' na začiatku Func_call");
    memorySafeExit(SYNTAX_ERROR);
}

void Var_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    push_check(stackPush(Stack, make_reduce(AST_VAR, 2)));
    push_check(stackPush(Stack, make_nonterm(N_Var_EXT)));
    push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
    push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_VAR)));
}

void Var_EXT_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    if (token.type == TOKEN_ASSIGN)
    {
        push_check(stackPush(Stack, make_reduce(AST_ASSIGN, 1)));
        push_check(stackPush(Stack, make_nonterm(N_Expr)));
        push_check(stackPush(Stack, make_term(TOKEN_ASSIGN)));
    }
    
    if (token.type == TOKEN_NEW_LINE)
    {
        
    }
    else
    {
        printError(SYNTAX_ERROR,"chyba za var");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void Var_arg_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    switch (token.type) {
        // FIRST(Expr)
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
            // Expr var_arg_list  => uložíme ako AST_PARAM (zoznam argumentov) => arity = 2
            push_check(stackPush(Stack, make_reduce(AST_PARAM, 2)));
            push_check(stackPush(Stack, make_nonterm(N_Var_arg_list)));
            push_check(stackPush(Stack, make_nonterm(N_Expr)));
            return;

        // eps (žiadne argumenty)
        case TOKEN_RPAREN:
            push_check(stackPush(Stack, make_reduce(AST_PARAM, 0)));
            return;

        default:
            printError(SYNTAX_ERROR, "Neočakávaný token vo Var_arg");
            memorySafeExit(SYNTAX_ERROR);
    }
}

void var_arg_list_func(DynamicStackPtr Stack, Token token)
{
     stackPop(Stack);
     if (token.type == TOKEN_COMMA) {
        // , Expr var_arg_list  => Expr (1) + tail (1) => arity = 2
        push_check(stackPush(Stack, make_reduce(AST_PARAM, 2)));
         push_check(stackPush(Stack, make_nonterm(N_Var_arg_list)));
         push_check(stackPush(Stack, make_nonterm(N_Expr)));
         push_check(stackPush(Stack, make_term(TOKEN_COMMA)));
    } else if (token.type == TOKEN_RPAREN) {
         // eps
         push_check(stackPush(Stack, make_reduce(AST_PARAM, 0)));
    } else {
         printError(SYNTAX_ERROR, "Neočakávaný token vo var_arg_list");
         memorySafeExit(SYNTAX_ERROR);
    }
}

void Call_assign_func(DynamicStackPtr Stack, Token token)
{
    // Call_assign -> IDENT Call_assign_list
    stackPop(Stack);
    // výsledok vytvára Call_assign_list (assign alebo call) => arity = 1
    push_check(stackPush(Stack, make_reduce(AST_STATEMENT, 2)));
    push_check(stackPush(Stack, make_nonterm(N_Call_assign_list)));
    push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
}

void Call_assign_list_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    if (token.type == TOKEN_ASSIGN) {
        // = Expr  => AST_ASSIGN(Expr) => arity = 1
        push_check(stackPush(Stack, make_reduce(AST_ASSIGN, 1)));
        push_check(stackPush(Stack, make_nonterm(N_Expr)));
        push_check(stackPush(Stack, make_term(TOKEN_ASSIGN)));
    } else if (token.type == TOKEN_LPAREN) {
        // ( Var_arg )  => AST_FUNC_CALL(Var_arg) => arity = 1
        push_check(stackPush(Stack, make_reduce(AST_FUNC_CALL, 1)));
        push_check(stackPush(Stack, make_term(TOKEN_RPAREN)));
        push_check(stackPush(Stack, make_nonterm(N_Var_arg)));
        push_check(stackPush(Stack, make_term(TOKEN_LPAREN)));
    } else {
        printError(SYNTAX_ERROR, "Očakávané '=' alebo '(' v Call_assign_list");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void Global_assign_func(DynamicStackPtr Stack, Token token)
{
    // Global_assign -> GLOBAL_ID = Expr
    stackPop(Stack);
    // AST_ASSIGN(Expr) => arity = 1
    push_check(stackPush(Stack, make_reduce(AST_ASSIGN, 2)));
    push_check(stackPush(Stack, make_nonterm(N_Expr)));
    push_check(stackPush(Stack, make_term(TOKEN_ASSIGN)));
    push_check(stackPush(Stack, make_term_with_value(TOKEN_GLOBAL_ID)));
}

void If_func(DynamicStackPtr Stack, Token token)
{
    // If -> if ( Expr ) Block Else
    stackPop(Stack);
    // deti: Expr, Block, Else => arity = 3
    push_check(stackPush(Stack, make_reduce(AST_IF, 3)));
    push_check(stackPush(Stack, make_nonterm(N_Else)));
    push_check(stackPush(Stack, make_nonterm(N_Block)));
    push_check(stackPush(Stack, make_term(TOKEN_RPAREN)));
    push_check(stackPush(Stack, make_nonterm(N_Expr)));
    push_check(stackPush(Stack, make_term(TOKEN_LPAREN)));
    push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_IF)));
}

void Else_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);

    if (token.type == TOKEN_KEYWORD_ELSE) {
        // else Block | else If  => 1 dieťa (Block alebo If)
        push_check(stackPush(Stack, make_reduce(AST_IF, 1)));
        // Rozhodnutie necháme na ďalší krok (podľa lookaheadu po spárovaní ELSE)
        push_check(stackPush(Stack, make_nonterm(N_Else_body)));
        push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_ELSE)));
    } else {
        // eps
        push_check(stackPush(Stack, make_reduce(AST_IF, 0)));
    }
}

void ElseBody_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    if (token.type == TOKEN_KEYWORD_IF) {
        // ElseBody -> If
        push_check(stackPush(Stack, make_nonterm(N_If)));
    } else if (token.type == TOKEN_BLOCK_STAR) {
        // ElseBody -> Block
        push_check(stackPush(Stack, make_nonterm(N_Block)));
    } else {
        printError(SYNTAX_ERROR, "Po 'else' očakávam 'if' alebo '{'\n");
        memorySafeExit(SYNTAX_ERROR);
    }
}

void While_func(DynamicStackPtr Stack, Token token)
{
    // While -> while ( Expr ) Block
    stackPop(Stack);
    // deti: Expr, Block => arity = 2
    push_check(stackPush(Stack, make_reduce(AST_WHILE, 2)));
    push_check(stackPush(Stack, make_nonterm(N_Block)));
    push_check(stackPush(Stack, make_term(TOKEN_RPAREN)));
    push_check(stackPush(Stack, make_nonterm(N_Expr)));
    push_check(stackPush(Stack, make_term(TOKEN_LPAREN)));
    push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_WHILE)));
}

void For_func(DynamicStackPtr Stack, Token token)
{
    // For -> for ( IDENT in Expr ) Block
    stackPop(Stack);
    // deti: Expr, Block => arity = 2  (IDENT v AST neukladáme)
    push_check(stackPush(Stack, make_reduce(AST_FOR, 3)));
    push_check(stackPush(Stack, make_nonterm(N_Block)));
    push_check(stackPush(Stack, make_term(TOKEN_RPAREN)));
    push_check(stackPush(Stack, make_nonterm(N_Expr)));
    push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_IN)));
    push_check(stackPush(Stack, make_term_with_value(TOKEN_IDENTIFIER)));
    push_check(stackPush(Stack, make_term(TOKEN_LPAREN)));
    push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_FOR)));
}

void Break_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    // jednoduchý statement (bez detí); zabalíme ho ako AST_STATEMENT (0)
    push_check(stackPush(Stack, make_reduce(AST_BREAK, 0)));
    push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_BREAK)));
}

void Continue_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    // jednoduchý statement (bez detí)
    push_check(stackPush(Stack, make_reduce(AST_CONTINUE, 0)));
    push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_CONTINUE)));
}

void Return_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    /* Striktné určenie arity podľa lookaheadu:
       return <expr>  => AST_RETURN(1)
       return         => AST_RETURN(0)
    */
   if (token.type==TOKEN_IDENTIFIER || token.type==TOKEN_GLOBAL_ID ||
       token.type==TOKEN_INTEGER   || token.type==TOKEN_FLOAT     ||
       token.type==TOKEN_STRING    || token.type==TOKEN_LPAREN)
    {
       push_check(stackPush(Stack, make_reduce(AST_RETURN, 1)));
       push_check(stackPush(Stack, make_nonterm(N_Return_expr)));
        push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_RETURN)));
    } else {
        push_check(stackPush(Stack, make_reduce(AST_RETURN, 0)));
        push_check(stackPush(Stack, make_term(TOKEN_KEYWORD_RETURN)));
   }
}

void Return_expr_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    switch (token.type)
    {
        case TOKEN_IDENTIFIER:
        case TOKEN_GLOBAL_ID:
        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_LPAREN:
            push_check(stackPush(Stack, make_nonterm(N_Expr)));
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

void Expr_func(DynamicStackPtr Stack, Token token, Token *gLookahead, DynamicStackPtr gAstStack)
{
    // odstráň neterminál
    stackPop(Stack);

    if (!gLookahead || !gAstStack) {
        printError(INTERNAL_ERROR, "Expr_func: chýbajú hooky\n");
        memorySafeExit(INTERNAL_ERROR);
    }

    AstNode *expr = precedence_parse_expr(gLookahead);
    if (!expr)
    {
        printError(SYNTAX_ERROR, "Chybný výraz\n");
        memorySafeExit(SYNTAX_ERROR);
    }

    // výsledný uzol výrazu dáme na AST zásobník
    aststack_push(gAstStack, expr);
}

void  New_line_func(DynamicStackPtr Stack, Token token)
{

    stackPop(Stack);
    if (token.type == TOKEN_NEW_LINE)
    {
        push_check(stackPush(Stack,make_nonterm(N_New_line_check)));
        push_check(stackPush(Stack, make_term(TOKEN_NEW_LINE)));
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

void Opt_newlines_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    if (token.type == TOKEN_NEW_LINE)
    {
        // Reprezentuje "NEW_LINE Opt_newlines"
        push_check(stackPush(Stack, make_nonterm(N_Opt_newlines)));
        push_check(stackPush(Stack, make_term(TOKEN_NEW_LINE)));
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


Symbol make_astnode(struct AstNode *n) {
    Symbol x = {0};
    x.is_astnode = true;
    x.ast_ptr = n;
    return x;
}

void aststack_push(DynamicStackPtr S, struct AstNode *n)
{
    push_check(stackPush(S, make_astnode(n)));
}

struct AstNode* aststack_read(DynamicStackPtr S)
{
    Symbol top = stackRead(S);
    return top.ast_ptr;
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
