#include "ast_strom.h"
#include "lexer.h"
AstNode *ast_new(AstNodeType kind, size_t n, AstNode **kids) {
    AstNode *new_node = malloc(sizeof(AstNode));
    if (!new_node) {
        // ... ošetření chyby alokace
    }

    new_node->type = kind;
    new_node->name = NULL; // Nastavení NULL pro bezpečnost
    new_node->children = NULL;
    new_node->children_count = 0;

    // --- Zde implementujte logiku pro nastavení UNIE a pole dětí ---

    if (kind == AST_PROGRAM) {
        // Pevná struktura (Program -> Prolog NEW_LINE MainClass)
        new_node->data.Program.prolog = kids[0]; // Prolog
        new_node->data.Program.main_class = kids[2]; // MainClass
        // Uvolnění syntaktického uzlu:
        //ast_free_single(kids[1]); // kids[1] je uzel pro TOKEN_NEW_LINE
    }
    // ... další specifické uzly (NK_If, NK_Func_dec, atd.) ...

    // Vracíme rodiče
    return new_node;
}
