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



AstNode *astNew(AstNodeType kind, size_t arity, AstNode **kids) {
    // alokácia nového uzla
    AstNode *new_node = (AstNode *)calloc(1, sizeof(AstNode));
    if (!new_node) {
        printError(INTERNAL_ERROR, "Chyba alokace: new_node");
        memorySafeExit(INTERNAL_ERROR);
    }

    // zápis predaných hodnôt do nového uzla
    new_node->tmpId = 0;
    // typ uzl
    new_node->kind  = kind;
    // počet detí koľko 
    new_node->arity = arity;

    // ak má mať uzol nejaké deti
    if (arity > 0) {
        // alokujeme miesto pre deti
        new_node->child = (AstNode **)malloc(sizeof(AstNode *) * arity);
        if (!new_node->child) {
            printError(INTERNAL_ERROR, "Chyba alokácie detí AST uzla\n");
            memorySafeExit(INTERNAL_ERROR);
        }
        // poradie detí je už pri volaní funkcie zoradené čiže iba skopírujeme deti z parametru do nového uzla
        for (size_t i = 0; i < arity; i++) {
            new_node->child[i] = kids[i];
        }
    }
    // vrátime nový uzol syntaktickej analýze
    return new_node;
}

AstNode *newLeaf(AstNodeType kind, Token token) {
    // alokácia nového listového uzla
    AstNode *newLeaf = (AstNode *)calloc(1, sizeof(AstNode));
    if (!newLeaf) {
        printError(INTERNAL_ERROR, "Chyba alokácie AST listu\n");
        memorySafeExit(INTERNAL_ERROR);
    }
    
    // priradenie typu listového uzla
    // ak je to nejaký typ, ktorý budeme počas volania prerábať, tak novému listovému uzlu priradíme už
    // typ uzlu po prerobení

    if (kind == AST_INT_LIT_EXP) {
        newLeaf->kind = AST_INT_LIT;
    } else if (kind == AST_FLOAT_LIT_EXP) {
        newLeaf->kind = AST_FLOAT_LIT;
    } else if (kind == AST_STRING_BLOCK)
        newLeaf->kind = AST_STRING_LIT;
    else {
        newLeaf->kind = kind;
    }
    // nastavíme že sa jedná o listový uzol
    newLeaf->isLeaf = true;
    // extrahuje konkrétnu hodnotu tokenu a zapíšeme ju do nového listového uzla
    astValueFromToken(&newLeaf->value, kind, &token);


    if (newLeaf->value.floatVal && newLeaf->kind == AST_INT_LIT) {
        newLeaf->kind = AST_FLOAT_LIT;
    }
    // vrátime novo vytvorený listový uzol
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
            // prepis reťazca ná long long 
            out->intVal = strtoll(token->value.identifier, NULL, 0);
            break;
        case AST_FLOAT_LIT:
            out->isFloat  = true;
            // prepis reťazca na double
            out->floatVal = strtod(token->value.identifier, NULL);
            break;
        case AST_STRING_LIT:
            out->hasStr = true;
            // prepis reťazca aby bol kompatibilný s jazykom wren
            char *tmp   = stringChangingLit(token->value.identifier);
            out->str    = tmp;
            break;
        case AST_STRING_BLOCK:
            out->hasStr = true;
            // prepis reťazca aby bol kompatibilný s jazykom wren
            char *temp  = stringChanging(token->value.identifier);
            out->str    = temp;
            break;
        case AST_IDENTIFIER:
        case AST_GLOBAL_ID:
            out->hasStr = true;
            // vloženie reťazca do abstraktného syntaktického stromu cez pomocnú funkciu
            // aby sa predišlo uvoľneniu hodnoty pri uvoľňovaní už nepotrebného tokenu
            out->str    = dupCstr(token->value.identifier);
            break;
        case AST_BOOL_LIT:
            out->isBool = true;
            // vloženie reťazca do abstraktného syntaktického stromu cez pomocnú funkciu
            // aby sa predišlo uvoľneniu hodnoty pri uvoľňovaní už nepotrebného tokenu
            out->str    = dupCstr(token->value.identifier);
            break;

        case AST_INT_LIT_EXP: {
            // prevedenie reťazca obsahujúceho číslo s exponentom na konkrétnu hodnotu 
            double    val  = floatExp(token->value.identifier);
            long long ival = (long long)val;
            //jedná sa o long long
            if (val == (double)ival) {
                out->isInt  = true;
                out->intVal = ival;
            }
            // jedná sa skutočne o double
            else {
                out->isFloat  = true;
                out->floatVal = val;
            }
            break;
        }

        case AST_FLOAT_LIT_EXP:
            out->isFloat  = true;
            // prevedenie reťazca obsahujúceho číslo s exponentom na konkrétnu hodnotu 
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
    // overenie či sa v reťazci nachádza exponent
    // zároveň zistíme na akej pozícii sa nachádza
    char *pos = strpbrk(s, "eE");
    // ak sa nenachádza iba prevedieme reťazec do očakávanej návratovej hodnoty
    if (!pos) {
        return strtod(s, NULL);
    }

    char baseStr[32];
    //skopírujeme do baseStr iba časť po pozíciu danej v pos, na ktorej sa nachádza exponent
    strncpy(baseStr, s, pos - s);
    // pridáme umelý  ukončovací znak
    baseStr[pos - s] = '\0';

    // prevedieme reťazec v baseStr do double
    double base = strtod(baseStr, NULL);
    // prevedieme exponent do double
    double exp  = strtod(pos + 1, NULL);

    // vrátime výsledne číslo
    return base * pow(10, exp);
}


char *stringChanging(char *tmp) {
    if (!tmp) {
        return NULL;
    }
    //získanie dĺžky reťazca
    size_t len = strlen(tmp);
    // alokujeme si miesto pre nový reťazec
    // pôvodný počet * 4 (\xyz = 4 znaky) + 1 (\0)
    char *out = malloc(len * 4 + 1);
    if (!out) {
        printError(INTERNAL_ERROR, "chyba v alokacii\n");
        memorySafeExit(INTERNAL_ERROR);
    }

    size_t j = 0;
    // prechádzanie každého znaku v pôvodnom reťazci
    for (size_t i = 0; i < len; i++) {
        // overenie o aký znak ide cez ascii kódy
        if (tmp[i] <= 32 || tmp[i] == 35 || tmp[i] == 92) {
            // zapíšeme do  nového reťazca znak vo formáte \xyz
            j += sprintf(out + j, "\\%03u", tmp[i]);

        } else {
            //inak zapíšeme do nového reťazca pôvodný znak
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
        printError(INTERNAL_ERROR, "chyba v alokacii\n");
        memorySafeExit(INTERNAL_ERROR);
    }

    size_t i = 0, j = 0;
    //prejdeme každý znak
    while (i < len) {
        // ak sa jedná o escape sekvenciu vložíme znak konkrétnej escape sekvencie
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

AstNode *buildIfChainFlat(size_t n, AstNode **kids) {
    // ak je nastavená nulová arita
    if (n == 0) {
        // alokácia nového uzla
        AstNode *node = calloc(1, sizeof *node);
        if (!node) {
            printError(INTERNAL_ERROR, "chyba alokácie\n");
            memorySafeExit(INTERNAL_ERROR);
        }
        //zápis a vrátenie uzla s jeho typom a aritou 0
        node->kind  = AST_IF_CHAIN;
        node->arity = 0;
        return node;
    }

    // ak sme na konci if reťazenia 
    if (n == 2 && kids[1] && kids[1]->kind == AST_EMPTY) {
        AstNode *flat1[1] = {kids[0]};

        astFreeWhole(kids[1]);
        return astNew(AST_IF_CHAIN, 1, flat1);
    }

    // nasleduje ďalšie if reťazenie, ktoré potrebujeme odstrániť a uložiť iba jeho deti
    if (n == 2 && kids[1] && kids[1]->kind == AST_IF_CHAIN) {
        size_t    tail = kids[1]->arity;
        AstNode **flat = malloc(sizeof(AstNode *) * (tail + 1));
        if (!flat) {
            printError(INTERNAL_ERROR, "chyba alokácie\n");
            memorySafeExit(INTERNAL_ERROR);
        }

        // ponecháme prvý IfChain uzol
        flat[0] = kids[0];
        // ak je arita druhého IFCHain > 0 => je potrebné ho uložiť
        if (tail)
            // skopírujeme do flat[1] iba deti druhého IF_CHAIN => samotný AST_IF_CHAIN vynecháme
            memcpy(&flat[1], kids[1]->child, sizeof(AstNode *) * tail);
        //vytvoríme nový uzol už bez AST_IF_CHAIN
        AstNode *node = astNew(AST_IF_CHAIN, tail + 1, flat);
        free(flat);

        free(kids[1]->child);
        free(kids[1]);

        bool seen_else = false;
        // syntaktická kontrola zaobchádzania s else vetvou
        for (size_t i = 0; i < node->arity; ++i) {
            AstNode *ch = node->child[i];
            // zistíme či sa jedná o else
            if (ch && ch->kind == AST_ELSE) {
                // ak sa jedná o else zistíme či sa už niekde nevyskytlo alebo nie je ako posledné dieťa if reťazenia 
                if (seen_else || i != node->arity - 1) {
                    printError(SYNTAX_ERROR, "Else je duplicitné alebo nie je posledné\n");
                    memorySafeExit(SYNTAX_ERROR);
                }
                seen_else = true;
                // kontrola prítomnosti ďalšej vetvy za else vetvou
            } else if (seen_else) {
                printError(SYNTAX_ERROR, "Po 'else' už nemôžu nasledovať ďalšie vetvy\n");
                memorySafeExit(SYNTAX_ERROR);
            }
        }
        return node;
    }


    return astNew(AST_IF_CHAIN, n, kids);
}

AstNode *buildClassBlockFlat(size_t n, AstNode **kids) {
    // ak je nastavená nulová arita
    if (n == 0) {
        AstNode *node = (AstNode *)calloc(1, sizeof(AstNode));
        if (!node) {
            printError(INTERNAL_ERROR, "chyba alokácie\n");
            memorySafeExit(INTERNAL_ERROR);
        }
        node->kind  = AST_MAIN_CLASS_BLOCK;
        node->arity = 0;
        return node;
    }

    // nasleduje ďalší MAIN_CLASS_BLOCK, ktorý potrebujeme odstrániť a uložiť iba jeho deti    
    if (n == 2 && kids[1] && kids[1]->kind == AST_MAIN_CLASS_BLOCK) {
        size_t    tail = kids[1]->arity;
        AstNode **flat = (AstNode **)malloc(sizeof(AstNode *) * (tail + 1));
        if (!flat) {
            printError(INTERNAL_ERROR, "chyba alokácie\n");
            memorySafeExit(INTERNAL_ERROR);
        }
        // ponecháme prvý IfChain uzol
        flat[0] = kids[0];
        if (tail)
            // skopírujeme do flat[1] iba deti druhého MAIN_CLASS_BLOKU => samotný AST_MAIN_CLASS_BLOCK vynecháme
            memcpy(&flat[1], kids[1]->child, sizeof(AstNode *) * tail);
        //vytvoríme nový uzol už bez AST_MAIN_CLASS_BLOCK
        AstNode *node = astNew(AST_MAIN_CLASS_BLOCK, tail + 1, flat);
        free(flat);

        free(kids[1]->child);
        free(kids[1]);
        return node;
    }

    return astNew(AST_MAIN_CLASS_BLOCK, n, kids);
}

AstNode *buildStmtListFlat(size_t n, AstNode **kids) {
    if (n == 0) { 
        AstNode *node = calloc(1, sizeof *node);
        if (!node) {
            printError(INTERNAL_ERROR, "chyba alokácie\n");
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
            printError(INTERNAL_ERROR, "chyba alokácie\n");
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
            printError(INTERNAL_ERROR, "chyba alokácie\n");
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
            printError(INTERNAL_ERROR, "chyba alokácie\n");
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


/*** konec ast.c ***/