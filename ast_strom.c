/*
 *
 * Autoři: xpasekj00
 * Soubor: ast_strom.c
 * Popis: zajistuje ulozeni vysledku z parseru do stromu
 */
 #include "ast_strom.h"
AstNode *ast_new(AstNodeType kind, size_t n, AstNode **kids) {
    AstNode *new_node = malloc(sizeof(AstNode));
    if (!new_node) {
    printError(INTERNAL_ERROR, "chyba v alokaci AST_node \n");
    }

    new_node->type = kind;
    new_node->value.identifier_name = NULL;
    new_node->children = NULL;
    new_node->children_count = 0;

    /*
     * === Povinný Prolog a kostra programu ===
     * Program -> Prolog TOKEN_NEW_LINE Main_class TOKEN_EOF
     */
    if (kind == AST_PROGRAM) {
        new_node->data.Program.prolog = kids[0];
        new_node->data.Program.main_class = kids[2];
        ast_free_single(kids[1]); // TOKEN_NEW_LINE
        ast_free_single(kids[3]); // TOKEN_EOF
    }

    /*
     * === Funkcie, Getter a Setter ===
     * Func_dec -> TOKEN_KEYWORD_STATIC TOKEN_IDENTIFIER Func_dec_tail
     */
    if (kind == AST_FUNC_DEC) {
        new_node->value.identifier_name = strdup(kids[1]->value.identifier_name);
        new_node->data.Func_dec.params = kids[2]; // Param list / NULL
        new_node->data.Func_dec.block = kids[3]; // AST_BLOCK
        ast_free_single(kids[0]); // STATIC
        ast_free_single(kids[1]); // IDENTIFIER
    }
    /*
     * === Tělo hlavní třídy ===
     * Main_class_block -> Func_dec Main_class_block | ε
     */
    if (kind == AST_MAIN_CLASS_BLOCK) {
        if (n == 2) {
            // rekurzivní spojení: přidáváme funkci do seznamu
            new_node->children_count = kids[1]->children_count + 1;
            new_node->children = malloc(sizeof(AstNode *) * new_node->children_count);
            if (!new_node->children) {
                printError(INTERNAL_ERROR, "AST_MAIN_CLASS_BLOCK: malloc selhal");
                memorySafeExit(INTERNAL_ERROR);
            }
    
            memcpy(new_node->children, kids[1]->children, sizeof(AstNode *) * kids[1]->children_count);
            new_node->children[new_node->children_count - 1] = kids[0]; // přidáme novou funkci
    
            free(kids[1]->children);
            ast_free_single(kids[1]);
        } else if (n == 0) {
            // ε — prázdný blok
            new_node->children = NULL;
            new_node->children_count = 0;
        } else {
            printError(SYNTAX_ERROR, "AST_MAIN_CLASS_BLOCK: neplatný počet dětí");
            memorySafeExit(SYNTAX_ERROR);
        }
    }
    /*
     * === if else podminky ===
     * If -> TOKEN_KEYWORD_IF TOKEN_LPAREN Expr TOKEN_RPAREN Block Else
     */
    if (kind == AST_IF) {
        new_node->data.If.condition = kids[2];
        new_node->data.If.if_block = kids[4];
        new_node->data.If.else_block = kids[5];
        ast_free_single(kids[0]); // IF
        ast_free_single(kids[1]); // (
        ast_free_single(kids[3]); // )
    }

    /*
     * === Cykly For a While ===
     * While -> TOKEN_KEYWORD_WHILE TOKEN_LPAREN Expr TOKEN_RPAREN Block
     */
    if (kind == AST_WHILE) {
        new_node->data.While.condition = kids[2];
        new_node->data.While.block = kids[4];
        ast_free_single(kids[0]); // WHILE
        ast_free_single(kids[1]); // (
        ast_free_single(kids[3]); // )
    }

    /*
     * === Cykly For a While ===
     * For -> TOKEN_KEYWORD_FOR TOKEN_LPAREN TOKEN_IDENTIFIER TOKEN_KEYWORD_IN Expr TOKEN_RPAREN Block
     */
    if (kind == AST_FOR) {
        new_node->value.identifier_name = strdup(kids[2]->value.identifier_name);
        new_node->data.For.container_expr = kids[4];
        new_node->data.For.block = kids[6];
        ast_free_single(kids[0]); // FOR
        ast_free_single(kids[1]); // (
        ast_free_single(kids[3]); // IN
        ast_free_single(kids[5]); // )
        ast_free_single(kids[2]); // IDENTIFIER
    }

    /*
     * === Return pro EXTSTAT ===
     * Return -> TOKEN_KEYWORD_RETURN Return_expr
     * Return_expr -> Expr | eps
     */
    if (kind == AST_RETURN) {
        new_node->data.Return.return_expr = kids[1];
        ast_free_single(kids[0]); // RETURN
    }

    /*
     * === Deklarace výrazu ===
     * Var -> TOKEN_KEYWORD_VAR TOKEN_IDENTIFIER Var_EXT
     * Var_EXT -> TOKEN_ASSIGN Expr | eps
     */
    if (kind == AST_VAR) {
        new_node->value.identifier_name = strdup(kids[1]->value.identifier_name);
        new_node->data.Var.var_ext = kids[2];
        ast_free_single(kids[0]); // VAR
        ast_free_single(kids[1]); // IDENTIFIER
    }

    /*
     * === Všeobecné prirazeni ===
     * Call_assign -> TOKEN_IDENTIFIER Call_assign_list
     * Call_assign_list -> TOKEN_ASSIGN Expr | TOKEN_LPAREN Var_arg TOKEN_RPAREN
     * Global_assign -> TOKEN_GLOBAL_ID TOKEN_ASSIGN Expr
     */
    if (kind == AST_ASSIGN) {
        new_node->data.Assign.target_var = kids[0];
        new_node->data.Assign.expr = kids[2];
        ast_free_single(kids[1]); // TOKEN_ASSIGN
    }

    /*
     * === Všeobecný block a jeho obsah ===
     * Block -> TOKEN_BLOCK_START Block_content TOKEN_BLOCK_END TOKEN_NEW_LINE
     */
    if (kind == AST_BLOCK) {
        new_node->children = kids[1]->children;
        new_node->children_count = kids[1]->children_count;
        ast_free_single(kids[0]);
        ast_free_single(kids[2]);
        ast_free_single(kids[3]);
        ast_free_single(kids[1]);
    }

    /*
     * === Cykly For a While ===
     * Break -> TOKEN_KEYWORD_BREAK
     * Continue -> TOKEN_KEYWORD_CONTINUE
     * === Výrazy ===
     * Expr -> precedenčná analýza
     */
    if (kind == AST_BREAK || kind == AST_CONTINUE || kind == AST_EXPR) {
        // Jednoduché uzly bez dětí
    }
    /*
     * === Prolog ===
     * Prolog -> TOKEN_KEYWORD_PACKAGE TOKEN_IDENTIFIER TOKEN_NEW_LINE TOKEN_KEYWORD_STATIC Main_class
     */
    if (kind == AST_PROLOG) {
        new_node->value.identifier_name = strdup(kids[1]->value.identifier_name);
        ast_free_single(kids[0]); // PACKAGE
        ast_free_single(kids[2]); // NEW_LINE
        ast_free_single(kids[3]); // STATIC
    }

    /*
     * === Getter ===
     * Getter -> TOKEN_KEYWORD_STATIC TOKEN_IDENTIFIER TOKEN_ASSIGN TOKEN_LPAREN TOKEN_RPAREN Block
     */
    if (kind == AST_GETTER) {
        new_node->value.identifier_name = strdup(kids[1]->value.identifier_name);
        new_node->data.Getter.block = kids[5];
        ast_free_single(kids[0]); // STATIC
        ast_free_single(kids[2]); // ASSIGN
        ast_free_single(kids[3]); // (
        ast_free_single(kids[4]); // )
        ast_free_single(kids[1]); // IDENTIFIER
    }

    /*
     * === Setter ===
     * Setter -> TOKEN_KEYWORD_STATIC TOKEN_IDENTIFIER TOKEN_ASSIGN TOKEN_LPAREN TOKEN_IDENTIFIER TOKEN_RPAREN Block
     */
    if (kind == AST_SETTER) {
        new_node->value.identifier_name = strdup(kids[1]->value.identifier_name);
        new_node->data.Setter.param_name = strdup(kids[4]->value.identifier_name);
        new_node->data.Setter.block = kids[6];
        ast_free_single(kids[0]); // STATIC
        ast_free_single(kids[2]); // ASSIGN
        ast_free_single(kids[3]); // (
        ast_free_single(kids[5]); // )
        ast_free_single(kids[1]); // IDENTIFIER
        ast_free_single(kids[4]); // PARAMETER IDENTIFIER
    }

    /*
     * === Parametr ===
     * Param -> TOKEN_IDENTIFIER
     */
    if (kind == AST_PARAM) {
        new_node->value.identifier_name = strdup(kids[0]->value.identifier_name);
        ast_free_single(kids[0]);
    }

    /*
     * === Volání funkce ===
     * Func_call -> TOKEN_IDENTIFIER TOKEN_LPAREN Arg_list TOKEN_RPAREN
     */
    if (kind == AST_FUNC_CALL) {
        new_node->value.identifier_name = strdup(kids[0]->value.identifier_name);
        new_node->children = kids[2]->children; // Argumenty
        new_node->children_count = kids[2]->children_count;
        ast_free_single(kids[0]); // IDENTIFIER
        ast_free_single(kids[1]); // (
        ast_free_single(kids[3]); // )
        ast_free_single(kids[2]); // Arg_list node
    }
    /*
     * === Seznam parametrů ===
     * Param_list -> Param_list TOKEN_COMMA Param | Paramo
     *Pravidlo pro volani funkce s vice parametry: (a,b,c)
     * postupne z nich vytvarime z nich strom
     *prvne potkame a n = 1 ulozime do kids[0]
     *druhe potkame a , b tim padem n = 3 zkopirujeme a a vytvorime a b
     *treti potakme a b , c tim padem n = 3 zkopirujeme a vytvorime a b c
     */
     if (kind == AST_PARAM_LIST) {
         if (n == 3) {
             new_node->children_count = kids[0]->children_count + 1;
             new_node->children = malloc(sizeof(AstNode *) * new_node->children_count);
             memcpy(new_node->children, kids[0]->children, sizeof(AstNode *) * kids[0]->children_count);

             new_node->children[new_node->children_count - 1] = kids[2];

             free(kids[0]->children);

             ast_free_single(kids[0]);
             ast_free_single(kids[1]);

         } else if(n ==1){ // n == 1

             new_node->children_count = 1;
             new_node->children = malloc(sizeof(AstNode *));
             new_node->children[0] = kids[0];
         }else{
        	printError(SEMANTICS_ERROR_UNEXPECTED,"Spatny pocet parametru");
         	memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
         }
     }

    /*
     * === Seznam příkazů ===
     * Statement_list -> Statement_list Statement | Statement
     *prvne potkame a n = 1 ulozime do kids[0]
     *druhe potkame a , b tim padem n = 3 zkopirujeme a a vytvorime a b
     */
    if (kind == AST_STATEMENT_LIST) {
        if(n == 2){
        	new_node->children_count = kids[0]->children_count+1;
         	new_node->children = malloc(sizeof(AstNode *) * new_node->children_count);
          	memcpy(new_node->children, kids[0]->children, sizeof(AstNode *) * kids[0]->children_count);
          	new_node->children[new_node->children_count - 1] = kids[1];
           	free(kids[0]->children);
            ast_free_single(kids[0]);
        }else if(n== 1){
        new_node->children_count = 1;
        new_node->children = malloc(sizeof(AstNode *));
        new_node->children[0] = kids[0];
        }else {
       	printError(SEMANTICS_ERROR_UNEXPECTED,"Spatny pocet parametru");
        memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }
    }
    /*
     * === Statement ===
     * Statement -> Expr | Assign | Return | If | While | For | ε
     *
     * Obaluje jeden příkaz nebo výraz (pokud existuje).
     * Pokud je n == 0 → jde o prázdný příkaz (ε).
     */
    if (kind == AST_STATEMENT) {
        if (n == 1) {
            new_node->children_count = 1;
            new_node->children = malloc(sizeof(AstNode *));
            if (new_node->children == NULL) {
                printError(INTERNAL_ERROR, "AST_STATEMENT: malloc selhal");
                memorySafeExit(INTERNAL_ERROR);
            }
            new_node->children[0] = kids[0];
        } else if (n == 0) {
            new_node->children = NULL;
            new_node->children_count = 0;
        } else {
            printError(SYNTAX_ERROR, "AST_STATEMENT: neočekávaný počet dětí");
            memorySafeExit(SYNTAX_ERROR);
        }
    }
    /*
     * === Literál: Integer ===
     * Int_lit -> TOKEN_INT
     */
     if (kind == AST_STRING_LIT) {
         new_node->value.string_value = strdup(kids[0]->value.string_value);
         ast_free_single(kids[0]);
     }

     /*
      * === Literál: Integer ===
      * Int_lit -> TOKEN_INT
      */
     if (kind == AST_INT_LIT) {
         new_node->value.int_value = kids[0]->value.int_value;
         ast_free_single(kids[0]);
     }

     /*
      * === Literál: Float ===
      * Float_lit -> TOKEN_FLOAT
      */
     if (kind == AST_FLOAT_LIT) {
         new_node->value.float_value = kids[0]->value.float_value;
         ast_free_single(kids[0]);
     }
    return new_node;
    }
    void ast_traverse(AstNode *node, int depth) {
        if (node == NULL) return;

        // Odsazení podle úrovně
        for (int i = 0; i < depth; i++) printf("  ");

        // Výpis typu uzlu
        printf("Node type: %s\n",ast_node_type_to_string(node->type));

        // Rekurze podle typu uzlu
        switch (node->type) {
            case AST_PROGRAM:
                ast_traverse(node->data.Program.prolog, depth + 1);
                ast_traverse(node->data.Program.main_class, depth + 1);
                break;

            case AST_FUNC_DEC:
                if (node->value.identifier_name){
                    for (int i = 0; i < depth + 1; i++) printf("  ");
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
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("For iterator: %s\n", node->value.identifier_name);
                ast_traverse(node->data.For.container_expr, depth + 1);
                ast_traverse(node->data.For.block, depth + 1);
                break;

            case AST_BLOCK:
            case AST_STATEMENT_LIST:
            case AST_PARAM_LIST:
            case AST_MAIN_CLASS:
                for (int i = 0; i < node->children_count; i++)
                    ast_traverse(node->children[i], depth + 1);
                break;

            case AST_VAR:
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("Var: %s\n", node->value.identifier_name);
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
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("String: \"%s\"\n", node->value.string_value);
                break;

            case AST_INT_LIT:
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("Int: %lld\n", node->value.int_value);
                break;

            case AST_FLOAT_LIT:
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("Float: %f\n", node->value.float_value);
                break;

            case AST_BREAK:
            case AST_CONTINUE:
                for (int i = 0; i < depth + 1; i++) printf("  ");
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
    switch(kid->type) {
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
    }
    new_node->type = k;
    new_node->children = NULL;
    new_node->children_count = 0;
    if (k == AST_INT_LIT) {
        new_node->value.int_value = strtoll(t.value.identifier, NULL, 10);
        free(t.value.identifier);
    } else if (k == AST_FLOAT_LIT) {
        new_node->value.float_value = strtod(t.value.identifier, NULL);
        free(t.value.identifier);
    } else if (k == AST_STRING_LIT) {
        new_node->value.identifier_name = strdup(t.value.identifier);
    }

    return new_node;
}
char *ast_node_type_to_string(AstNodeType type) {
    switch (type) {
        case AST_PROGRAM:         return "AST_PROGRAM";
        case AST_PROLOG:          return "AST_PROLOG";
        case AST_MAIN_CLASS:      return "AST_MAIN_CLASS";
        case AST_FUNC_DEC:        return "AST_FUNC_DEC";
        case AST_GETTER:          return "AST_GETTER";
        case AST_SETTER:          return "AST_SETTER";
        case AST_BLOCK:           return "AST_BLOCK";
        case AST_VAR:             return "AST_VAR";
        case AST_ASSIGN:          return "AST_ASSIGN";
        case AST_IF:              return "AST_IF";
        case AST_WHILE:           return "AST_WHILE";
        case AST_FOR:             return "AST_FOR";
        case AST_RETURN:          return "AST_RETURN";
        case AST_BREAK:           return "AST_BREAK";
        case AST_CONTINUE:        return "AST_CONTINUE";
        case AST_FUNC_CALL:       return "AST_FUNC_CALL";
        case AST_PARAM:           return "AST_PARAM";
        case AST_PARAM_LIST:      return "AST_PARAM_LIST";
        case AST_EXPR:            return "AST_EXPR";
        case AST_STATEMENT_LIST:  return "AST_STATEMENT_LIST";
        case AST_STRING_LIT:      return "AST_STRING_LIT";
        case AST_INT_LIT:         return "AST_INT_LIT";
        case AST_FLOAT_LIT:       return "AST_FLOAT_LIT";
        default:                  return "UNKNOWN_AST_NODE";
    }
}
