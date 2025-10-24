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
#include "ast_strom.h"

Symbol make_term(TokenType T)
{
    Symbol x;
    x.is_terminal = true;
    x.symbol = T;
    x.value_needed = false;
    return x;
}

Symbol make_term_value(Token T)
{
    Symbol x;
    x.is_terminal = true;
    x.symbol = T.type;
    x.value_needed = true;
    switch (T.type)
    {
        case TOKEN_INTEGER:
            x.value.identifier = T.value.identifier;
            break;
        case TOKEN_FLOAT:
            x.value.number = T.value.number;
            break;
        case TOKEN_STRING:
            x.value.string = T.value.string;
            break;
        default:
            x.value_needed = false;
            break;
    }
    return x;
}

Symbol make_nonterm(Nonterm N)
{
    Symbol x;
    x.is_terminal = false;
    x.symbol = N;
    return x;
}


Symbol make_reduce(AstNodeType kind, size_t arity)
{
    Symbol x = {0};
    x.is_reduce = true;
    x.reduce_kind = kind;
    x.reduce_arity = arity;
    return x;
}
void push_check(bool boolean);

void parser()
{
    DynamicStackPtr AstStack = stackConstruct(DEFAULT_INIT_SIZE);
    DynamicStackPtr Stack = stackConstruct(DEFAULT_INIT_SIZE);
    if (Stack == NULL | AstStack == NULL)
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

    while (!stackIsEmpty(Stack))
    {
        Stack_top = stackRead(Stack);
        if(Stack_top.is_terminal)
        {

            if (Stack_top.is_reduce)
            {
                stackPop(Stack);
                size_t n = Stack_top.reduce_arity;

                AstNode **kids = n ? malloc(n * sizeof(AstNode*)) : NULL;
                if (n && !kids) { printError(INTERNAL_ERROR, "AST alloc"); memorySafeExit(INTERNAL_ERROR); }

                // na vrchu sú pravé deti; preto vkladáme odzadu
                for (size_t i = 0; i < n; ++i) {
                    AstNode *kid = (AstNode*)stackRead(AstStack);
                    stackPop(AstStack);
                    kids[n - 1 - i] = kid;
                }

                AstNode *parent = ast_new(Stack_top.reduce_kind, n, kids);
                free(kids);
                stackPush(AstStack, parent);
                continue;
            }

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
                        default:
                            ok = true;
                    }
                }

                switch (token.type) {
                    case TOKEN_STRING:
                        stackPush(AstStack, new_leaf(NK_String, token));
                        break;
                    case TOKEN_INTEGER:
                        stackPush(AstStack, new_leaf(NK_Int, token));
                        break;
                    case TOKEN_FLOAT:
                        stackPush(AstStack, new_leaf(NK_Float, token));
                        break;
                    default:
                        // kľúčové slová, NEW_LINE, interpunkcia: zvyčajne do AST nejdú => nič nepushuj
                        break;
                }
                if (!ok) {
                    printError(SYNTAX_ERROR, "Nesúlad hodnoty terminálu");
                    memorySafeExit(SYNTAX_ERROR);
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
        else
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
                case N_Main_class_item:
                    Main_class_item_func(Stack, token);
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
                    Expr_func(Stack, token);
                    break;
                default:
                    printError(INTERNAL_ERROR, "Nespravny Neterminal");
                    memorySafeExit(INTERNAL_ERROR);
                    break;
            }
        }
    }
}

void push_check(bool boolean)
{
    if(!boolean)
    {
        printError(INTERNAL_ERROR,"Chyba pushovania na Stack");
        memorySafeExit(INTERNAL_ERROR);
    }
}

void Program_func(DynamicStackPtr Stack, Token token)
{
    stackPop(Stack);
    stackPush(Stack,make_nonterm(N_Main_class));
    stackPush(Stack,make_term(TOKEN_NEW_LINE));
    stackPush(Stack,make_nonterm(N_Prolog));
}

void Prolog_func(DynamicStackPtr Stack, Token token)
{
    push_check(stackPush(Stack, make_reduce(NK_Prolog, 1)));

    stackPop(Stack);
    stackPush(Stack,make_term(TOKEN_KEYWORD_IFJ));
    stackPush(Stack,make_term(TOKEN_KEYWORD_FOR));
    Token ifj25;
    ifj25.type = TOKEN_STRING;
    ifj25.value.string = "ifj25";
    stackPush(Stack,make_term_value(ifj25));
    stackPush(Stack,make_term(TOKEN_KEYWORD_IMPORT));
}

/* --- prediktívne funkcie podľa novej LL gramatiky (len premenované prototypy) --- */
void Main_class_func(DynamicStackPtr Stack, Token token)          { NOT_IMPL("Main_class_func"); }
void Main_class_block_func(DynamicStackPtr Stack, Token token)    { NOT_IMPL("Main_class_block_func"); }
void Main_class_item_func(DynamicStackPtr Stack, Token token)     { NOT_IMPL("Main_class_item_func"); }

void Block_func(DynamicStackPtr Stack, Token token)               { NOT_IMPL("Block_func"); }
void Block_content_func(DynamicStackPtr Stack, Token token)       { NOT_IMPL("Block_content_func"); }
void smth_func(DynamicStackPtr Stack, Token token)                { NOT_IMPL("smth_func"); }
void smth_tail_func(DynamicStackPtr Stack, Token token)           { NOT_IMPL("smth_tail_func"); }
void smth_list_func(DynamicStackPtr Stack, Token token)           { NOT_IMPL("smth_list_func"); }
void Oneline_expr_opt_func(DynamicStackPtr Stack, Token token)    { NOT_IMPL("Oneline_expr_opt_func"); }

void Func_dec_func(DynamicStackPtr Stack, Token token)            { NOT_IMPL("Func_dec_func"); }
void Func_dec_tail_func(DynamicStackPtr Stack, Token token)       { NOT_IMPL("Func_dec_tail_func"); }
void Param_func(DynamicStackPtr Stack, Token token)               { NOT_IMPL("Param_func"); }
void Param_list_func(DynamicStackPtr Stack, Token token)          { NOT_IMPL("Param_list_func"); }
void Func_call_func(DynamicStackPtr Stack, Token token)           { NOT_IMPL("Func_call_func"); }

void Var_func(DynamicStackPtr Stack, Token token)                 { NOT_IMPL("Var_func"); }
void Var_EXT_func(DynamicStackPtr Stack, Token token)             { NOT_IMPL("Var_EXT_func"); }
void Var_arg_func(DynamicStackPtr Stack, Token token)             { NOT_IMPL("Var_arg_func"); }
void var_arg_list_func(DynamicStackPtr Stack, Token token)        { NOT_IMPL("var_arg_list_func"); }

void Call_assign_func(DynamicStackPtr Stack, Token token)         { NOT_IMPL("Call_assign_func"); }
void Call_assign_list_func(DynamicStackPtr Stack, Token token)    { NOT_IMPL("Call_assign_list_func"); }
void Global_assign_func(DynamicStackPtr Stack, Token token)       { NOT_IMPL("Global_assign_func"); }

void If_func(DynamicStackPtr Stack, Token token)                  { NOT_IMPL("If_func"); }
void Else_func(DynamicStackPtr Stack, Token token)                { NOT_IMPL("Else_func"); }
void While_func(DynamicStackPtr Stack, Token token)               { NOT_IMPL("While_func"); }
void For_func(DynamicStackPtr Stack, Token token)                 { NOT_IMPL("For_func"); }
void Break_func(DynamicStackPtr Stack, Token token)               { NOT_IMPL("Break_func"); }
void Continue_func(DynamicStackPtr Stack, Token token)            { NOT_IMPL("Continue_func"); }

void Return_func(DynamicStackPtr Stack, Token token)              { NOT_IMPL("Return_func"); }
void Return_expr_func(DynamicStackPtr Stack, Token token)         { NOT_IMPL("Return_expr_func"); }
void Expr_func(DynamicStackPtr Stack, Token token)                { NOT_IMPL("Expr_func"); }
