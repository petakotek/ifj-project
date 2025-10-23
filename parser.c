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
    switch (T.type) 
    {
        case TOKEN_INTEGER:     
            x.value.identifier = T.value.identifier; 
            break;
        case TOKEN_FLOAT:   
            x.value.number = T.value.number; 
            break;
        case TOKEN_STRING:
            x.value.string = T.value.string; // predpoklad: vlastník je lexer; ak treba, použi strdup()
            break;
        default:
            // ak typ nemá „hodnotu“, tak has_value nemá význam
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

void push_check(bool boolean);

void parser()
{
    DynamicStackPtr Stack = stackConstruct(DEFAULT_INIT_SIZE);
    if (Stack == NULL) 
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
                            ok = (Stack_top.value.identifier == token.value.identifier);
                            break;
                        case TOKEN_FLOAT:
                            ok = (Stack_top.value.number == token.value.number);
                            break;
                        default:
                            // pre typy bez hodnoty porovnávať netreba
                            ok = true;
                        }
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
                case N_MainClass:
                    MainClass_func(Stack, token);
                    break;
                case N_MainClassBlock:
                    MainClassBlock_func(Stack, token);
                    break;
                case N_MainClassItem:
                    MainClassItem_func(Stack, token);
                    break;
                case N_Block:
                    Block_func(Stack, token);
                    break;
                case N_Smth:
                    Smth_func(Stack, token);
                    break;
                case N_SmthTail:
                    SmthTail_func(Stack, token);
                    break;
                case N_SmthList:
                    SmthList_func(Stack, token);
                    break;
                case N_FuncDec:
                    FuncDec_func(Stack, token);
                    break;
                case N_Param:
                    Param_func(Stack, token);
                    break;
                case N_ParamList:
                    ParamList_func(Stack, token);
                    break;
                case N_Getter:
                    Getter_func(Stack, token);
                    break;
                case N_Setter:
                    Setter_func(Stack, token);
                    break;
                case N_Var:
                    Var_func(Stack, token);
                    break;
                case N_Var_EXT:
                    Var_EXT_func(Stack, token);
                    break;
                case N_Var_EXTSTAT:
                    Var_EXTSTAT_func(Stack, token);
                    break;
                case N_VarArg:
                    VarArg_func(Stack, token);
                    break;
                case N_VarArgList:
                    VarArgList_func(Stack, token);
                    break;
                case N_Assign:
                    Assign_func(Stack, token);
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
                case N_ReturnExpr:
                    ReturnExpr_func(Stack, token);
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
    stackPush(Stack,make_nonterm(N_MainClass));
    stackPush(Stack,make_term(TOKEN_NEW_LINE));
    stackPush(Stack,make_nonterm(N_Prolog));
}
void Prolog_func(DynamicStackPtr Stack, Token token)
{ 
    stackPop(Stack);
    stackPush(Stack,make_term(TOKEN_KEYWORD_IFJ));
    stackPush(Stack,make_term(TOKEN_KEYWORD_FOR));
    Token ifj25;
    ifj25.type = TOKEN_IDENTIFIER;
    ifj25.value.string = "ifj25";
    stackPush(Stack,make_term_value(ifj25));
    stackPush(Stack,make_term(TOKEN_KEYWORD_IMPORT));

}
void MainClass_func(DynamicStackPtr Stack, Token token)      { NOT_IMPL("MainClass_func"); }
void MainClassBlock_func(DynamicStackPtr Stack, Token token) { NOT_IMPL("MainClassBlock_func"); }
void MainClassItem_func(DynamicStackPtr Stack, Token token)  { NOT_IMPL("MainClassItem_func"); }
void Block_func(DynamicStackPtr Stack, Token token)          { NOT_IMPL("Block_func"); }
void Smth_func(DynamicStackPtr Stack, Token token)           { NOT_IMPL("Smth_func"); }
void SmthTail_func(DynamicStackPtr Stack, Token token)       { NOT_IMPL("SmthTail_func"); }
void SmthList_func(DynamicStackPtr Stack, Token token)       { NOT_IMPL("SmthList_func"); }
void FuncDec_func(DynamicStackPtr Stack, Token token)        { NOT_IMPL("FuncDec_func"); }
void Param_func(DynamicStackPtr Stack, Token token)          { NOT_IMPL("Param_func"); }
void ParamList_func(DynamicStackPtr Stack, Token token)      { NOT_IMPL("ParamList_func"); }
void Getter_func(DynamicStackPtr Stack, Token token)         { NOT_IMPL("Getter_func"); }
void Setter_func(DynamicStackPtr Stack, Token token)         { NOT_IMPL("Setter_func"); }
void Var_func(DynamicStackPtr Stack, Token token)            { NOT_IMPL("Var_func"); }
void Var_EXT_func(DynamicStackPtr Stack, Token token)        { NOT_IMPL("Var_EXT_func"); }
void Var_EXTSTAT_func(DynamicStackPtr Stack, Token token)    { NOT_IMPL("Var_EXTSTAT_func"); }
void VarArg_func(DynamicStackPtr Stack, Token token)         { NOT_IMPL("VarArg_func"); }
void VarArgList_func(DynamicStackPtr Stack, Token token)     { NOT_IMPL("VarArgList_func"); }
void Assign_func(DynamicStackPtr Stack, Token token)         { NOT_IMPL("Assign_func"); }
void If_func(DynamicStackPtr Stack, Token token)             { NOT_IMPL("If_func"); }
void Else_func(DynamicStackPtr Stack, Token token)           { NOT_IMPL("Else_func"); }
void While_func(DynamicStackPtr Stack, Token token)          { NOT_IMPL("While_func"); }
void For_func(DynamicStackPtr Stack, Token token)            { NOT_IMPL("For_func"); }
void Break_func(DynamicStackPtr Stack, Token token)          { NOT_IMPL("Break_func"); }
void Continue_func(DynamicStackPtr Stack, Token token)       { NOT_IMPL("Continue_func"); }
void Return_func(DynamicStackPtr Stack, Token token)         { NOT_IMPL("Return_func"); }
void ReturnExpr_func(DynamicStackPtr Stack, Token token)     { NOT_IMPL("ReturnExpr_func"); }
void Expr_func(DynamicStackPtr Stack, Token token)           { NOT_IMPL("Expr_func"); }

