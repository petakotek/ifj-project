/*
 *
 * Autoři: xpasekj00
 * Soubor: ast_strom.c
 * Popis: zajistuje ulozeni vysledku z parseru do stromu
 */
#include "ast_strom.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "support_functions.h"

AstNode *build_classblock_flat(size_t n, AstNode **kids) {
    if (n == 0) {
        AstNode *node = (AstNode*)calloc(1, sizeof(AstNode));
        if (!node) { printError(INTERNAL_ERROR, "alloc MAIN_CLASS_BLOCK(0)"); memorySafeExit(INTERNAL_ERROR); }
        node->kind = AST_MAIN_CLASS_BLOCK;
        node->arity = 0;
        return node;
    }

    // (FUNC_DEC, tail MAIN_CLASS_BLOCK)
    if (n == 2 && kids[1] && kids[1]->kind == AST_MAIN_CLASS_BLOCK) {
        size_t tail = kids[1]->arity;
        AstNode **flat = (AstNode**)malloc(sizeof(AstNode*) * (tail + 1));
        if (!flat) { printError(INTERNAL_ERROR, "alloc classblock flat"); memorySafeExit(INTERNAL_ERROR); }
        flat[0] = kids[0];
        if (tail) memcpy(&flat[1], kids[1]->child, sizeof(AstNode*) * tail);

        AstNode *node = ast_new(AST_MAIN_CLASS_BLOCK, tail + 1, flat);
        free(flat);

        free(kids[1]->child);
        free(kids[1]);
        return node;
    }

    return ast_new(AST_MAIN_CLASS_BLOCK, n, kids);
}

// same shape as your build_param_flat, but for statement lists:
AstNode *build_stmtlist_flat(size_t n, AstNode **kids) {
    if (n == 0) {                       // ε
        AstNode *node = calloc(1, sizeof *node);
        if (!node) { printError(INTERNAL_ERROR,"alloc STMT_LIST(0)"); memorySafeExit(INTERNAL_ERROR); }
        node->kind = AST_STATEMENT_LIST;
        node->arity = 0;
        return node;
    }
    if (n == 2 && kids[1] && kids[1]->kind == AST_STATEMENT_LIST) {
        size_t tail = kids[1]->arity;
        AstNode **flat = malloc(sizeof(AstNode*) * (tail + 1));
        if (!flat) { printError(INTERNAL_ERROR,"alloc stmtlist flat"); memorySafeExit(INTERNAL_ERROR); }
        flat[0] = kids[0];
        if (tail) memcpy(&flat[1], kids[1]->child, sizeof(AstNode*) * tail);
        AstNode *node = ast_new(AST_STATEMENT_LIST, tail + 1, flat);
        free(flat);
        free(kids[1]->child);
        free(kids[1]);
        return node;
    }
    return ast_new(AST_STATEMENT_LIST, n, kids);
}


// Sploští pravorekurzívny PARAM zoznam na ploché deti: IDENT, IDENT, ...
AstNode *build_param_flat(size_t n, AstNode **kids) {
    // ε-vetva: prázdny zoznam
    if (n == 0) {
        AstNode *node = (AstNode*)calloc(1, sizeof(AstNode));
        if (!node) { printError(INTERNAL_ERROR, "alloc PARAM(0)"); memorySafeExit(INTERNAL_ERROR); }
        node->kind = AST_PARAM;
        node->arity = 0;
        return node;
    }

    // 2-prvková vetva: (IDENT, tail PARAM)
    if (n == 2 && kids[0] && kids[1] && kids[1]->kind == AST_PARAM) {
        size_t tail = kids[1]->arity;

        AstNode **flat = (AstNode**)malloc(sizeof(AstNode*) * (tail + 1));
        if (!flat) { printError(INTERNAL_ERROR, "alloc param flat"); memorySafeExit(INTERNAL_ERROR); }

        flat[0] = kids[0]; // head IDENT
        if (tail > 0) {
            // prenesieme deti z tail PARAM, nie samotný obal
            memcpy(&flat[1], kids[1]->child, sizeof(AstNode*) * tail);
        }

        // vytvor nový plochý PARAM
        AstNode *node = ast_new(AST_PARAM, tail + 1, flat);
        free(flat);

        // zrušíme len obal tail PARAM (nie jeho deti – tie sme prebrali)
        free(kids[1]->child);
        free(kids[1]);

        return node;
    }

    // fallback (napr. jediný IDENT bez tailu – ak by si niekde chcel)
    return ast_new(AST_PARAM, n, kids);
}

AstNode *ast_new(AstNodeType kind, size_t arity, AstNode **kids) {

    AstNode *new_node = (AstNode*)calloc(1, sizeof(AstNode));
    if (!new_node) {
        printError(INTERNAL_ERROR, "Chyba alokace: new_node");
        memorySafeExit(INTERNAL_ERROR);
    }
    

    new_node->kind = kind;
    new_node->arity = arity;

    if (arity > 0) 
    {
        
        //fprintf(stderr, "  -> Alokujem pole detí pre %zu uzlov\n", arity);
        
        new_node->child = (AstNode**)malloc(sizeof(AstNode*) * arity);
        if (!new_node->child) {
            printError(INTERNAL_ERROR, "Chyba alokácie detí AST uzla\n");
            memorySafeExit(INTERNAL_ERROR);
        }
        for (size_t i = 0; i < arity; ++i)
        {
            
            //fprintf(stderr, "    -> Priraďujem dieťa[%zu] = %d\n", i, kids[i]->value);
            
            new_node->child[i] = kids[i];
        }

    }
    //fprintf(stderr, "[AST-NEW] Uzol %s (%p) úspešne vytvorený\n", ast_node_type_to_string(new_node->kind),  (void*)new_node);
    

    return new_node;
}


AstNode *new_leaf(AstNodeType kind, Token token) {
    AstNode *newLeaf = (AstNode*)calloc(1, sizeof(AstNode));
    if (!newLeaf) {
        printError(INTERNAL_ERROR, "Chyba alokácie AST listu\n");
        memorySafeExit(INTERNAL_ERROR);
    }
    newLeaf->kind = kind;
    newLeaf->is_leaf = true;

    ast_value_from_token(&newLeaf->value, kind, &token);
    
    //if (newLeaf->value.has_str)
    //    fprintf(stderr, "  -> String hodnota: '%s'\n", newLeaf->value.str);
    //else if (newLeaf->value.is_int)
    //    fprintf(stderr, "  -> Int hodnota: %d\n", newLeaf->value.int_val);
    //else if (newLeaf->value.is_float)
    //   fprintf(stderr, "  -> Float hodnota: %f\n", newLeaf->value.float_val);
    
    return newLeaf;
}


void ast_free_whole(AstNode *node) {
    if (!node) 
    {
        return;
    }
    if (node->is_leaf) 
    {
        if (node->value.has_str) 
        {
            free(node->value.str);
            node->value.str = NULL;
        }
        free(node);
        return;
    }
    for (size_t i = 0; i < node->arity; ++i) 
    {
        ast_free_whole(node->child[i]);
    }
    free(node->child);
    free(node);
}



void ast_value_from_token(AstValue *out, AstNodeType kind, Token *token) {
    memset(out, 0, sizeof(*out));
    switch (kind) 
    {
        case AST_INT_LIT:
            out->is_int = true;
            out->int_val = strtoll(token->value.string,NULL,0);   // prispôsob ak máš odlišné polia
            break;
        case AST_FLOAT_LIT:
            out->is_float = true;
            out->float_val = strtod(token->value.string,NULL);
            break;
        case AST_STRING_LIT:
            out->has_str = true;
            out->str = dup_cstr(token->value.string);
            break;
        case AST_IDENTIFIER:
        case AST_GLOBAL_ID: 
            out->has_str = true;
            out->str = dup_cstr(token->value.identifier);
            break;
        default:
            // pre nelistové alebo iné typy nič nekopíruj
            break;
    }
}

char *dup_cstr(const char *string) {
    if (!string) 
    {
        return NULL;
    }
    size_t len = strlen(string) + 1;
    char *newString = (char*)malloc(len);
    if (!newString) {
        printError(INTERNAL_ERROR, "Chyba alokácie (dup_cstr)\n");
        memorySafeExit(INTERNAL_ERROR);
    }
    memcpy(newString, string, len);
    return newString;
}



const char *ast_node_type_to_string(AstNodeType kind) {
    switch (kind) {
        case AST_PROGRAM:          return "PROGRAM";
        case AST_PROLOG:           return "PROLOG";
        case AST_MAIN_CLASS:       return "MAIN_CLASS";
        case AST_MAIN_CLASS_BLOCK: return "MAIN_CLASS_BLOCK";
        case AST_BLOCK:            return "BLOCK";
        case AST_STATEMENT:        return "STATEMENT";
        case AST_STATEMENT_LIST:   return "STATEMENT_LIST";
        case AST_FUNC_DEC:         return "FUNC_DEC";
        case AST_PARAM:            return "PARAM";
        case AST_FUNC_CALL:        return "FUNC_CALL";
        case AST_VAR:              return "VAR";
        case AST_ASSIGN:           return "ASSIGN";
        case AST_IF:               return "IF";
        case AST_WHILE:            return "WHILE";
        case AST_FOR:              return "FOR";
        case AST_BREAK:            return "BREAK";
        case AST_CONTINUE:         return "CONTINUE";
        case AST_RETURN:           return "RETURN";
        case AST_IDENTIFIER:       return "IDENTIFIER";
        case AST_GLOBAL_ID:        return "GLOBAL_ID";
        case AST_INT_LIT:          return "INT_LIT";
        case AST_FLOAT_LIT:        return "FLOAT_LIT";
        case AST_STRING_LIT:       return "STRING_LIT";
    case AST_PLUS:                  return "STRING_PLUS";
    case AST_MINUS:                  return "STRING_MINUS";
    case AST_MUL:                  return "STRING_MUL";
    case AST_DIV:                  return "STRING_DIV";
        case AST_EMPTY: return "EMPTY";
        default:                   return "UNKNOWN_AST_NODE";
    }
}



static void ast_dump_rec(const AstNode *n, int depth, const AstNode *parent) {
    if (!n) {
        fprintf(stderr, "%*s(null)\n", depth * 2, "");
        return;
    }

    // Ochrana proti samoodkazovaniu
    if (n == parent) {
        fprintf(stderr, "%*s[CYCLE DETECTED: node %p refers to itself]\n", depth * 2, "", (void*)n);
        return;
    }

    fprintf(stderr, "%*s[%s]", depth * 2, "", ast_node_type_to_string(n->kind));

    if (n->is_leaf) {
        if (n->value.has_str && n->value.str)
            fprintf(stderr, " (str=\"%s\")", n->value.str);
        else if (n->value.is_int)
            fprintf(stderr, " (int=%lld)", n->value.int_val);
        else if (n->value.is_float)
            fprintf(stderr, " (float=%g)", n->value.float_val);
    }

    fprintf(stderr, "\n");

    for (size_t i = 0; i < n->arity; ++i) {
        ast_dump_rec(n->child[i], depth + 1, n);
    }
}

void ast_dump(const AstNode *root) {
    fprintf(stderr, "===== AST STROM =====\n");
    ast_dump_rec(root, 0, NULL);
}