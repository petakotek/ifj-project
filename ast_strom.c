/*
 *
 * Autoři: xpasekj00
 * Soubor: ast_strom.c
 * Popis: zajistuje ulozeni vysledku z parseru do stromu
 */
#define _POSIX_C_SOURCE 200809L
#include "ast_strom.h"

#include <string.h>
#include "support_functions.h"
/*
 * Macro pro kontrolu pristupu do kids abychom nesalahi na co nemame
 */
#define SAFE_CHILD(i) ((kids != NULL && kids[i] != NULL) ? kids[i] : NULL)
#define SAFE_ID(i)                                           \
    ((SAFE_CHILD(i) && SAFE_CHILD(i)->value.identifier_name) \
         ? SAFE_CHILD(i)->value.identifier_name              \
         : NULL )
#define SAFE_STR(i) \
    ((SAFE_CHILD(i) && SAFE_CHILD(i)->value.string_value) ? SAFE_CHILD(i)->value.string_value : "")
#define SAFE_INT(i) ((SAFE_CHILD(i)) ? SAFE_CHILD(i)->value.int_value : 0)
#define SAFE_FLOAT(i) ((SAFE_CHILD(i)) ? SAFE_CHILD(i)->value.float_value : 0.0)

/*
 *  Vytváří nový uzel abstraktního syntaktického stromu (AST).
 *  Funkce pouze ukládá odkazy na již vytvořené poduzly (kids) nebo hodnoty z tokenů.
 *  @param:kind – typ uzlu (AST_NODE_TYPE)
 *      n    – počet potomků
 *      kids – pole ukazatelů na potomky
 *
 *  @return: Ukazatel na nově vytvořený uzel AST.
 */
AstNode *ast_new(AstNodeType kind, AstNode **kids) {
    fprintf(stderr, "DEBUG: ast_new volán pro typ: %s\n", ast_node_type_to_string(kind));

    // Bezpečnostní kontrola vstupu (typ uzlu)
    if (kind < 0 || kind >= AST_TYPE_COUNT) {
        printError(INTERNAL_ERROR, "Neplatný typ AST uzlu");
        memorySafeExit(INTERNAL_ERROR);
    }

    AstNode *new_node = malloc(sizeof(AstNode));
    if (!new_node) {
        printError(INTERNAL_ERROR, "Chyba alokace: new_node");
        memorySafeExit(INTERNAL_ERROR);
    }

    // Inicializace nového uzlu
    memset(new_node, 0, sizeof(AstNode));
    new_node->type = kind;

    // ==========================
    //  Specifické typy uzlů
    // ==========================
    switch (kind) {
        // Program -> Prolog TOKEN_NEW_LINE Main_class TOKEN_EOF
        case AST_PROGRAM:
            new_node->data.Program.main_class = SAFE_CHILD(1);
            new_node->data.Program.prolog = SAFE_CHILD(0);
            fprintf(stderr, "DEBUG: Vytvořen AST_PROGRAM. Prolog: %p, MainClass: %p\n",
                    (void *)SAFE_CHILD(0), (void *)SAFE_CHILD(1));
            break;

        // Func_dec -> TOKEN_KEYWORD_STATIC TOKEN_IDENTIFIER Func_dec_tail
        case AST_FUNC_DEC: {
            AstNode *k0 = SAFE_CHILD(0);  // Identifikátor (TOKEN_IDENTIFIER)
            AstNode *k1 = SAFE_CHILD(1);  // Parametry, Block nebo Assign/Setter hlavička

            const char *id                  = SAFE_ID(0);
            new_node->value.identifier_name = id ? strdup(id) : NULL;

            if (k1) {
                if (k1->type == AST_PARAM_LIST) {
                    new_node->data.Func_dec.params = k1;
                    new_node->data.Func_dec.block  = SAFE_CHILD(2);
                } else if (k1->type == AST_ASSIGN) {
                    new_node->data.Func_dec.params = NULL;
                    new_node->data.Func_dec.block  = k1;
                } else if (k1->type == AST_BLOCK) {
                    new_node->data.Func_dec.block = k1;
                }
            }

            fprintf(stderr, "DEBUG: Vytvořen AST_FUNC_DEC. Jméno: %s, Parametry: %p, Blok: %p\n",
                    new_node->value.identifier_name ? new_node->value.identifier_name : "(null)",
                    (void *)new_node->data.Func_dec.params, (void *)new_node->data.Func_dec.block);
            break;
        }

        // If -> TOKEN_KEYWORD_IF TOKEN_LPAREN Expr TOKEN_RPAREN Block Else
        case AST_IF:
            new_node->data.If.condition  = SAFE_CHILD(0);  // Expr
            new_node->data.If.if_block   = SAFE_CHILD(1);  // Block
            new_node->data.If.else_block = SAFE_CHILD(2);  // Else (Block nebo If)
            fprintf(stderr, "DEBUG: Vytvořen AST_IF. Podmínka: %p, IF: %p, ELSE: %p\n",
                    (void *)SAFE_CHILD(0), (void *)SAFE_CHILD(1), (void *)SAFE_CHILD(2));
            break;

        // While -> TOKEN_KEYWORD_WHILE TOKEN_LPAREN Expr TOKEN_RPAREN Block
        case AST_WHILE:
            new_node->data.While.condition = SAFE_CHILD(0);  // Expr
            new_node->data.While.block     = SAFE_CHILD(1);  // Block
            fprintf(stderr, "DEBUG: Vytvořen AST_WHILE. Podmínka: %p, Blok: %p\n",
                    (void *)SAFE_CHILD(0), (void *)SAFE_CHILD(1));
            break;

        // For -> TOKEN_KEYWORD_FOR TOKEN_LPAREN TOKEN_IDENTIFIER TOKEN_KEYWORD_IN Expr TOKEN_RPAREN
        // Block
        case AST_FOR: {
            const char *id                    = SAFE_ID(0);  // Iterator name
            new_node->value.identifier_name   = id ? strdup(id) : NULL;
            new_node->data.For.container_expr = SAFE_CHILD(1);  // Expr (container)
            new_node->data.For.block          = SAFE_CHILD(2);  // Block
            fprintf(stderr, "DEBUG: Vytvořen AST_FOR. Proměnná: %s, Výraz: %p, Blok: %p\n",
                    id ? id : "(null)", (void *)SAFE_CHILD(1), (void *)SAFE_CHILD(2));
            break;
        }

        // Return -> TOKEN_KEYWORD_RETURN Return_expr (Expr | eps)
        case AST_RETURN:
            new_node->data.Return.return_expr = SAFE_CHILD(0);  // Expr nebo NULL
            fprintf(stderr, "DEBUG: Vytvořen AST_RETURN. Výraz: %p\n", (void *)SAFE_CHILD(0));
            break;

        // Var -> TOKEN_KEYWORD_VAR TOKEN_IDENTIFIER Var_EXT (TOKEN_ASSIGN Expr | eps)
        case AST_VAR: {
            const char *id                  = SAFE_ID(0);
            new_node->value.identifier_name = id ? strdup(id) : NULL;
            new_node->data.Var.var_ext      = SAFE_CHILD(1);  // Expr nebo NULL
            fprintf(stderr, "DEBUG: Vytvořen AST_VAR. Jméno: %s, Init. výraz: %p\n",
                    id ? id : "(null)", (void *)SAFE_CHILD(1));
            break;
        }

        // Reprezentuje Call_assign_list -> TOKEN_ASSIGN Expr nebo Global_assign -> TOKEN_GLOBAL_ID
        // TOKEN_ASSIGN Expr
        case AST_ASSIGN:
            new_node->data.Assign.target_var = SAFE_CHILD(0);  // Cíl (Var nebo Global_assign)
            new_node->data.Assign.expr       = SAFE_CHILD(1);  // Expr
            fprintf(stderr, "DEBUG: Vytvořen AST_ASSIGN. Cíl: %p, Výraz: %p\n",
                    (void *)SAFE_CHILD(0), (void *)SAFE_CHILD(1));
            break;

        // Param -> TOKEN_IDENTIFIER Param_list | eps (Reprezentuje jeden TOKEN_IDENTIFIER)
        case AST_PARAM: {
            const char *id                  = SAFE_ID(0);
            //new_node->value.identifier_name = kids[0]->value.identifie;
            fprintf(stderr, "DEBUG: Vytvořen AST_PARAM. Jméno: %s\n",id);
            break;
        }

        // Func_call -> TOKEN_IDENTIFIER TOKEN_LPAREN Var_arg TOKEN_RPAREN | TOKEN_KEYWORD_IFJ
        // TOKEN_DOT TOKEN_IDENTIFIER TOKEN_LPAREN Var_arg TOKEN_RPAREN
        case AST_FUNC_CALL: {
            const char *id                  = SAFE_ID(0);
            AstNode    *args                = SAFE_CHILD(1);
            new_node->value.identifier_name = id ? strdup(id) : NULL;

            if (args && args->children_count > 0) {
                new_node->children_count = args->children_count;
                new_node->children       = malloc(sizeof(AstNode *) * new_node->children_count);
                if (!new_node->children) {
                    printError(INTERNAL_ERROR, "Chyba alokace pro argumenty volání funkce");
                    memorySafeExit(INTERNAL_ERROR);
                }
                memcpy(new_node->children, args->children,
                       sizeof(AstNode *) * new_node->children_count);
            } else {
                new_node->children_count = 0;
            }

            fprintf(stderr, "DEBUG: Vytvořen AST_FUNC_CALL. Jméno: %s, Počet arg: %i\n",
                    id ? id : "(null)", (int)new_node->children_count);
            break;
        }

        // Literál (nemá přímé pravidlo, ale je součástí Expr)
        case AST_STRING_LIT: {
            const char *str              = SAFE_STR(0);
            new_node->value.string_value = str ? strdup(str) : NULL;
            fprintf(stderr, "DEBUG: Vytvořen AST_STRING_LIT. Hodnota: \"%s\"\n",
                    str ? str : "(null)");
            break;
        }

        // Literál (nemá přímé pravidlo, ale je součástí Expr)
        case AST_INT_LIT:
            new_node->value.int_value = SAFE_INT(0);
            fprintf(stderr, "DEBUG: Vytvořen AST_INT_LIT. Hodnota: %lld\n",
                    new_node->value.int_value);
            break;

        // Prolog -> TOKEN_KEYWORD_IMPORT TOKEN_STRING("ifj25") TOKEN_KEYWORD_FOR TOKEN_KEYWORD_IFJ
        case AST_PROLOG:
            // Prolog by měl nést hodnotu tokenu TOKEN_STRING("ifj25")
            fprintf(stderr, "DEBUG: Vytvořen AST_PROLOG. Import: %s\n",
                    new_node->value.string_value);
            break;

        case AST_FLOAT_LIT:
            new_node->value.float_value = SAFE_FLOAT(0);
            fprintf(stderr, "DEBUG: Vytvořen AST_FLOAT_LIT. Hodnota: %f\n",
                    new_node->value.float_value);
            break;

        // Statement_list, Main_class_block, Block, Param_list, Main_class
        case AST_STATEMENT_LIST:
        case AST_MAIN_CLASS_BLOCK:  // Main_class_block -> Func_dec Main_class_block | eps
        case AST_BLOCK:  // Block -> TOKEN_BLOCK_START Block_content TOKEN_BLOCK_END TOKEN_NEW_LINE
        case AST_PARAM_LIST:  // Param_list -> TOKEN_COMMA TOKEN_IDENTIFIER Param_list | eps
        case AST_MAIN_CLASS:  // Main_class -> ... Main_class_block ...
        {
            size_t total_count = 0;

            // Fáze 1: Zjištění celkového počtu dětí po zploštění
            for (size_t i = 0; kids && kids[i]; i++) {
                AstNode *child = kids[i];
                if (!child)
                    continue;

                if (child->type == AST_STATEMENT_LIST || child->type == AST_MAIN_CLASS_BLOCK ||
                    child->type == AST_BLOCK || child->type == AST_PARAM_LIST ||
                    child->type == AST_MAIN_CLASS) {
                    total_count += child->children_count;
                } else {
                    total_count++;
                }
            }

            if (total_count > 0) {
                new_node->children_count = total_count;
                new_node->children       = malloc(sizeof(AstNode *) * total_count);
                if (!new_node->children) {
                    printError(INTERNAL_ERROR, "Chyba alokace pro děti blokového uzlu");
                    memorySafeExit(INTERNAL_ERROR);
                }

                // Fáze 2: Propojení dětí a zploštění
                size_t counter = 0;
                for (size_t i = 0; kids && kids[i]; i++) {
                    AstNode *child = kids[i];
                    if (!child)
                        continue;

                    if (child->type == AST_STATEMENT_LIST || child->type == AST_MAIN_CLASS_BLOCK ||
                        child->type == AST_BLOCK || child->type == AST_PARAM_LIST ||
                        child->type == AST_MAIN_CLASS) {
                        for (int j = 0; j < child->children_count; j++) {
                            new_node->children[counter++] = child->children[j];
                        }
                    } else {
                        new_node->children[counter++] = child;
                    }
                }
            }
            for (int i = 0; i < total_count; i++) {
                fprintf(stderr, "DEBUG: ma potomky typu %s: \n",
                        ast_node_type_to_string(new_node->children[i]->type));
            }
            fprintf(stderr, "DEBUG: Vytvořen %s s %i potomky (flattened)\n",
                    ast_node_type_to_string(kind), (int)new_node->children_count);
            break;
        }

        // Statement je příkaz (smth_list -> Return | Var | If | While | For | Break | Continue |
        // Call_assign | Global_assign | eps)
        case AST_STATEMENT: {
            AstNode *child = SAFE_CHILD(0);

            if (child) {
                new_node->children_count = 1;
                new_node->children       = malloc(sizeof(AstNode *));
                if (!new_node->children) {
                    printError(INTERNAL_ERROR, "Chyba alokace pro AST_STATEMENT\n");
                    memorySafeExit(INTERNAL_ERROR);
                }
                new_node->children[0] = child;
            } else {
                new_node->children_count = 0;  // Empty statement
            }

            fprintf(stderr, "DEBUG: Vytvořen AST_STATEMENT s %i potomky\n",
                    (int)new_node->children_count);
            break;
        }

        // Break -> TOKEN_KEYWORD_BREAK
        case AST_BREAK:
        // Continue -> TOKEN_KEYWORD_CONTINUE
        case AST_CONTINUE:
            // Nemají žádné děti.
            fprintf(stderr, "DEBUG: Vytvořen %s (bez potomků)\n", ast_node_type_to_string(kind));
            break;

        default:
            fprintf(stderr, "DEBUG: Defaultní zpracování uzlu: %s.\n",
                    ast_node_type_to_string(kind));
            break;
    }

    fprintf(stderr, "DEBUG: ast_new návrat pro typ: %s, uzel: %p\n", ast_node_type_to_string(kind),
            (void *)new_node);
    return new_node;
}
void ast_traverse(AstNode *node, int depth) {
    if (node == NULL)
        return;

    // Odsazení podle úrovně
    for (int i = 0; i < depth; i++)
        printf("  ");

    // Výpis typu uzlu
    printf("Node type: %s\n", ast_node_type_to_string(node->type));

    // Rekurze podle typu uzlu
    switch (node->type) {
        case AST_PROGRAM:
            ast_traverse(node->data.Program.prolog, depth + 1);
            ast_traverse(node->data.Program.main_class, depth + 1);
            break;

        case AST_FUNC_DEC:
            if (node->value.identifier_name) {
                for (int i = 0; i < depth + 1; i++)
                    printf("  ");
                printf("Function: %s\n", node->value.identifier_name);
            }
            ast_traverse(node->data.Func_dec.params, depth + 1);
            ast_traverse(node->data.Func_dec.block, depth + 1);
            break;

        case AST_IF:
            ast_traverse(node->data.If.condition, depth + 1);
            ast_traverse(node->data.If.if_block, depth + 1);
            if (node->data.If.else_block)
                ast_traverse(node->data.If.else_block, depth + 1);
            break;

        case AST_WHILE:
            ast_traverse(node->data.While.condition, depth + 1);
            ast_traverse(node->data.While.block, depth + 1);
            break;

        case AST_FOR:
            for (int i = 0; i < depth + 1; i++)
                printf("  ");
            printf("For iterator: %s\n", node->value.identifier_name);
            ast_traverse(node->data.For.container_expr, depth + 1);
            ast_traverse(node->data.For.block, depth + 1);
            break;

        case AST_BLOCK:
        case AST_STATEMENT_LIST:
        case AST_PARAM_LIST:
        case AST_MAIN_CLASS_BLOCK:
        case AST_MAIN_CLASS:
            printf("pocet deti na print %i \n", node->children_count);
            for (int i = 0; i < node->children_count; i++) {
                ast_traverse(node->children[i], depth + 1);
            }
            break;

        case AST_VAR:
            for (int i = 0; i < depth + 1; i++) {
                printf("  ");
                //  printf("Var: %s\n", node->value.identifier_name);
            }
            ast_traverse(node->data.Var.var_ext, depth + 1);
            break;

        case AST_ASSIGN:
            ast_traverse(node->data.Assign.target_var, depth + 1);
            ast_traverse(node->data.Assign.expr, depth + 1);
            break;

        case AST_RETURN:
            ast_traverse(node->data.Return.return_expr, depth + 1);
            break;

        case AST_STRING_LIT:
            for (int i = 0; i < depth + 1; i++)
                printf("  ");
            printf("String: \"%s\"\n", node->value.string_value);
            break;

        case AST_IDENTIFIER:
            for (int i = 0; i < depth + 1; i++)
                printf("  ");
            printf("identifier : %s\n", node->value.identifier_name);
            break;
        case AST_INT_LIT:
            for (int i = 0; i < depth + 1; i++)
                printf("  ");
            printf("Int: %lld\n", node->value.int_value);
            break;

        case AST_FLOAT_LIT:
            for (int i = 0; i < depth + 1; i++)
                printf("  ");
            printf("Float: %f\n", node->value.float_value);
            break;
        case AST_STATEMENT:
            if (node->children_count > 0) {
                ast_traverse(node->children[0], depth + 1);
            }
            break;
        case AST_BREAK:
        case AST_CONTINUE:
            for (int i = 0; i < depth + 1; i++)
                printf("  ");
            printf("%s\n", node->type == AST_BREAK ? "Break" : "Continue");
            break;

        default:
            break;
    }
}
void ast_free_whole(AstNode *node) {
    if (node == NULL)
        return;

    if (node->children != NULL) {
        for (int i = 0; i < node->children_count; i++) {
            ast_free_whole(node->children[i]);
        }
        free(node->children);
    }

    switch (node->type) {
        case AST_PROGRAM:
            ast_free_whole(node->data.Program.prolog);
            ast_free_whole(node->data.Program.main_class);
            break;

        case AST_FUNC_DEC:
            ast_free_whole(node->data.Func_dec.params);
            ast_free_whole(node->data.Func_dec.block);
            if (node->value.identifier_name)
                free(node->value.identifier_name);
            break;

        case AST_IF:
            ast_free_whole(node->data.If.condition);
            ast_free_whole(node->data.If.if_block);
            ast_free_whole(node->data.If.else_block);
            break;

        case AST_WHILE:
            ast_free_whole(node->data.While.condition);
            ast_free_whole(node->data.While.block);
            break;

        case AST_FOR:
            ast_free_whole(node->data.For.container_expr);
            ast_free_whole(node->data.For.block);
            if (node->value.identifier_name)
                free(node->value.identifier_name);
            break;

        case AST_VAR:
            ast_free_whole(node->data.Var.var_ext);
            if (node->value.identifier_name)
                free(node->value.identifier_name);
            break;

        case AST_ASSIGN:
            ast_free_whole(node->data.Assign.target_var);
            ast_free_whole(node->data.Assign.expr);
            break;

        case AST_RETURN:
            ast_free_whole(node->data.Return.return_expr);
            break;

        case AST_STRING_LIT:
            if (node->value.string_value)
                free(node->value.string_value);
            break;

        case AST_PARAM:
            if (node->value.identifier_name)
                free(node->value.identifier_name);
            break;

        case AST_INT_LIT:
        case AST_FLOAT_LIT:
        case AST_BREAK:
        case AST_CONTINUE:
            // žádná dynamická paměť
            break;

        default:
            break;
    }

    free(node);
}
void ast_free_single(AstNode *kid) {
    if (!kid) {
        return;
    }
    switch (kid->type) {
        case AST_STRING_LIT:
            if (kid->value.string_value != NULL) {
                free(kid->value.string_value);
            }
            break;

        case AST_PARAM:
        case AST_VAR:
        case AST_FUNC_CALL:
        case AST_GETTER:
        case AST_SETTER:
        case AST_FOR:
        case AST_PROLOG:
            if (kid->value.identifier_name != NULL) {
                free(kid->value.identifier_name);
            }
            break;

        default:
            break;
    }
    free(kid);
}
AstNode *new_leaf(AstNodeType k, Token t) {
    AstNode *new_node = malloc(sizeof(AstNode));
    if (!new_node) {
        printError(INTERNAL_ERROR, "chyba v alokaci AST_node \n");
        memorySafeExit(INTERNAL_ERROR);
    }
    memset(new_node, 0, sizeof(AstNode));

    new_node->type           = k;
    new_node->children       = NULL;
    new_node->children_count = 0;
    if (k == AST_INT_LIT) {
        new_node->value.int_value = strtoll(t.value.identifier, NULL, 10);
        free(t.value.identifier);
    } else if (k == AST_FLOAT_LIT) {
        new_node->value.float_value = strtod(t.value.identifier, NULL);
        free(t.value.identifier);
    } else if (k == AST_STRING_LIT) {
        new_node->value.string_value = strdup(t.value.identifier);

    } else if (k == AST_IDENTIFIER) {
        new_node->value.identifier_name = strdup(t.value.identifier);
    }

    return new_node;
}
char *ast_node_type_to_string(AstNodeType type) {
    switch (type) {
        case AST_PROGRAM:
            return "AST_PROGRAM";
        case AST_PROLOG:
            return "AST_PROLOG";
        case AST_MAIN_CLASS:
            return "AST_MAIN_CLASS";
        case AST_FUNC_DEC:
            return "AST_FUNC_DEC";
        case AST_GETTER:
            return "AST_GETTER";
        case AST_SETTER:
            return "AST_SETTER";
        case AST_BLOCK:
            return "AST_BLOCK";
        case AST_VAR:
            return "AST_VAR";
        case AST_ASSIGN:
            return "AST_ASSIGN";
        case AST_IF:
            return "AST_IF";
        case AST_WHILE:
            return "AST_WHILE";
        case AST_FOR:
            return "AST_FOR";
        case AST_RETURN:
            return "AST_RETURN";
        case AST_BREAK:
            return "AST_BREAK";
        case AST_CONTINUE:
            return "AST_CONTINUE";
        case AST_FUNC_CALL:
            return "AST_FUNC_CALL";
        case AST_PARAM:
            return "AST_PARAM";
        case AST_PARAM_LIST:
            return "AST_PARAM_LIST";
        case AST_EXPR:
            return "AST_EXPR";
        case AST_STATEMENT_LIST:
            return "AST_STATEMENT_LIST";
        case AST_STRING_LIT:
            return "AST_STRING_LIT";
        case AST_INT_LIT:
            return "AST_INT_LIT";
        case AST_FLOAT_LIT:
            return "AST_FLOAT_LIT";
        case AST_MAIN_CLASS_BLOCK:
            return "AST_MAIN_CLASS_BLOCK";
        case AST_IDENTIFIER:
            return "AST_IDENTIFIER";
        case AST_GLOBAL_ID:
            return "AST_GLOBAL_ID";
        case AST_TYPE_COUNT:
            return "AST_TYPE_COUNT";
        case AST_STATEMENT:
            return "AST_STATEMENT";
        default:
            return "UNKNOWN_AST_NODE";
    }
}
