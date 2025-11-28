/** 
 * Fakulta informačných technologií VUT Brno
 * 
 * @brief súbor pre implementáciu abstraktného syntaktického strom, 
 * pre účely implementácie syntaktickej analýzy, pre projekt do predmetu IFJ
 * @file ast.c
 * @author: Jakub Jambrozi (xjambrj00)
 */
#include "ast.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support_functions.h"


AstNode *buildIfChainFlat(size_t n, AstNode **kids) {
    // ε
    if (n == 0) {
        AstNode *node = calloc(1, sizeof *node);
        if (!node) {
            printError(INTERNAL_ERROR, "alloc IF_CHAIN(0)");
            memorySafeExit(INTERNAL_ERROR);
        }
        node->kind  = AST_IF_CHAIN;
        node->arity = 0;
        return node;
    }

    if (n == 2 && kids[1] && kids[1]->kind == AST_EMPTY) {
        AstNode *flat1[1] = {kids[0]};

        astFreeWhole(kids[1]);
        return astNew(AST_IF_CHAIN, 1, flat1);
    }

    if (n == 2 && kids[1] && kids[1]->kind == AST_IF_CHAIN) {
        size_t    tail = kids[1]->arity;
        AstNode **flat = malloc(sizeof(AstNode *) * (tail + 1));
        if (!flat) {
            printError(INTERNAL_ERROR, "alloc ifchain flat");
            memorySafeExit(INTERNAL_ERROR);
        }

        flat[0] = kids[0];
        if (tail)
            memcpy(&flat[1], kids[1]->child, sizeof(AstNode *) * tail);

        AstNode *node = astNew(AST_IF_CHAIN, tail + 1, flat);
        free(flat);

        free(kids[1]->child);
        free(kids[1]);

        bool seen_else = false;
        for (size_t i = 0; i < node->arity; ++i) {
            AstNode *ch = node->child[i];
            if (ch && ch->kind == AST_ELSE) {
                if (seen_else || i != node->arity - 1) {
                    printError(SYNTAX_ERROR, "Else je duplicitné alebo nie je posledné");
                    memorySafeExit(SYNTAX_ERROR);
                }
                seen_else = true;
            } else if (seen_else) {
                printError(SYNTAX_ERROR, "Po 'else' už nemôžu nasledovať ďalšie vetvy");
                memorySafeExit(SYNTAX_ERROR);
            }
        }
        return node;
    }


    return astNew(AST_IF_CHAIN, n, kids);
}

AstNode *buildClassBlockFlat(size_t n, AstNode **kids) {
    if (n == 0) {
        AstNode *node = (AstNode *)calloc(1, sizeof(AstNode));
        if (!node) {
            printError(INTERNAL_ERROR, "alloc MAIN_CLASS_BLOCK(0)");
            memorySafeExit(INTERNAL_ERROR);
        }
        node->kind  = AST_MAIN_CLASS_BLOCK;
        node->arity = 0;
        return node;
    }

    if (n == 2 && kids[1] && kids[1]->kind == AST_MAIN_CLASS_BLOCK) {
        size_t    tail = kids[1]->arity;
        AstNode **flat = (AstNode **)malloc(sizeof(AstNode *) * (tail + 1));
        if (!flat) {
            printError(INTERNAL_ERROR, "alloc classblock flat");
            memorySafeExit(INTERNAL_ERROR);
        }
        flat[0] = kids[0];
        if (tail)
            memcpy(&flat[1], kids[1]->child, sizeof(AstNode *) * tail);

        AstNode *node = astNew(AST_MAIN_CLASS_BLOCK, tail + 1, flat);
        free(flat);

        free(kids[1]->child);
        free(kids[1]);
        return node;
    }

    return astNew(AST_MAIN_CLASS_BLOCK, n, kids);
}

AstNode *buildStmtListFlat(size_t n, AstNode **kids) {
    if (n == 0) {  // ε
        AstNode *node = calloc(1, sizeof *node);
        if (!node) {
            printError(INTERNAL_ERROR, "alloc STMT_LIST(0)");
            memorySafeExit(INTERNAL_ERROR);
        }
        node->kind  = AST_STATEMENT_LIST;
        node->arity = 0;
        return node;
    }
    if (n == 2 && kids[1] && kids[1]->kind == AST_STATEMENT_LIST) {
        size_t    tail = kids[1]->arity;
        AstNode **flat = malloc(sizeof(AstNode *) * (tail + 1));
        if (!flat) {
            printError(INTERNAL_ERROR, "alloc stmtlist flat");
            memorySafeExit(INTERNAL_ERROR);
        }
        flat[0] = kids[0];
        if (tail)
            memcpy(&flat[1], kids[1]->child, sizeof(AstNode *) * tail);
        AstNode *node = astNew(AST_STATEMENT_LIST, tail + 1, flat);
        free(flat);
        free(kids[1]->child);
        free(kids[1]);
        return node;
    }
    return astNew(AST_STATEMENT_LIST, n, kids);
}


AstNode *buildParamFlat(size_t n, AstNode **kids) {

    if (n == 0) {
        AstNode *node = (AstNode *)calloc(1, sizeof(AstNode));
        if (!node) {
            printError(INTERNAL_ERROR, "alloc PARAM(0)");
            memorySafeExit(INTERNAL_ERROR);
        }
        node->kind  = AST_PARAM;
        node->arity = 0;
        return node;
    }


    if (n == 2 && kids[0] && kids[1] && kids[1]->kind == AST_PARAM) {
        size_t tail = kids[1]->arity;

        AstNode **flat = (AstNode **)malloc(sizeof(AstNode *) * (tail + 1));
        if (!flat) {
            printError(INTERNAL_ERROR, "alloc param flat");
            memorySafeExit(INTERNAL_ERROR);
        }

        flat[0] = kids[0]; 
        if (tail > 0) {

            memcpy(&flat[1], kids[1]->child, sizeof(AstNode *) * tail);
        }


        AstNode *node = astNew(AST_PARAM, tail + 1, flat);
        free(flat);


        free(kids[1]->child);
        free(kids[1]);

        return node;
    }


    return astNew(AST_PARAM, n, kids);
}

AstNode *astNew(AstNodeType kind, size_t arity, AstNode **kids) {
    AstNode *new_node = (AstNode *)calloc(1, sizeof(AstNode));
    if (!new_node) {
        printError(INTERNAL_ERROR, "Chyba alokace: new_node");
        memorySafeExit(INTERNAL_ERROR);
    }
    new_node->tmpId = 0;

    new_node->kind  = kind;
    new_node->arity = arity;

    if (arity > 0) {
        new_node->child = (AstNode **)malloc(sizeof(AstNode *) * arity);
        if (!new_node->child) {
            printError(INTERNAL_ERROR, "Chyba alokácie detí AST uzla\n");
            memorySafeExit(INTERNAL_ERROR);
        }
        for (size_t i = 0; i < arity; ++i) {
            new_node->child[i] = kids[i];
        }
    }
    return new_node;
}

AstNode *newLeaf(AstNodeType kind, Token token) {
    AstNode *newLeaf = (AstNode *)calloc(1, sizeof(AstNode));
    if (!newLeaf) {
        printError(INTERNAL_ERROR, "Chyba alokácie AST listu\n");
        memorySafeExit(INTERNAL_ERROR);
    }
    if (kind == AST_INT_LIT_EXP) {
        newLeaf->kind = AST_INT_LIT;
    } else if (kind == AST_FLOAT_LIT_EXP) {
        newLeaf->kind = AST_FLOAT_LIT;
    } else if (kind == AST_STRING_BLOCK)
        newLeaf->kind = AST_STRING_LIT;
    else {
        newLeaf->kind = kind;
    }
    newLeaf->isLeaf = true;
    astValueFromToken(&newLeaf->value, kind, &token);
    if (newLeaf->value.floatVal && newLeaf->kind == AST_INT_LIT) {
        newLeaf->kind = AST_FLOAT_LIT;
    }
    return newLeaf;
}

void astFreeWhole(AstNode *node) {
    if (!node) {
        return;
    }
    if (node->isLeaf) {
        if (node->value.hasStr || node->value.isBool) {
            free(node->value.str);
            node->value.str = NULL;
        }
        free(node);
        return;
    }
    for (size_t i = 0; i < node->arity; ++i) {
        astFreeWhole(node->child[i]);
    }
    free(node->child);
    free(node);
}

void astValueFromToken(AstValue *out, AstNodeType kind, Token *token) {
    memset(out, 0, sizeof(*out));
    switch (kind) {
        case AST_INT_LIT:
            out->isInt  = true;
            out->intVal = strtoll(token->value.identifier, NULL, 0);
            break;
        case AST_FLOAT_LIT:
            out->isFloat  = true;
            out->floatVal = strtod(token->value.identifier, NULL);
            break;
        case AST_STRING_LIT:
            out->hasStr = true;
            char *tmp   = stringChangingLit(token->value.identifier);
            out->str    = tmp;
            break;
        case AST_STRING_BLOCK:
            out->hasStr = true;
            char *temp  = stringChanging(token->value.identifier);
            out->str    = temp;
            break;
        case AST_IDENTIFIER:
        case AST_GLOBAL_ID:
            out->hasStr = true;
            out->str    = dupCstr(token->value.identifier);
            break;
        case AST_BOOL_LIT:
            out->isBool = true;
            out->str    = dupCstr(token->value.identifier);
            break;

        case AST_INT_LIT_EXP: {
            double    val  = floatExp(token->value.identifier);
            long long ival = (long long)val;
            if (val == (double)ival) {
                out->isInt  = true;
                out->intVal = ival;
            }

            else {
                out->isFloat  = true;
                out->floatVal = val;
            }
            break;
        }

        case AST_FLOAT_LIT_EXP:
            out->isFloat  = true;
            out->floatVal = floatExp(token->value.identifier);
            break;
        default:
            break;
    }
}

char *dupCstr(const char *string) {
    if (!string) {
        return NULL;
    }
    size_t len       = strlen(string) + 1;
    char  *newString = (char *)malloc(len);
    if (!newString) {
        printError(INTERNAL_ERROR, "Chyba alokácie (dupCstr)\n");
        memorySafeExit(INTERNAL_ERROR);
    }
    memcpy(newString, string, len);
    return newString;
}

double floatExp(const char *s) {
    char *pos = strpbrk(s, "eE");
    if (!pos) {
        return strtod(s, NULL);
    }
    char base_str[32];
    strncpy(base_str, s, pos - s);
    base_str[pos - s] = '\0';

    double base = strtod(base_str, NULL);
    double exp  = strtod(pos + 1, NULL);
    return pow(base, exp);
}

const char *astNodeTypeToString(AstNodeType kind) {
    switch (kind) {
        case AST_PROGRAM:
            return "PROGRAM";
        case AST_PROLOG:
            return "PROLOG";
        case AST_MAIN_CLASS:
            return "MAIN_CLASS";
        case AST_MAIN_CLASS_BLOCK:
            return "MAIN_CLASS_BLOCK";
        case AST_BLOCK:
            return "BLOCK";
        case AST_STATEMENT_LIST:
            return "STATEMENT_LIST";
        case AST_FUNC_DEC:
            return "FUNC_DEC";
        case AST_PARAM:
            return "PARAM";
        case AST_FUNC_CALL:
            return "FUNC_CALL";
        case AST_VAR:
            return "VAR";
        case AST_ASSIGN:
            return "ASSIGN";
        case AST_IF:
            return "IF";
        case AST_WHILE:
            return "WHILE";
        case AST_FOR:
            return "FOR";
        case AST_BREAK:
            return "BREAK";
        case AST_CONTINUE:
            return "CONTINUE";
        case AST_RETURN:
            return "RETURN";
        case AST_IDENTIFIER:
            return "IDENTIFIER";
        case AST_GLOBAL_ID:
            return "GLOBAL_ID";
        case AST_INT_LIT:
            return "INT_LIT";
        case AST_FLOAT_LIT:
            return "FLOAT_LIT";
        case AST_STRING_LIT:
            return "STRING_LIT";
        case AST_PLUS:
            return "STRING_PLUS";
        case AST_MINUS:
            return "STRING_MINUS";
        case AST_MUL:
            return "STRING_MUL";
        case AST_DIV:
            return "STRING_DIV";
        case AST_GT:
            return "STRING_GT";
        case AST_LT:
            return "STRING_LT";
        case AST_GTE:
            return "STRING_GTE";
        case AST_LTE:
            return "STRING_LTE";
        case AST_EQ:
            return "STRING_EQ";
        case AST_NEQ:
            return "STRING_NEQ";  // !=
        case AST_LPAREN:
            return "STRING_LPAREN";
        case AST_RPAREN:
            return "STRING_RPAREN";
        case AST_NOT:
            return "STRING_NEGACE";
        case AST_OR:
            return "STRING_OR";
        case AST_EMPTY:
            return "EMPTY";
        case AST_NULL:
            return "NULL";
        case AST_IFJ_CALL:
            return "IFJ_CALL";
        case AST_ELSE:
            return "ELSE";
        case AST_IF_CHAIN:
            return "IF CHAIN";
        case AST_DOUBLE_DOT:
            return "DOUBLE_DOT";
        case AST_TRIPLE_DOT:
            return "TRIPLE_DOT";
        case AST_BOOL_LIT:
            return "BOOL_LIT";
        case AST_KEYWORD_NUM:
            return "KEYWORD_NUM";
        case AST_KEYWORD_STRING:
            return "KEYWORD_STRING";
        case AST_IS:
            return "STRING_IS";
        case AST_GETTER:
            return "GETTER";
        case AST_SETTER:
            return "SETTER";
        case AST_CONDITION:
            return "CONDITION";
        case AST_TERNARY:
            return "TERNARY_OPERATOR";
        case AST_AND:
            return "STRING_AND";
        case AST_SMALL_NULL:
            return "SMALL_NULL";
        case AST_GETTER_CALL:
            return "GETTER_CALL";
        case AST_SETTER_CALL:
            return "SETTER_CALL";
        default:
            return "UNKNOWN_AST_NODE";
    }
}

#ifdef ENABLE_DEVELOPMENT_LOGGING
static void ast_dump_rec(const AstNode *n, int depth, const AstNode *parent) {
    if (!n) {
        fprintf(stderr, "%*s(null)\n", depth * 2, "");
        return;
    }
    if (n == parent) {
        fprintf(stderr, "%*s[CYCLE DETECTED: node %p refers to itself]\n", depth * 2, "", (void *)n);
        return;
    }

    fprintf(stderr, "%*s[%s]", depth * 2, "", astNodeTypeToString(n->kind));

    if (n->isLeaf) {
        if ((n->value.hasStr || n->value.isBool) && n->value.str)
            fprintf(stderr, " (str=%s)", n->value.str);
        else if (n->value.isInt)
            fprintf(stderr, " (int=%lld)", n->value.intVal);
        else if (n->value.isFloat)
            fprintf(stderr, " (float=%g)", n->value.floatVal);
    }

    fprintf(stderr, "\n");

    for (size_t i = 0; i < n->arity; ++i) {
        ast_dump_rec(n->child[i], depth + 1, n);
    }
}

void astDump(const AstNode *root) {
    fprintf(stderr, "===== AST STROM =====\n");
    ast_dump_rec(root, 0, NULL);
}
#else
void astDump(const AstNode *root) {
    printLog("Vypisuji strom s korenem %p\n", root);
    return;
}
#endif

char *stringChanging(char *tmp) {
    if (!tmp) {
        return NULL;
    }
    size_t len = strlen(tmp);
    char *out = malloc(len * 4 + 1);
    if (!out) {
        printError(INTERNAL_ERROR, "chyba v alokacii");
        memorySafeExit(INTERNAL_ERROR);
    }

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (tmp[i] <= 32 || tmp[i] == 35 || tmp[i] == 92) {
            j += sprintf(out + j, "\\%03u", tmp[i]);

        } else {
            out[j] = tmp[i];
            j++;
        }
    }

    out[j] = '\0';

    return out;
}

char *stringChangingLit(char *tmp) {
    if (!tmp) {
        return NULL;
    }
    char *unesc = escapeString(tmp);
    char *out = stringChanging(unesc);

    free(unesc);
    return out;
}

char *escapeString(char *tmp) {
    size_t len = strlen(tmp);
    char  *out = malloc(len + 1);
    if (!out) {
        printError(INTERNAL_ERROR, "chyba v alokacii");
        memorySafeExit(INTERNAL_ERROR);
    }

    size_t i = 0, j = 0;
    while (i < len) {
        if (tmp[i] == '\\' && i + 1 < len) {
            char c = tmp[i + 2];

            if (c == 'n') {
                out[j++] = '\n';
                i += 2;
            } else if (c == 't') {
                out[j++] = '\t';
                i += 2;
            } else if (c == '\\') {
                out[j++] = '\\';
                i += 2;
            } else if (c == '"') {
                out[j++] = '\"';
                i += 2;
            } else if (c == 'r') {
                out[j++] = '\r';
                i += 2;
            } else {
                out[j++] = tmp[i++];
            }
        } else {
            out[j++] = tmp[i++];
        }
    }

    out[j] = '\0';
    return out;
}
/*** konec ast.c ***/