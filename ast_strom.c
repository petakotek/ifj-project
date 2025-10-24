#include "ast_strom.h"
#include <cstdlib>
#include "lexer.h"
AstNode *ast_new(AstNodeType kind, size_t n, AstNode **kids) {
    AstNode *new_node = malloc(sizeof(AstNode));
    if (!new_node) {

    }

    new_node->type = kind;
    new_node->value.identifier_name = NULL; // Nastavení NULL pro bezpečnost
    new_node->children = NULL;
    new_node->children_count = 0;

    /*
    *Program -> Prolog TOKEN_NEW_LINE Main_class TOKEN_EOF
    */
    if (kind == AST_PROGRAM) {
            new_node->data.Program.prolog = kids[0];
            new_node->data.Program.main_class = kids[2];
            ast_free_single(kids[1]); // TOKEN_NEW_LINE
            ast_free_single(kids[3]); // TOKEN_EOF
        }

        if (kind == AST_FUNC_DEC) {
            // Pravidlo: STATIC TOKEN_IDENTIFIER Func_dec_tail
            // Děti by měly být: [STATIC, IDENTIFIER, Func_dec_tail (Block/Assignment)]
            // Předpokládáme, že kids[1] je listový uzel s jménem
            new_node->value.identifier_name = strdup(kids[1]->value.identifier_name);

            // Zpracování Func_dec_tail (Block nebo přiřazení)
            // Děti se budou lišit podle Func_dec_tail (zde jen ukázka pro základní funkci)
            new_node->data.Func_dec.params = kids[2]; // AST_PARAM_LIST/NULL
            new_node->data.Func_dec.block = kids[3]; // AST_BLOCK

            ast_free_single(kids[0]); // STATIC
            ast_free_single(kids[1]); // Jméno (již zkopírováno, původní uzel pryč)
        }

        // Pozn: AST_GETTER a AST_SETTER by měly být řešeny v rámci Func_dec_tail redukce,
        // ale pokud jsou to samostatné redukce:
        if (kind == AST_GETTER) {
            // Zjednodušená ukázka
            new_node->data.Getter.block = kids[2];
            new_node->value.identifier_name = strdup(kids[1]->value.identifier_name);
            // ... (uvolnění syntaktických tokenů)
        }

        if (kind == AST_SETTER) {
            // Zjednodušená ukázka
            new_node->data.Setter.param_name = strdup(kids[3]->value.identifier_name); // Param name
            new_node->data.Setter.block = kids[4];
            new_node->value.identifier_name = strdup(kids[1]->value.identifier_name); // Setter name
            // ... (uvolnění syntaktických tokenů)
        }

        if (kind == AST_IF) {
            // Pravidlo: IF ( Expr ) Block Else
            // Arita: 6 (IF, '(', Expr, ')', Block, Else)
            new_node->data.If.condition = kids[2];
            new_node->data.If.if_block = kids[4];
            new_node->data.If.else_block = kids[5]; // Může být NULL (pro eps)

            // Uvolnění syntaxe
            ast_free_single(kids[0]); // IF
            ast_free_single(kids[1]); // (
            ast_free_single(kids[3]); // )
        }

        if (kind == AST_WHILE) {
            // Pravidlo: WHILE ( Expr ) Block
            // Arita: 5 (WHILE, '(', Expr, ')', Block)
            new_node->data.While.condition = kids[2];
            new_node->data.While.block = kids[4];

            // Uvolnění syntaxe
            ast_free_single(kids[0]); // WHILE
            ast_free_single(kids[1]); // (
            ast_free_single(kids[3]); // )
        }

        if (kind == AST_FOR) {
            // Pravidlo: FOR ( IDENTIFIER IN Expr ) Block
            // Arita: 7 (FOR, '(', IDENTIFIER, IN, Expr, ')', Block)
            new_node->value.identifier_name = strdup(kids[2]->value.identifier_name); // Iterator name
            new_node->data.For.container_expr = kids[4];
            new_node->data.For.block = kids[6];

            // Uvolnění syntaxe
            ast_free_single(kids[0]); // FOR
            ast_free_single(kids[1]); // (
            ast_free_single(kids[3]); // IN
            ast_free_single(kids[5]); // )
            ast_free_single(kids[2]); // Identifier uzel (již zkopírován)
        }

        if (kind == AST_RETURN) {
            // Pravidlo: RETURN Return_expr
            // Arita: 2 (RETURN, Return_expr (Expr / NULL))
            new_node->data.Return.return_expr = kids[1];
            ast_free_single(kids[0]); // RETURN
        }

        if (kind == AST_VAR) {
            // Pravidlo: VAR IDENTIFIER Var_EXT
            // Arita: 3 (VAR, IDENTIFIER, Var_EXT (Assign/eps))
            new_node->value.identifier_name = strdup(kids[1]->value.identifier_name);
            new_node->data.Var.var_ext = kids[2]; // AST_ASSIGN nebo NULL
            ast_free_single(kids[0]); // VAR
            ast_free_single(kids[1]); // Identifier uzel
        }

        if (kind == AST_ASSIGN) {
            // Zde by měla redukce Call_assign nebo Global_assign zajistit, že kids[0] je cílový uzel/jméno
            // Zjednodušený pohled: Kids[0] je CÍL (Identifikátor/Global) a Kids[2] je výraz.
            new_node->data.Assign.target_var = kids[0]; // Předpokládá se již hotový uzel cíle (VAR, Global, Call_assign)
            new_node->data.Assign.expr = kids[2];
            ast_free_single(kids[1]); // TOKEN_ASSIGN
        }

        // --- Uzly využívající dynamické pole new_node->children ---

        if (kind == AST_STATEMENT_LIST || kind == AST_PARAM_LIST || kind == AST_MAIN_CLASS_BLOCK) {
            // Tyto uzly reprezentují SEZNAMY.
            // KIDS je pole uzlů, které by měly být všechny ponechány (bez syntaktického čištění).

            // Ukládáme pole kids a jeho velikost
            new_node->children = kids;
            new_node->children_count = n;

            // POZOR: Původní pole kids (pointer) se NEUvolní zde, ale stává se součástí AST.
            // V redukčním bloku byste měli mallocovat pole kids pro danou velikost.
            // Nyní se jen předá vlastnictví paměti
            return new_node;
        }

        if (kind == AST_BLOCK) {
            // Pravidlo: TOKEN_BLOCK_START Block_content TOKEN_BLOCK_END TOKEN_NEW_LINE
            // Arita: 4
            new_node->children = kids[1]->children; // Převezme děti z AST_STATEMENT_LIST (Block_content)
            new_node->children_count = kids[1]->children_count;

            // Uvolnění syntaxe a AST_STATEMENT_LIST uzlu
            ast_free_single(kids[0]); // {
            ast_free_single(kids[2]); // }
            ast_free_single(kids[3]); // NEW_LINE
            ast_free_single(kids[1]); // Uzel Block_content (již přesunul své děti)
        }

        // --- Jednoduché uzly bez dětí ---

        if (kind == AST_BREAK || kind == AST_CONTINUE || kind == AST_EXPR) {
            // Tyto uzly nemají (nebo v případě Expr zatím neznáme) pevnou strukturu dětí
            // a nepotřebují speciální logiku v unii.
            // Vracíme prázdný uzel s nastaveným typem.
        }

    // Vracíme rodiče
    return new_node;
}
void ast_free_single(AstNode *kid) {
    if (!kid) {
        return;
    }
    if (kid->value.identifier_name != NULL) {
        free(kid->value.identifier_name);
    }
    if(kid->value.string_value != NULL){
    	free(kid->value.identifier_name);
    }
    free(kid);
}
