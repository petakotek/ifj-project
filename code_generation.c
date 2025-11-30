#include "code_generation.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "builtin_functions.h"
#include "dynamic_stack.h"
#include "support_functions.h"
static size_t loopCounter = 0;
static size_t ifCounter   = 0;

void printN(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}
/**
 * @brief print na .IFJcode25 ktery je musi byt na zacatku
 */
void emitProlog() { printN(".IFJcode25"); }
/**
 * @brief Změní velikost bufferu pro jména labelů.
 *
 * @param prefix Buffer, který se má realokovat.
 * @param size Ukazatel na velikost, která se zdvojnásobí.
 * @return Ukazatel na nově alokovaný buffer.
 */
char *resizeLabelName(char *prefix, size_t *size) {
    *size *= 2;
    prefix = realloc(prefix, *size);
    if (prefix == NULL) {
        printError(INTERNAL_ERROR, "error in resize_label_reallock");
        memorySafeExit(INTERNAL_ERROR);
    }
    return prefix;
}
/**
 * @brief Generuje unikátní label na základě existujícího prefixu větve a nového bloku.
 *
 * Label je tvořen z řetězce existujících labelů ve větvi s připojením nového bloku.
 * Příklad: `fun$1$while$2` je label pro druhý `while` cyklus uvnitř funkce `fun`.
 *
 * @param funcName Jméno bloku (např. "fun", "if", "while").
 * @param funcStruckt Kontext funkce obsahující zásobník labelů (`labelStack`).
 * @param pridaniDoStacku Logická hodnota určující, zda se má vygenerovaný label uložit na zásobník.
 * @param globalCounter Globální čítač pro unikátní číslování aktuálního bloku.
 * @return Ukazatel na nově alokovaný řetězec obsahující výsledný label.
 */
char *generatorLable(char *funcName, FuncContext *funcStruckt, bool pridaniDoStacku, size_t globalCounter) {
    size_t sizeOfBuffer = 128;
    char  *resultLabel   = (char *)malloc(sizeOfBuffer);
    if (resultLabel == NULL) {
        printError(INTERNAL_ERROR, "Error allocating memory for label");
        memorySafeExit(INTERNAL_ERROR);
    }

    int currentLen = 0;

    // Získání existujícího prefixu ze zásobníku (pokud existuje)
    if (!stacklabelStackIsEmpty(funcStruckt->labelStack)) {
        char *prefix = stacklabelStackRead(funcStruckt->labelStack);
        if (prefix != NULL) {
            size_t prefixLen = strlen(prefix);

            if (prefixLen + 1 + 10 >= sizeOfBuffer) {
                resultLabel = resizeLabelName(resultLabel, &sizeOfBuffer);
                if (resultLabel == NULL) {
                    printError(INTERNAL_ERROR, "Error resizing label buffer");
                    memorySafeExit(INTERNAL_ERROR);
                }
            }
            if (prefixLen + 1 + 10 >= sizeOfBuffer) {
                resultLabel = resizeLabelName(resultLabel, &sizeOfBuffer);
                if (resultLabel == NULL) {
                    printError(INTERNAL_ERROR, "Error resizing label buffer");
                    memorySafeExit(INTERNAL_ERROR);
                }
            }

            currentLen = snprintf(resultLabel, sizeOfBuffer, "%s", prefix);
            if (currentLen < 0 || (size_t)currentLen >= sizeOfBuffer) {
                free(resultLabel);
                printError(INTERNAL_ERROR, "Error during initial label copy");
                memorySafeExit(INTERNAL_ERROR);
                return NULL;
            }
        }
    }

    // 2. Přidání nového bloku jako suffix: "$if$3"
    // Formát: $TypBloku$CisloBloku
    size_t remainingSpace = sizeOfBuffer - currentLen;
    int    written         = 0;
    if (funcStruckt->main == false) {
        written = snprintf(resultLabel + currentLen, remainingSpace, "$%s$%zu", funcName, globalCounter);
    } else if (!pridaniDoStacku) {
        written = snprintf(resultLabel + currentLen, remainingSpace, "$%s", funcName);
    } else {
        written = snprintf(resultLabel + currentLen, remainingSpace, "%s", funcName);
    }
    if (written <= 0 || (size_t)written >= remainingSpace) {
        // potřebna resize
        if ((size_t)written >= remainingSpace) {
            resultLabel = resizeLabelName(resultLabel, &sizeOfBuffer);
            if (resultLabel == NULL) {
                printError(INTERNAL_ERROR, "Error resizing label buffer (suffix)");
                memorySafeExit(INTERNAL_ERROR);
            }
            remainingSpace = sizeOfBuffer - currentLen;
            // znovu zapsani
            written = snprintf(resultLabel + currentLen, remainingSpace, "$%s$%zu", funcStruckt->funcName,
                               globalCounter);
        }

        if (written <= 0) {
            free(resultLabel);
            printError(INTERNAL_ERROR, "Error during suffix creation");
            memorySafeExit(INTERNAL_ERROR);
        }
    }
    currentLen += written;

    if (pridaniDoStacku) {
        // Uložení plného label na zásobník
        // pro funkce a if-else while-for pro dalsi zanoreni
        stacklabelStackPush(funcStruckt->labelStack, resultLabel);
    }
    // a jeho vraceni
    return resultLabel;
}
/**
 * @brief Generuje kód voláním emitCommand pro všechny potomky uzlu.
 *
 * @param root Uzel, jehož potomci se zpracovávají.
 * @param funcStruct Kontext funkce pro generování kódu.
 */
void emitChildren(AstNode *root, FuncContext *funcStruct) {
    for (size_t i = 0; i < root->arity; i++) {
        emitCommand(root->child[i], funcStruct);
    }
}
/*
 * @brief Funkce na zalozeni struktury na zapsani var promenych.
 */
VarList *varListCreate() {
    VarList *list   = malloc(sizeof(VarList));
    list->capacity  = 16;
    list->count     = 0;
    list->varNames = calloc(1, sizeof(char *) * list->capacity);
    return list;
}
/**
 * @brief Přidá jméno proměnné do seznamu (listu) s automatickou změnou velikosti.
 *
 * @param list Ukazatel na strukturu VarList, do které se má proměnná přidat.
 * @param name Ukazatel na řetězec obsahující jméno proměnné, které se má přidat.
 */
void varListAdd(VarList *list, char *name) {
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->varNames[i], name) == 0) {
            return;
        }
    }
    // az moc promenych resize
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->varNames = realloc(list->varNames, sizeof(char *) * list->capacity);
    }
    list->varNames[list->count] = strdup(name);
    list->count++;
}
/**
 * @brief Uvolní (zničí) seznam proměnných a veškerou alokovanou paměť.
 *
 * @param list Ukazatel na strukturu VarList, která má být zničena.
 */
void varListDescrution(VarList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->varNames[i]);
    }
    free(list->varNames);
    free(list);
}
/**
 * @brief Generuje kód pro všechny deklarované funkce.
 *
 * @param root Kořenový uzel AST obsahující deklarace funkcí.
 * @param funcContext Kontext funkce (proměnné, zásobníky atd.) pro generování kódu.
 */
void emitFunctions(AstNode *root, FuncContext *funcContext) {
    AstNode *main = NULL;
    for (size_t i = 0; i < root->arity; i++) {
        if (root->child[i]->kind == AST_FUNC_DEC && strcmp(root->child[i]->child[0]->value.str, "main$0") == 0) {
            main = root->child[i];
        } else {
            emitCommand(root->child[i], funcContext);
        }
    }
    emitCommand(main, funcContext);
}
/**
 * @brief Přidá do seznamu dočasných proměnných (tmps) potřebné pro binární operace.
 *
 * @param vars Ukazatel na seznam proměnných (VarList), kam se mají dočasné proměnné přidat.
 * @param func Jméno aktuální funkce pro prefix jména dočasné proměnné.
 * @param id Unikátní ID pro danou operaci.
 * @param node Ukazatel na uzel AST, který reprezentuje binární operaci.
 */
void addBinopTmps(VarList *vars, const char *func, int id, AstNode *node) {
    char buf[128];

    if (node->kind == AST_PLUS) {
        snprintf(buf, sizeof buf, "%s_ln_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_rn_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_ty_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_rs_%d", func, id);
        varListAdd(vars, buf);
    } else if (node->kind == AST_GT || node->kind == AST_LT || node->kind == AST_LTE || node->kind == AST_GTE ||
               node->kind == AST_IS) {
        snprintf(buf, sizeof buf, "%s_ln_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_rn_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_rty_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_lty_%d", func, id);
        varListAdd(vars, buf);
    } else if (node->kind == AST_NOT) {
        snprintf(buf, sizeof buf, "%s_ln_%d", func, id);
        varListAdd(vars, buf);
    } else if (node->kind == AST_MUL) {
        snprintf(buf, sizeof buf, "%s_ln_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_rn_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_rty_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_lty_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_rs_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_counter_%d", func, id);
        varListAdd(vars, buf);
    } else if (node->kind == AST_AND) {
        snprintf(buf, sizeof buf, "%s$and_false$%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s$and_end$%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_and_left_%d", func, id);
        varListAdd(vars, buf);
    } else if (node->kind == AST_OR) {
        snprintf(buf, sizeof buf, "%s$or_success$%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s$or_end$%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_or_left_%d", func, id);
        varListAdd(vars, buf);
    } else if (node->kind == AST_IDENTIFIER || node->kind == AST_GLOBAL_ID) {
        snprintf(buf, sizeof buf, "%s_ty_bool_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_val_bool_%d", func, id);
        varListAdd(vars, buf);
    } else {
        snprintf(buf, sizeof buf, "%s_ln_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_rn_%d", func, id);
        varListAdd(vars, buf);
        snprintf(buf, sizeof buf, "%s_ty_%d", func, id);
        varListAdd(vars, buf);
    }
}
/*
 *tohle odmitam komentovat za to se stydim
 */
void printIfj() {
    printN("DEFVAR LF@temp$var$readStr");
    printN("DEFVAR LF@temp$var$readNum");
    printN("DEFVAR LF@temp$var$readBool");
    printN("DEFVAR LF@temp$var$write");
    printN("DEFVAR LF@temp$var$writeType");
    printN("DEFVAR LF@temp$var$floorVal");
    printN("DEFVAR LF@temp$var$floorType");
    printN("DEFVAR LF@temp$var$strVal");
    printN("DEFVAR LF@temp$var$strType");
    printN("DEFVAR LF@temp$var$lengthVal");
    printN("DEFVAR LF@temp$var$lengthType");
    printN("DEFVAR LF@temp$var$lengthRet");
    printN("DEFVAR LF@temp$var$substringSin");
    printN("DEFVAR LF@temp$var$substringStart");
    printN("DEFVAR LF@temp$var$substringEnd");
    printN("DEFVAR LF@temp$var$substringSLen");
    printN("DEFVAR LF@temp$var$substringSout");
    printN("DEFVAR LF@temp$var$substringType");
    printN("DEFVAR LF@temp$var$substringCmpResult");
    printN("DEFVAR LF@temp$var$substringStemp");

    printN("DEFVAR LF@temp$var$strcmpS1");
    printN("DEFVAR LF@temp$var$strcmpS2");
    printN("DEFVAR LF@temp$var$strcmpType");
    printN("DEFVAR LF@temp$var$strcmpCmpResult");

    printN("DEFVAR LF@temp$var$ordStr");
    printN("DEFVAR LF@temp$var$ordNum");
    printN("DEFVAR LF@temp$var$ordType");
    printN("DEFVAR LF@temp$var$ordIsInt");
    printN("DEFVAR LF@temp$var$ordOut");

    printN("DEFVAR LF@temp$var$chrVal");
    printN("DEFVAR LF@temp$var$chrType");
    printN("DEFVAR LF@temp$var$chrIsInt");
}
/**
 * @brief Alokuje paměť a inicializuje strukturu kontextu funkce (FuncContext).
 *
 * @return Ukazatel na novou strukturu FuncContext.
 */
FuncContext *constuctorFuncContext() {
    FuncContext *funcStruct = malloc(sizeof(FuncContext));
    if (!funcStruct) {
        printError(INTERNAL_ERROR, "chyba v alokaci");
        memorySafeExit(INTERNAL_ERROR);
    }
    funcStruct->funcName     = NULL;
    funcStruct->labelStack   = NULL;
    funcStruct->labelEnd     = NULL;
    funcStruct->main          = false;
    funcStruct->valueCounter = 0;
    return funcStruct;
}

void destructorFuncContext(FuncContext *funcStruct) { free(funcStruct); }
/**
 * @brief Rekurzivně sbírá lokální proměnné a alokuje dočasné proměnné pro operace v AST.
 *
 * @param node Aktuální uzel AST.
 * @param vars Seznam lokálních proměnných k naplnění.
 * @param funcStruct Kontext funkce.
 */
void collectLocalVars(AstNode *node, VarList *vars, FuncContext *funcStruct) {
    if (node == NULL)
        return;
    for (size_t i = 0; i < node->arity; i++) {
        collectLocalVars(node->child[i], vars, funcStruct);
    }
    switch (node->kind) {
        case AST_VAR:
            if (node->child[0] && node->child[0]->kind == AST_IDENTIFIER) {
                varListAdd(vars, node->child[0]->value.str);
            }
            break;

        case AST_PARAM:
            break;
        case AST_IFJ_CALL:
        case AST_PLUS:
        case AST_MINUS:
        case AST_MUL:
        case AST_DIV:
        case AST_EQ:
        case AST_LT:
        case AST_GT:
        case AST_IS:
        case AST_LTE:
        case AST_GTE:
        case AST_AND:
        case AST_NOT:
        case AST_NEQ:
        case AST_TERNARY:
        case AST_GLOBAL_ID:
        case AST_IDENTIFIER:
        case AST_OR: {
            node->tmpId = funcStruct->valueCounter++;
            addBinopTmps(vars, funcStruct->funcName, node->tmpId, node);
            break;
        }
        default:
            break;
    }
}
/**
 * @brief Rekurzivně sbírá jména globálních proměnných z AST.
 *
 *
 * @param root Ukazatel na aktuální uzel AST pro rekurzivní průchod.
 * @param global Ukazatel na seznam proměnných (VarList), kam se mají globální proměnné přidat.
 */
void emitLookGlobal(AstNode *root, VarList *global) {
    if (!root)
        return;

    if (root->kind == AST_GLOBAL_ID && root->value.str != NULL) {
        varListAdd(global, root->value.str);
        return;
    }

    for (size_t i = 0; i < root->arity; i++)
        emitLookGlobal(root->child[i], global);
}
/**
 * @brief Generuje kód pro definici a inicializaci globálních proměnných.
 *
 * @param root Kořenový uzel AST celého programu.
 */
void emitGlobal(AstNode *root) {
    VarList *globals = varListCreate();
    emitLookGlobal(root, globals);
    for (int i = 0; i < globals->count; i++) {
        printN("DEFVAR GF@%s", globals->varNames[i]);
        printN("MOVE GF@%s nil@nil", globals->varNames[i]);
    }
    varListDescrution(globals);
}
/**
 * @brief Generuje kód pro návrat z funkce (return statement).
 *
 *
 * @param root Ukazatel na uzel AST reprezentující příkaz return.
 * @param funcStruct Kontext funkce (FuncContext) obsahující koncový label.
 */
void emitReturn(AstNode *root, FuncContext *funcStruct) {
    emitChildren(root, funcStruct);
    if (root->child[0]->kind == AST_EMPTY) {
        printN("PUSHS nil@nil");
    }
    printN("POPS LF@retval");
    printN("JUMP %s", funcStruct->labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro celou deklaraci funkce (funkční tělo).
 *
 * Funkce je zodpovědná za kompletní emitování funkčního těla
 * @param root Ukazatel na uzel AST (`AST_FUNC_DEC`), který reprezentuje celou deklaraci funkce.
 */
void emitFuncDec(AstNode *root) {
    DynamiclabelStackStackPtr labelStack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    bool                      main        = false;
    char                     *label       = NULL;
    size_t                    tmpCounter = 0;
    FuncContext              *context     = constuctorFuncContext();
    context->funcName                    = root->child[0]->value.str;
    context->labelEnd                    = NULL;
    context->labelStack                  = labelStack;
    if (strcmp(root->child[0]->value.str, "main$0") == 0 && root->child[1]->child[0]->kind == AST_EMPTY) {
        main          = true;
        context->main = true;
        label         = generatorLable(root->child[0]->value.str, context, true, tmpCounter);
        printN("LABEL %s", label);
    } else {
        label = generatorLable(root->child[0]->value.str, context, true, tmpCounter);
        printN("LABEL %s", label);
    }
    context->main      = false;
    char *labelEnd    = generatorLable("end", context, false, tmpCounter);
    context->labelEnd = labelEnd;
    printN("CREATEFRAME");
    printN("PUSHFRAME");
    printIfj();
    // sbirani lokalnich promenych na deklaraci
    VarList *localVars = varListCreate();
    varListAdd(localVars, "retval");
    collectLocalVars(root->child[2], localVars, context);  // funkce uvnitr
    for (int i = 0; i < localVars->count; i++) {
        printN("DEFVAR LF@%s", localVars->varNames[i]);
    }
    AstNode *params = root->child[1];
    if (params->child[0]->kind != AST_EMPTY) {
        for (size_t i = 0; i < params->arity - 1; i++) {
            const char *pname = params->child[i]->value.str;
            printN("DEFVAR LF@%s", pname);
            printN("POPS LF@%s", pname);
        }
    }
    printN("MOVE LF@retval nil@nil");
    emitChildren(root->child[2], context);
    printN("LABEL %s", labelEnd);
    if (!main) {
        printN("PUSHS LF@retval");
        printN("POPFRAME");
        printN("RETURN");
    } else {
        printN("EXIT int@0");
    }
    varListDescrution(localVars);
    while (!stacklabelStackIsEmpty(labelStack)) {
        char *item = stacklabelStackRead(labelStack);
        stacklabelStackPop(labelStack);
        free(item);
    }
    stacklabelStackDestruct(labelStack);
    destructorFuncContext(context);
    free(labelEnd);
}
/**
 * @brief Generuje IFJcode25 instrukce pro volání funkce.
 *
 * Funkce vytvoří nový dočasný rámec (`CREATEFRAME`), zpracuje argumenty
 * volání (parametry) a poté provede volání pomocí instrukce `CALL`.
 *
 * @param node Uzel AST reprezentující volání funkce.
 * @param funcStruct Kontex funkce (FuncContext) pro zpracování argumentů.
 */
void emit_call_func(AstNode *node, FuncContext *funcStruct) {
    printN("CREATEFRAME");
    for (size_t i = 0; i < node->child[1]->arity; i++) {
        emitCommand(node->child[1]->child[i], funcStruct);
    }
    printN("CALL $%s$0", node->child[0]->value.str);
}
/**
 * @brief Generuje IFJcode25 kód pro operaci sčítání/konkatenace (+).
 *
 * Zajišťuje typovou kontrolu a provádí sčítání pro čísla nebo konkatenaci pro řetězce.
 *
 * @param node Uzel AST pro operaci AST_PLUS.
 * @param funcStruct Kontext funkce pro dočasné proměnné a labely.
 */
void emitPlus(AstNode *node, FuncContext *funcStruct) {
    for (size_t i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], ty[128], rs[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStruct->funcName, id);
    snprintf(rs, sizeof rs, "%s_rs_%d", funcStruct->funcName, id);

    int  size_for_label = strlen(funcStruct->funcName) + 20;
    char labelOk[size_for_label], labelAdd[size_for_label], labelConcat[size_for_label], labelEnd[size_for_label],
        labelErr[size_for_label];
    snprintf(labelOk, sizeof labelOk, "%s$ok_add$%d", funcStruct->funcName, id);
    snprintf(labelAdd, sizeof labelAdd, "%s$add$%d", funcStruct->funcName, id);
    snprintf(labelConcat, sizeof labelConcat, "%s$concat$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$end_add$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$err$%d", funcStruct->funcName, id);
    //  Ulozeni do docasnych promenychg
    printN("POPS LF@%s", rn);  // Uložíme pravou hodnotu (right)
    printN("POPS LF@%s", ln);  // Uložíme levou hodnotu (left)

    // vratime zpatky
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    // Typová kontrola pomocí zásobníku
    // Stav zásobníku: [left_value, right_value]

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");  // zásobník: [left_value,	right_type]

    // Dočasně uložíme typ pravého operandu
    printN("POPS LF@%s", ty);  // zásobník: [left_value]

    // Zjistíme typ levého operandu (left)
    printN("TYPES");  // zásobník: [type_left]

    // Vrátíme typ pravého operandu zpět
    printN("PUSHS LF@%s", ty);  // zásobník: [type_left, type_right]
                                // Porovnání, že typy jsou stejné
    printN("JUMPIFEQS %s", labelOk);
    printN("JUMP %s", labelErr);

    printN("LABEL %s", labelOk);
    // Je to string?
    printN("PUSHS LF@%s", ty);      // [right_type]
    printN("PUSHS string@string");  // [right_type, "string"]
    printN("JUMPIFEQS %s", labelConcat);

    // Je to int?
    printN("PUSHS LF@%s", ty);   // [right_type]
    printN("PUSHS string@int");  // [right_type, "int"]
    printN("JUMPIFEQS %s", labelAdd);

    // Je to float?
    printN("PUSHS LF@%s", ty);     // [right_type]
    printN("PUSHS string@float");  // [right_type, "float"]
    printN("JUMPIFEQS %s", labelAdd);

    printN("JUMP %s", labelErr);
    // --- ADD větev (pro int a float) ---
    printN("LABEL %s", labelAdd);
    // Musíme znovu načíst původní hodnoty
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("ADDS");
    printN("JUMP %s", labelEnd);
    // --- CONCAT větev (pro string) ---
    printN("LABEL %s", labelConcat);
    printN("MOVE LF@%s string@\"\"", rs);
    printN("CONCAT LF@%s LF@%s LF@%s", rs, ln, rn);
    printN("PUSHS LF@%s", rs);
    printN("JUMP %s", labelEnd);

    // --- CHYBA ---
    printN("LABEL %s", labelErr);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    // --- KONEC ---
    printN("LABEL %s", labelEnd);
    printN("");
}
/**
 * @brief Generuje IFJcode25 kód pro binární operaci odčítání (-).
 * Na unární minus je zavolan `emitUnaryMinus`.
 *
 * @param node Uzel AST pro operaci AST_MINUS.
 * @param funcStruct Kontext funkce.
 */
void emitMinus(AstNode *node, FuncContext *funcStruct) {
    if (node->arity == 1) {
        emitUnaryMinus(node, funcStruct);
        return;
    }
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    // Po emitBinaryOp máme na zásobníku: [hodnota_left, hodnota_right]
    int  id = node->tmpId;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStruct->funcName, id);

    int  size_for_label = strlen(funcStruct->funcName) + 20;
    char labelOk[size_for_label], labelEnd[size_for_label], labelErr[size_for_label];
    snprintf(labelOk, sizeof labelOk, "%s$ok_minus$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$end_minus$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$err_minus$%d", funcStruct->funcName, id);
    //  Ulozeni do docasnych promenychg
    printN("POPS LF@%s", rn);  // Uložíme pravou hodnotu (right)
    printN("POPS LF@%s", ln);  // Uložíme levou hodnotu (left)

    // vratime zpatky
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    // Typová kontrola pomocí zásobníku
    // Stav zásobníku: [left_value, right_value]

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");  // zásobník: [left_value,	right_type]

    // Dočasně uložíme typ pravého operandu
    printN("POPS LF@%s", ty);  // zásobník: [left_value]

    // Zjistíme typ levého operandu (left)
    printN("TYPES");  // zásobník: [type_left]

    // Vrátíme typ pravého operandu zpět
    printN("PUSHS LF@%s", ty);  // zásobník: [type_left, type_right]
                                // Porovnání, že typy jsou stejné
    printN("JUMPIFEQS %s", labelOk);
    printN("JUMP %s", labelErr);

    printN("LABEL %s", labelOk);
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", labelErr);

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("SUBS");
    printN("JUMP %s", labelEnd);
    // --- CHYBA ---
    printN("LABEL %s", labelErr);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    // --- KONEC ---
    printN("LABEL %s", labelEnd);
    printN("");
}
/**
 * @brief Generuje IFJcode25 kód pro operaci nasobení/konkatenac (*).
 *
 * Zajišťuje typovou kontrolu a provádí nasobeni pro čísla nebo konkatenaci pro řetězce a čislo.
 *
 * @param node Uzel AST pro operaci AST_PLUS.
 * @param funcStruct Kontext funkce pro dočasné proměnné a labely.
 */
void emitMul(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }

    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128], rs[128], counter[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->funcName, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->funcName, id);
    snprintf(rs, sizeof rs, "%s_rs_%d", funcStruct->funcName, id);
    snprintf(counter, sizeof counter, "%s_counter_%d", funcStruct->funcName, id);

    char label_mul[128], labelConcat[128], labelEnd[128], labelErr[128], label_start[128], label_loop_end[128],labelConcat_1[128];
    snprintf(label_mul, sizeof label_mul, "%s$mul$%d", funcStruct->funcName, id);
    snprintf(labelConcat, sizeof labelConcat, "%s$concat$%d", funcStruct->funcName, id);
    snprintf(labelConcat_1, sizeof labelConcat_1, "%s$concat_1$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$end_mul$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$err$%d", funcStruct->funcName, id);
    snprintf(label_start, sizeof label_start, "%s$start$%d", funcStruct->funcName, id);
    snprintf(label_loop_end, sizeof label_loop_end, "%s$loop_end$%d", funcStruct->funcName, id);

    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    // Zjisti typy
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", lty);

    printN("PUSHS LF@%s", rn);
    printN("TYPES");
    printN("POPS LF@%s", rty);

    // Kontrola: levý je string?
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", labelConcat_1);

    // Kontrola: pravý je string = chyba
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", labelErr);

    // Jinak násobení float * float
    printN("LABEL %s", label_mul);

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("MULS");
    printN("JUMP %s", labelEnd);

    printN("LABEL %s",labelConcat_1);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFNEQS %s", labelErr);

    printN("PUSHS LF@%s", rn);
    printN("ISINTS");
    printN("PUSHS bool@true");
    printN("JUMPIFNEQS %s", labelErr);
    // CONCAT větev
    printN("LABEL %s", labelConcat);

    printN("MOVE LF@%s int@0", counter);
    printN("PUSHS LF@%s", rn);
    printN("FLOAT2INTS");
    printN("POPS LF@%s", counter);

    printN("MOVE LF@%s string@\"\"", rs);

    printN("PUSHS LF@%s", counter);
    printN("PUSHS int@0");
    printN("JUMPIFEQS %s", label_loop_end);

    printN("MOVE LF@%s LF@%s", rs, ln);

    printN("PUSHS LF@%s", counter);
    printN("PUSHS int@1");
    printN("SUBS");
    printN("POPS LF@%s", counter);

    printN("LABEL %s", label_start);

    printN("PUSHS LF@%s", counter);
    printN("PUSHS int@0");
    printN("JUMPIFEQS %s", label_loop_end);

    printN("CONCAT LF@%s LF@%s LF@%s", rs, rs, ln);

    printN("PUSHS LF@%s", counter);
    printN("PUSHS int@1");
    printN("SUBS");
    printN("POPS LF@%s", counter);

    printN("JUMP %s", label_start);

    printN("LABEL %s", label_loop_end);
    printN("PUSHS LF@%s", rs);

    printN("JUMP %s", labelEnd);

    printN("LABEL %s", labelErr);
    printN("EXIT int@%d", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro dělení (/), včetně konverze int na float a ošetření dělení nulou.
 *
 * @param node Uzel AST pro operaci dělení.
 * @param funcStrukt Kontext funkce.
 */
void emitDiv(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    // Příprava dočasných proměnných a labelů
    int  id = node->tmpId;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStruct->funcName, id);

    char labelOk[128], labelIntDiv[128], labelDivZero[128], labelEnd[128], labelErr[128];
    snprintf(labelOk, sizeof labelOk, "%s$ok_div$%d", funcStruct->funcName, id);
    snprintf(labelIntDiv, sizeof labelIntDiv, "%s$intdiv_div$%d", funcStruct->funcName, id);
    snprintf(labelDivZero, sizeof labelDivZero, "%s$divzero_div$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$end_div$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$err_div$%d", funcStruct->funcName, id);

    // Uložení operandů do dočasných proměnných
    printN("POPS LF@%s", rn);  // Uložíme pravou hodnotu (right)
    printN("POPS LF@%s", ln);  // Uložíme levou hodnotu (left)

    // Vrátíme operandy zpět na zásobník pro typovou kontrolu
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    // Stav zásobníku: [left_value, right_value]

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");  // zásobník: [left_value, right_type]

    // Dočasně uložíme typ pravého operandu
    printN("POPS LF@%s", ty);  // zásobník: [left_value]

    // Zjistíme typ levého operandu (left)
    printN("TYPES");  // zásobník: [type_left]

    // Vrátíme typ pravého operandu zpět
    printN("PUSHS LF@%s", ty);  // zásobník: [type_left, type_right]

    // Porovnání, že typy jsou stejné
    printN("JUMPIFEQS %s", labelOk);
    printN("JUMP %s", labelErr);

    // Typy jsou stejné - nyní zkontrolujeme konkrétní typ
    printN("LABEL %s", labelOk);

    // Zakázat string (string nelze dělit)
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", labelErr);

    // Je to int? → potřeba konverze na float
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labelIntDiv);

    // Je to float → kontrola dělení float nulou
    printN("PUSHS LF@%s", rn);
    printN("PUSHS float@0x0p+0");
    printN("JUMPIFEQS %s", labelDivZero);

    // Float dělení (ln / rn)
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("DIVS");
    printN("JUMP %s", labelEnd);

    // Int dělení - kontrola int/0, pak konverze na float
    printN("LABEL %s", labelIntDiv);

    // Kontrola dělení int nulou
    printN("PUSHS LF@%s", rn);
    printN("PUSHS int@0");
    printN("JUMPIFEQS %s", labelDivZero);

    // Konverze int → float pro oba operandy
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    // Dělení po konverzi (ln / rn)
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("DIVS");
    printN("JUMP %s", labelEnd);

    // Chyba: dělení nulou
    printN("LABEL %s", labelDivZero);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    // Chyba: nekompatibilní typy
    printN("LABEL %s", labelErr);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    // Konec dělení
    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro operaci rovnosti (==) s kontrolou typové shody.
 *
 * Pokud se typy liší, vrací `bool@false`. Jinak používá instrukci `EQS`.
 *
 * @param node Uzel AST pro operaci AST_EQ.
 * @param funcStrukt Kontext funkce.
 */
void emitEqual(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    // Příprava dočasných proměnných a labelů
    int  id = node->tmpId;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStruct->funcName, id);

    char labelOk[128], labelEnd[128], labelErr[128];
    snprintf(labelOk, sizeof labelOk, "%s$ok$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$end_equal$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$err$%d", funcStruct->funcName, id);

    // Uložení operandů do dočasných proměnných
    printN("POPS LF@%s", rn);  // Uložíme pravou hodnotu (right)
    printN("POPS LF@%s", ln);  // Uložíme levou hodnotu (left)

    // Vrátíme operandy zpět na zásobník pro typovou kontrolu
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    // Stav zásobníku: [left_value, right_value]

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");  // zásobník: [left_value, right_type]

    // Dočasně uložíme typ pravého operandu
    printN("POPS LF@%s", ty);  // zásobník: [left_value]

    // Zjistíme typ levého operandu (left)
    printN("TYPES");  // zásobník: [type_left]

    // Vrátíme typ pravého operandu zpět
    printN("PUSHS LF@%s", ty);  // zásobník: [type_left, type_right]

    // Porovnání, že typy jsou stejné
    printN("JUMPIFEQS %s", labelOk);
    printN("JUMP %s", labelErr);

    // Typy jsou stejné - nyní zkontrolujeme konkrétní typ
    printN("LABEL %s", labelOk);
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("EQS");
    printN("JUMP %s", labelEnd);

    printN("LABEL %s", labelErr);
    printN("PUSHS bool@false");
    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro porovnání menší než (<) s konverzí na float.
 *
 * Konvertuje oba numerické operandy na `float` a používá instrukci `LTS`.
 *
 * @param node Uzel AST pro operaci AST_LT.
 * @param funcStruct Kontext funkce.
 */
void emitLess(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->funcName, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->funcName, id);

    char labelOk[128], labelEnd[128], labelErr[128], labellFloat[128], labelRFloat[128], labelRCheck[128];

    snprintf(labelOk, sizeof labelOk, "%s$le_ok$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$le_end$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$le_err$%d", funcStruct->funcName, id);
    snprintf(labellFloat, sizeof labellFloat, "%s$le_l_f$%d", funcStruct->funcName, id);
    snprintf(labelRFloat, sizeof labelRFloat, "%s$le_r_f$%d", funcStruct->funcName, id);
    snprintf(labelRCheck, sizeof labelRCheck, "%s$le_r_chk$%d", funcStruct->funcName, id);

    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    printN("PUSHS LF@%s", rn);
    printN("TYPES");
    printN("POPS LF@%s", rty);
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", lty);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelRCheck);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labellFloat);

    printN("JUMP %s", labelErr);

    printN("LABEL %s", labellFloat);
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);

    printN("LABEL %s", labelRCheck);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelOk);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labelRFloat);
    printN("JUMP %s", labelErr);

    printN("LABEL %s", labelRFloat);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    printN("LABEL %s", labelOk);

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("LTS");
    printN("JUMP %s", labelEnd);

    // Chyba: nil nebo různé typy
    printN("LABEL %s", labelErr);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro porovnání větší nebo rovno (>=).
 *
 * Implementováno jako (A > B) OR (A == B) s automatickou konverzí int na float.
 *
 * @param node Uzel AST pro operaci AST_GTE.
 * @param funcStruct Kontext funkce.
 */
void emitGreater(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->funcName, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->funcName, id);

    char labelOk[128], labelEnd[128], labelErr[128], labellFloat[128], labelRFloat[128], labelRCheck[128];

    snprintf(labelOk, sizeof labelOk, "%s$le_ok$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$le_end$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$le_err$%d", funcStruct->funcName, id);
    snprintf(labellFloat, sizeof labellFloat, "%s$le_l_f$%d", funcStruct->funcName, id);
    snprintf(labelRFloat, sizeof labelRFloat, "%s$le_r_f$%d", funcStruct->funcName, id);
    snprintf(labelRCheck, sizeof labelRCheck, "%s$le_r_chk$%d", funcStruct->funcName, id);

    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    printN("PUSHS LF@%s", rn);
    printN("TYPES");
    printN("POPS LF@%s", rty);
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", lty);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelRCheck);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labellFloat);

    printN("JUMP %s", labelErr);

    printN("LABEL %s", labellFloat);
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);

    printN("LABEL %s", labelRCheck);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelOk);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labelRFloat);
    printN("JUMP %s", labelErr);

    printN("LABEL %s", labelRFloat);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    printN("LABEL %s", labelOk);

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("GTS");
    printN("JUMP %s", labelEnd);

    // Chyba: nil nebo různé typy
    printN("LABEL %s", labelErr);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro porovnání menší nebo rovno (<=).
 *
 * Implementováno jako (A < B) OR (A == B) s automatickou konverzí int na float.
 *
 * @param node Uzel AST pro operaci AST_LTE.
 * @param funcStruct Kontext funkce.
 */
void emit_less_equal(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->funcName, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->funcName, id);

    char labelOk[128], labelEnd[128], labelErr[128], labellFloat[128], labelRFloat[128], labelRCheck[128];

    snprintf(labelOk, sizeof labelOk, "%s$le_ok$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$le_end$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$le_err$%d", funcStruct->funcName, id);
    snprintf(labellFloat, sizeof labellFloat, "%s$le_l_f$%d", funcStruct->funcName, id);
    snprintf(labelRFloat, sizeof labelRFloat, "%s$le_r_f$%d", funcStruct->funcName, id);
    snprintf(labelRCheck, sizeof labelRCheck, "%s$le_r_chk$%d", funcStruct->funcName, id);

    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    printN("PUSHS LF@%s", rn);
    printN("TYPES");
    printN("POPS LF@%s", rty);
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", lty);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelRCheck);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labellFloat);

    printN("JUMP %s", labelErr);

    printN("LABEL %s", labellFloat);
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);

    printN("LABEL %s", labelRCheck);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelOk);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labelRFloat);
    printN("JUMP %s", labelErr);

    printN("LABEL %s", labelRFloat);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    printN("LABEL %s", labelOk);

    // LTS
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("LTS");

    // EQS
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("EQS");

    // ORS
    printN("ORS");
    printN("JUMP %s", labelEnd);

    printN("LABEL %s", labelErr);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro porovnání větší nebo rovno (>=).
 *
 * Implementováno jako (A > B) OR (A == B) s automatickou konverzí int na float.
 *
 * @param node Uzel AST pro operaci AST_GTE.
 * @param funcStruct Kontext funkce.
 */
void emit_greater_equal(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->funcName, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->funcName, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->funcName, id);

    char labelOk[128], labelEnd[128], labelErr[128], labellFloat[128], labelRFloat[128], labelRCheck[128];

    snprintf(labelOk, sizeof labelOk, "%s$le_ok$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$le_end$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$le_err$%d", funcStruct->funcName, id);
    snprintf(labellFloat, sizeof labellFloat, "%s$le_l_f$%d", funcStruct->funcName, id);
    snprintf(labelRFloat, sizeof labelRFloat, "%s$le_r_f$%d", funcStruct->funcName, id);
    snprintf(labelRCheck, sizeof labelRCheck, "%s$le_r_chk$%d", funcStruct->funcName, id);

    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    printN("PUSHS LF@%s", rn);
    printN("TYPES");
    printN("POPS LF@%s", rty);
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", lty);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelRCheck);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labellFloat);

    printN("JUMP %s", labelErr);

    printN("LABEL %s", labellFloat);
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);

    printN("LABEL %s", labelRCheck);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelOk);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labelRFloat);
    printN("JUMP %s", labelErr);

    printN("LABEL %s", labelRFloat);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    printN("LABEL %s", labelOk);
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("GTS");

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("EQS");
    printN("ORS");
    printN("JUMP %s", labelEnd);
    printN("JUMP %s", labelEnd);

    printN("LABEL %s", labelErr);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    printN("LABEL %s", labelEnd);
}

void emitEqualNot(AstNode *node, FuncContext *funcStruct) {
    emitEqual(node, funcStruct);
    printN("NOTS");
}
/**
 * @brief Generuje IFJcode25 kód pro logické AND (`&&`) s implementací short circuit.
 *
 * Pokud je levý operand `false` nebo `nil`, operace se okamžitě ukončí a vrátí `false`.
 *
 * @param node Uzel AST pro operaci AST_AND.
 * @param funcContext Kontext funkce.
 */
void emitAnd(AstNode *node, FuncContext *funcContext) {
    int  id = node->tmpId;
    char labelFalse[128], labelEnd[128];
    snprintf(labelFalse, sizeof labelFalse, "%s$and_false$%d", funcContext->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$and_end$%d", funcContext->funcName, id);

    char leftTmp[128];
    snprintf(leftTmp, sizeof leftTmp, "%s_and_left_%d", funcContext->funcName, id);

    emitCondition(node->child[0], funcContext);
    printN("POPS LF@%s", leftTmp);

    // short circuit: if LHS == false → false
    printN("PUSHS LF@%s", leftTmp);
    printN("PUSHS bool@false");
    printN("JUMPIFEQS %s", labelFalse);

    // short circuit: if LHS == nil → false
    printN("PUSHS LF@%s", leftTmp);
    printN("PUSHS nil@nil");
    printN("JUMPIFEQS %s", labelFalse);

    // LHS == true → evaluate RHS
    emitCondition(node->child[1], funcContext);
    printN("JUMP %s", labelEnd);

    printN("LABEL %s", labelFalse);
    printN("PUSHS bool@false");

    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro logické OR (||) s implementací short circuit.
 *
 * Pokud je levý operand `true`, operace se okamžitě ukončí a vrátí `true`.
 *
 * @param node Uzel AST pro operaci AST_OR.
 * @param funcContext Kontext funkce.
 */
void emitOr(AstNode *node, FuncContext *funcContext) {
    int  id = node->tmpId;
    char labelTrue[128], labelEnd[128];
    snprintf(labelTrue, sizeof labelTrue, "%s$or_success$%d", funcContext->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$or_end$%d", funcContext->funcName, id);

    char leftTmp[128];
    snprintf(leftTmp, sizeof leftTmp, "%s_or_left_%d", funcContext->funcName, id);

    emitCondition(node->child[0], funcContext);
    printN("POPS LF@%s", leftTmp);

    printN("PUSHS LF@%s", leftTmp);
    printN("PUSHS bool@true");
    printN("JUMPIFEQS %s", labelTrue);

    emitCondition(node->child[1], funcContext);
    printN("JUMP %s", labelEnd);

    printN("LABEL %s", labelTrue);
    printN("PUSHS bool@true");

    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro logickou negaci (NOT) pomocí instrukce NOTS.
 *
 * @param node Uzel AST pro operaci AST_NOT.
 * @param funcStruct Kontext funkce.
 */
void emitNegace(AstNode *node, FuncContext *funcStruct) {
    int  id = node->tmpId;
    char value[128], valueType[128];
    snprintf(value, sizeof value, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(valueType, sizeof value, "%s_ty_%d", funcStruct->funcName, id);

    char labelFalse[128], labelEnd[128];
    snprintf(labelFalse, sizeof labelFalse, "%s$and_not_false$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$and_end$%d", funcStruct->funcName, id);
    emitCondition(node->child[0], funcStruct);
    // Stack: [left_value]
    printN("NOTS");
    // Stack: [result] (buď left pokud false/nil, nebo right)
}
/**
 * @brief Generuje IFJcode25 kód pro unární negaci (-) pro int a float.
 *
 * Negace je implementována jako odečtení hodnoty od nuly (`0 - hodnota`).
 *
 * @param node Uzel AST pro unární minus.
 * @param funcStrukt Kontext funkce.
 */
void emitUnaryMinus(AstNode *node, FuncContext *funcStruct) {
    emitCommand(node->child[0], funcStruct);
    int id = node->tmpId;

    char ln[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStruct->funcName, id);

    char labelOk[128];
    char labelOkFloat[128];
    char labelEnd[128];
    snprintf(labelOk, sizeof labelOk, "%s$ok$%d", funcStruct->funcName, id);
    snprintf(labelOkFloat, sizeof labelOkFloat, "%s$ok_float$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$end_Minus$%d", funcStruct->funcName, id);

    // Vyndej hodnotu a ulož ji
    printN("POPS LF@%s", ln);

    // Zkontroluj typ
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", ty);

    // Kontrola na int
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labelOk);

    // Kontrola na float
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelOkFloat);

    // Error - nejde o int ani float
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    // OK label - proveď operaci
    printN("LABEL %s", labelOk);
    printN("PUSHS int@0");
    printN("PUSHS LF@%s", ln);
    printN("SUBS");
    printN("JUMP %s", labelEnd);

    printN("LABEL %s", labelOkFloat);
    double nula = 0.0;
    printN("PUSHS float@%a", nula);
    printN("PUSHS LF@%s", ln);
    printN("SUBS");
    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro operátor kontroly typu (`is`).
 *
 * @param node Uzel AST pro operaci kontroly typu.
 * @param funcStruct Kontext funkce.
 */
void emitIs(AstNode *node, FuncContext *funcStruct) {
    emitCommand(node->child[0], funcStruct);
    int  id = node->tmpId;
    char ln[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->funcName, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->funcName, id);
    char labelNum[128], labelString[128], labelNull[128];
    char labelOk[128], labelErr[128], labelEnd[128];
    snprintf(labelNum, sizeof labelNum, "%s$Num$%d", funcStruct->funcName, id);
    snprintf(labelString, sizeof labelString, "%s$String$%d", funcStruct->funcName, id);
    snprintf(labelNull, sizeof labelNull, "%s$Null$%d", funcStruct->funcName, id);
    snprintf(labelOk, sizeof labelOk, "%s$ok$%d", funcStruct->funcName, id);
    snprintf(labelErr, sizeof labelErr, "%s$err$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$end_is$%d", funcStruct->funcName, id);

    // Pop levého operandu do LF@ln
    printN("POPS LF@%s", ln);

    // Zjisti typ levého operandu
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", lty);

    switch (node->child[1]->kind) {
        case AST_KEYWORD_NUM:  // "Num"
            printN("JUMP %s", labelNum);
            break;
        case AST_KEYWORD_STRING:  // "String"
            printN("JUMP %s", labelString);
            break;
        case AST_NULL:  // "Null"
            printN("JUMP %s", labelNull);
            break;
        default:
            printN("PUSHS bool@false");
            return;
    }

    // ---- Num ----
    printN("LABEL %s", labelNum);
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", labelOk);
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", labelOk);
    printN("JUMP %s", labelErr);

    // ---- String ----
    printN("LABEL %s", labelString);
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", labelOk);
    printN("JUMP %s", labelErr);

    // ---- Null ----
    printN("LABEL %s", labelNull);
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", labelOk);
    printN("JUMP %s", labelErr);

    // ---- výsledek ----
    printN("LABEL %s", labelOk);
    printN("PUSHS bool@true");
    printN("JUMP %s", labelEnd);

    printN("LABEL %s", labelErr);
    printN("PUSHS bool@false");

    printN("LABEL %s", labelEnd);
}
/**
 * @brief Generuje IFJcode25 kód pro přiřazení hodnoty lokální proměnné (=).
 *
 * @param node Uzel AST pro přiřazení.
 * @param funcStruct Kontext funkce.
 */
void emitAssigned(AstNode *node, FuncContext *funcStruct) {
    if (node->child[0]->kind == AST_SETTER_CALL) {
        emitCommand(node->child[1], funcStruct);
        emitSetterCall(node->child[0]);
        return;
    }

    emitCommand(node->child[1], funcStruct);
    printN("POPS LF@%s", node->child[0]->value.str);
}
/**
 * @brief Generuje IFJcode25 kód pro inicializaci lokální proměnné.
 *
 * @param node Uzel AST pro deklaraci/inicializaci proměnné.
 * @param funcStruct Kontext funkce.
 */
void emitVar(AstNode *node, FuncContext *funcStruct) {
    if (node->child[1]->kind != AST_EMPTY) {
        emitCommand(node->child[1]->child[0], funcStruct);
        printN("POPS LF@%s", node->child[0]->value.str);
    }
}
/**
 * @brief Generuje IFJcode25 kód pro inicializaci globální proměnné v těle funkce.
 *
 * @param node Uzel AST pro inicializaci globální proměnné.
 * @param funcStruct Kontext funkce.
 */
void emitGLobal(AstNode *node, FuncContext *funcStruct) {
    if (node->child[1]->kind != AST_EMPTY) {
        emitCommand(node->child[1]->child[0], funcStruct);
        printN("POPS GF@%s", node->child[0]->value.str);
    }
}
/**
 * @brief Generuje IFJcode25 kód pro celou sekvenci `if-else if-else` (if chain).
 *
 * @param node Kořenový uzel AST pro celý `if-else if-else` řetězec.
 * @param funcContext Kontext funkce pro generování labelů.
 */
void emitIfChain(AstNode *node, FuncContext *funcContext) {
    /**
     * vytvorenie lablu na spode if chainu na ktory v pripade splnenia podmienky
     * budeme z jednotlivych podmienok skakat
     */
    char *endLabel = generatorLable("ifchainend", funcContext, false, ifCounter);

    for (size_t i = 0; i < node->arity; i++) {
        if (node->child[i]->kind == AST_IF) {
            emitIf(node->child[i], funcContext, endLabel);
        } else if (node->child[i]->kind == AST_ELSE) {
            emitElse(node->child[i], funcContext);
            printN("JUMP %s", endLabel);
            break;
        } else {
            printError(INTERNAL_ERROR, "Chyba vo vystaveni AST stromu");
            memorySafeExit(INTERNAL_ERROR);
        }
    }
    printN("LABEL %s", endLabel);
    free(endLabel);
}
/**
 * @brief Generuje IFJcode25 kód pro blok `if` nebo `else if`.
 *
 * @param node Uzel AST pro příkaz `if`.
 * @param funcContext Kontext funkce.
 * @param endLabel Koncový label celého `if-else if-else` řetězce.
 */
void emitIf(AstNode *node, FuncContext *funcContext, char *endLabel) {
    ifCounter++;
    char *label = NULL;
    label       = generatorLable("ifEnd", funcContext, false, ifCounter);
    // dame skontrolovat podmienku
    emitCondition(node->child[0], funcContext);
    // mame vysledok podmienky true/false na zasobniku
    // pridame si tam umelo false
    printN("PUSHS bool@false");
    // porovname prve 2 hodnoty na zasobniku, nami umelo pridany false a aj vysledok cond
    // ak sa budu rovnat preskoci na novovytvoreny label
    printN("JUMPIFEQS %s", label);

    emitChildren(node->child[1], funcContext);

    // po vykonani if blocku skocime na koniec if retazca kedze bola splnena if podmienka
    printN("JUMP %s", endLabel);

    // vytvorenie lablu v ifjcode na ktory budeme skakat pri vyhodnoteni podmienky ako false
    printN("LABEL %s", label);
    free(label);
}
/**
 * @brief Generuje IFJcode25 kód pro tělo bloku `else`.
 *
 * @param node Uzel AST pro příkaz `else`.
 * @param funcContext Kontext funkce.
 */
void emitElse(AstNode *node, FuncContext *funcContext) { emitChildren(node->child[0], funcContext); }

/**
 * @brief Funkce pro zpracovani while cyklu
 * @param node Ukazatel na aktualne
 */
void emitWhile(AstNode *node, FuncContext *funcContext) {
    loopCounter++;
    char *blockPrefix = generatorLable("while", funcContext, true, loopCounter);
    char *labelStart   = generatorLable("start", funcContext, false, loopCounter);
    printN("LABEL %s", labelStart);
    char *endLabel = NULL;
    endLabel       = generatorLable("end", funcContext, false, loopCounter);
    if (node->child[0]->kind == AST_CONDITION) {
        // while -> condition -> AST samotneho operatoru
        emitCommand(node->child[0], funcContext);
        printN("PUSHS bool@false");
        printN("JUMPIFEQS %s", endLabel);
    } else {
        printError(INTERNAL_ERROR, "(GENERATOR) Chyba pri sestaveni AST stromu\n");
        memorySafeExit(INTERNAL_ERROR);
    }

    emitCommand(node->child[1], funcContext);  // zpracovani obsahu uvnitr while
    printN("JUMP %s", labelStart);             // * JUMP zpět na začáteční návěští

    printN("LABEL %s", endLabel);  // ! LABEL KONEC
    free(endLabel);
    free(labelStart);
    char *poppedPrefix = stacklabelStackRead(funcContext->labelStack);
    if (poppedPrefix == blockPrefix) {
        stacklabelStackPop(funcContext->labelStack);
        free(blockPrefix);
    } else {
        printError(INTERNAL_ERROR, "nepovedene uvolneni lablu na konci while loopu");
        memorySafeExit(INTERNAL_ERROR);
    }
    loopCounter--;
}
/**
 * @brief Generuje IFJcode25 kód pro příkaz `break`.
 *
 * Implementuje skok (`JUMP`) na koncový label aktuální smyčky.
 *
 * @param funcContext Kontext funkce.
 */
void emitBreak(FuncContext *funcContext) {
    char *label = generatorLable("end", funcContext, false, loopCounter);
    printN("JUMP %s", label);
    free(label);
}
/**
 * @brief Generuje IFJcode25 kód pro příkaz `continue`.
 *
 * Implementuje skok (`JUMP`) na počáteční label aktuální smyčky.
 *
 * @param funcContext Kontext funkce.
 */
void emitContinue(FuncContext *funcContext) {
    char *label = generatorLable("start", funcContext, false, loopCounter);
    printN("JUMP %s", label);
    free(label);
}
/**
 * @brief Generuje IFJcode25 kód pro konverzi proměnné na logickou hodnotu (boolean).
 *
 * Implementuje pravidla konverze: `nil` a `bool@false` jsou **false**;
 * jakákoliv jiná hodnota nebo typ je **true**.
 *
 * @param node Uzel AST pro proměnnou.
 * @param funcContext Kontext funkce.
 */
void emitBool(AstNode *node, FuncContext *funcContext) {
    if (node->kind == AST_IDENTIFIER || node->kind == AST_GLOBAL_ID) {
        char tyVar[128], valVar[128];
        char isFalseLabel[128], endLabel[128];
        char isTypeBoolLabel[128];
        int  id = node->tmpId;

        snprintf(tyVar, sizeof tyVar, "%s_ty_bool_%d", funcContext->funcName, id);
        snprintf(valVar, sizeof valVar, "%s_val_bool_%d", funcContext->funcName, id);
        snprintf(isFalseLabel, sizeof isFalseLabel, "%s$bool_is_false$%d", funcContext->funcName, id);
        snprintf(endLabel, sizeof endLabel, "%s$bool_end$%d", funcContext->funcName, id);
        snprintf(isTypeBoolLabel, sizeof isTypeBoolLabel, "%s$bool_type_ok$%d", funcContext->funcName, id);

        const char *varName = node->value.str;

        // Získání hodnoty a typu
        if (node->kind == AST_IDENTIFIER) {
            printN("PUSHS LF@%s", varName);
        } else {
            printN("PUSHS GF@%s", varName);
        }
        printN("POPS LF@%s", valVar);
        printN("PUSHS LF@%s", valVar);
        printN("TYPES");
        printN("POPS LF@%s", tyVar);

        // --- KONTROLA 1: NIL ---
        printN("PUSHS LF@%s", tyVar);
        printN("PUSHS string@nil");
        printN("JUMPIFEQS %s", isFalseLabel);

        // --- KONTROLA 2: TYP NENÍ BOOL ---
        printN("PUSHS LF@%s", tyVar);
        printN("PUSHS string@bool");
        printN("JUMPIFEQS %s", isTypeBoolLabel);

        // --- TRUE případ (jiný typ než nil a bool) ---
        printN("PUSHS bool@true");
        printN("JUMP %s", endLabel);

        // --- KONTROLA 3: HODNOTA BOOL@FALSE
        printN("LABEL %s", isTypeBoolLabel);
        printN("PUSHS LF@%s", valVar);
        printN("PUSHS bool@false");
        printN("JUMPIFEQS %s", isFalseLabel);

        // --- TRUE případ (bool@true) ---
        printN("PUSHS bool@true");
        printN("JUMP %s", endLabel);

        // --- FALSE případy (nil nebo bool@false) ---
        printN("LABEL %s", isFalseLabel);
        printN("PUSHS bool@false");

        // --- Konec ---
        printN("LABEL %s", endLabel);
    }
}
/**
 * @brief Generuje IFJcode25 kód pro vyhodnocení libovolného výrazu v kontextu podmínky (`if`, `while`).
 *
 * Tato funkce zajišťuje, že výsledek výrazu na datovém zásobníku je vždy logická hodnota (`bool@true` nebo `bool@false`).
 * @param node Uzel AST reprezentující podmínku nebo výraz.
 * @param funcContext Kontext funkce.
 */
void emitCondition(AstNode *node, FuncContext *funcContext) {
    if (node->arity == 0) {
        printLog(astNodeTypeToString(node->kind));
        if (node->kind == AST_SMALL_NULL || (node->kind == AST_BOOL_LIT && strcmp(node->value.str, "false") == 0) ||
            (node->kind == AST_INT_LIT && node->value.intVal == 0) ||
            (node->kind == AST_FLOAT_LIT && node->value.floatVal == 0.0)) {
            printN("PUSHS bool@false");
        } else if (node->kind == AST_IDENTIFIER || node->kind == AST_GLOBAL_ID) {
            emitBool(node, funcContext);
        } else {
            printN("PUSHS bool@true");
        }
        return;
    }
    if (node->kind == AST_LT || node->kind == AST_LTE || node->kind == AST_EQ || node->kind == AST_GT ||
        node->kind == AST_GTE || node->kind == AST_OR || node->kind == AST_NOT || node->kind == AST_NEQ ||
        node->kind == AST_AND || node->kind == AST_IS) {
        emitCommand(node, funcContext);
        return;
    }
    AstNode *child = node->child[0];
    if (child->kind == AST_LT || child->kind == AST_LTE || child->kind == AST_EQ || child->kind == AST_GT ||
        child->kind == AST_GTE || child->kind == AST_OR || child->kind == AST_NOT || child->kind == AST_NEQ ||
        child->kind == AST_AND || child->kind == AST_IS) {
        emitCommand(child, funcContext);
    } else if (child->kind == AST_SMALL_NULL ||
               (child->kind == AST_BOOL_LIT && strcmp(child->value.str, "false") == 0)) {
        printN("PUSHS bool@false");
    } else if (child->kind == AST_IDENTIFIER || child->kind == AST_GLOBAL_ID) {
        emitBool(child, funcContext);
    } else {
        printN("PUSHS bool@true");
    }
}
/**
 * @brief Generuje IFJcode25 kód pro cyklus `for-in-range`.
 *
 * Implementuje iteraci v číselném rozsahu (`[start, end]` nebo `[start, end)`) s čítačem.
 * Zajišťuje **typovou kontrolu** hranic rozsahu, konverzi na int a ošetření chyb.
 * Vytváří návěští pro skoky (`break`, `continue`).
 *
 * @param node Uzel AST pro cyklus `for`.
 * @param funcContext Kontext funkce.
 */
void emitFor(AstNode *node, FuncContext *funcContext) {
    loopCounter++;

    char *blockPrefix = generatorLable("for", funcContext, true, loopCounter);

    char *errorLabel         = generatorLable("TypesError", funcContext, false, loopCounter);
    bool  generateErrorLabel = false;

    char *labelStart = generatorLable("start", funcContext, false, loopCounter);

    char *endLabel = generatorLable("end", funcContext, false, loopCounter);

    char *variable = node->child[0]->child[0]->value.str;  // ? hodnota proměnné sektoru podmínky for cyklu

    AstNode *leftRange       = node->child[0]->child[1]->child[0];  // ukazatel na levy uzel rozsahu
    AstNode *rightRange      = node->child[0]->child[1]->child[1];  // ukazatel na pravy uzel rozsahu
    bool     rightIsVariable = false;

    printN("DEFVAR LF@%s", variable);  // definovani hodnoty podmínky for cyklu

    // kotrola pravého intervalu, pokud tam je proměnná

    if (rightRange->value.hasStr) {
        generateErrorLabel = true;  // Jedná se o proměnnou, generuji chybové návěští
        // FLOAT2INT
        rightIsVariable = true;
        printN("PUSHS LF@%s", rightRange->value.str);  // pushne se tam promenna
        printN("TYPES");                               // kontrola typu promenne
        printN("PUSHS string@float");

        printN("JUMPIFNEQS %sexitSix", errorLabel);  // ! pokud nejsou stejne, tak skáču na chybové návěští
        printN("PUSHS LF@%s", rightRange->value.str);
        printN("ISINTS");
        printN("PUSHS bool@true");
        printN("JUMPIFNEQS %sexitSix", errorLabel);
        printN("PUSHS LF@%s", rightRange->value.str);
        printN("FLOAT2INTS");
    }

    // kontrola levého intervalu, pokud tam je proměnná
    if (leftRange->value.hasStr) {
        generateErrorLabel = true;  // Jedná se o proměnnou, generuji chybové návěští

        printN("PUSHS LF@%s", leftRange->value.str);  // pushne se tam promenna
        printN("TYPES");                              // kontrola typu promenne
        printN("PUSHS string@float");

        printN("JUMPIFNEQS %sexitSix", errorLabel);  // ! pokud nejsou stejne, tak skáču na chybové návěští
        printN("PUSHS LF@%s", leftRange->value.str);
        printN("ISINTS");
        printN("PUSHS bool@true");
        printN("JUMPIFNEQS %sexitSix", errorLabel);
        printN("PUSHS LF@%s", leftRange->value.str);
        printN("FLOAT2INTS");  // vložení hodnoty na zásobník aby mohla potom proměnná podmínky si načíst tuto hodnotu

    } else {  // jinak víme, že SA zajistí, že tam je celé číslo
        printN("PUSHS int@%d",
               leftRange->value.intVal - 1);  // nastaveni hodnoty, nastavim o 1 mensi, abych mohl rovnou inkrementovat
    }

    printN("POPS LF@%s", variable);   // ulozeni hodnoty do promenne
    printN("PUSHS LF@%s", variable);  // promennou s hodnotu hodim na zasobnik

    printN("LABEL %s", labelStart);  // ! LABEL ZACATEK

    printN("PUSHS int@1");
    printN("ADDS");                  // pricteni jednicky k promenne
    printN("POPS LF@%s", variable);  // potrebuji vysledek hodit opet do promenne

    printN("PUSHS LF@%s", variable);  // potrebuji hodit hodnotu promenne zpet na zasobnik aby mohlo dojit k porovnani

    if (node->child[0]->child[1]->kind == AST_DOUBLE_DOT)  // ? [start, end]
    {
        // na zásobník vkládám jen hodnotu pravé hodnoty v intervalu, jelikož levá tam už je
        if (rightIsVariable) {
            printN("PUSHS LF@%s", rightRange->value.str);
            printN("FLOAT2INTS");
        } else {
            printN("PUSHS int@%d", rightRange->value.intVal);
        }

        printN("GTS");  // ? porovnání aktuální hodnoty proměnné s konečnou hodnotou
        printN("PUSHS bool@true");

        printN("JUMPIFEQS %s", endLabel);
        printN("PUSHS LF@%s", variable);  // hodim si na zasobnik opet hodnotu, ktere zvysuji hodnotu

        emitCommand(node->child[1], funcContext);  // zpracovani dalsich veci ve for cyklu

        printN("JUMP %s", labelStart);  // * JUMP zpět na začáteční návěští

    } else {  // ? [start, end)

        // na zásobník vkládám jen hodnotu pravé hodnoty v intervalu, jelikož levá tam už je
        if (rightIsVariable) {
            printN("PUSHS LF@%s", rightRange->value.str);
            printN("FLOAT2INTS");
        } else {
            printN("PUSHS int@%d", rightRange->value.intVal);
        }

        printN("EQS");  // ? porovnání, pokud je hodnota proměnné stejná jako konečná hodnota => ukončuji cyklus
        printN("PUSHS bool@true");
        printN("JUMPIFEQS %s", endLabel);

        // ? druhé porovnání, pokud není náhodnou větší
        printN("PUSHS LF@%s", variable);
        if (rightIsVariable) {
            printN("PUSHS LF@%s", rightRange->value.str);
            printN("FLOAT2INTS");
        } else {
            printN("PUSHS int@%d", rightRange->value.intVal);
        }
        printN("GTS");  // ? porovnání, pokud je hodnota proměnné stejná jako konečná hodnota => ukončuji cyklus
        printN("PUSHS bool@true");
        printN("JUMPIFEQS %s", endLabel);

        printN("PUSHS LF@%s", variable);  // vraceni hodnoty na zasobnik

        emitCommand(node->child[1], funcContext);

        printN("JUMP %s", labelStart);  // * JUMP zpět na začáteční návěští
    }
    if (generateErrorLabel) {
        printN("LABEL %sexitSix", errorLabel);
        printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
        generateErrorLabel = false;
    }

    printN("LABEL %s", endLabel);  // ! LABEL KONEC

    free(errorLabel);
    free(endLabel);
    free(labelStart);
    char *poppedPrefix = stacklabelStackRead(funcContext->labelStack);
    if (poppedPrefix == blockPrefix) {
        stacklabelStackPop(funcContext->labelStack);
        free(blockPrefix);  // Tímto uvolňujeme alokovanou paměť
    } else {
        printError(INTERNAL_ERROR, "nepovedene uvolneni lablu na konci while loopu");
        memorySafeExit(INTERNAL_ERROR);
    }
    loopCounter--;
}
/**
 * @brief Generuje IFJcode25 kód pro volání setter funkce.
 *
 * @param node Uzel AST s názvem setteru (AST_SETTER_CALL).
 */
void emitSetterCall(AstNode *node) { printN("CALL %s", node->value.str); }
/**
 * @brief Generuje IFJcode25 kód pro definici a tělo setter funkce.
 *
 * Setter je generován jako standardní funkce, která přijímá jeden parametr.
 *
 * @param node Uzel AST pro definici setteru (AST_SETTER).
 */
void emitSetter(AstNode *node) {
    DynamiclabelStackStackPtr labelStack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    FuncContext              *context    = constuctorFuncContext();
    context->funcName                   = node->child[0]->value.str;

    context->labelStack = labelStack;
    char *labelEnd       = generatorLable("setterEnd", context, true, 0);
    context->labelEnd   = labelEnd;
    printN("LABEL %s", node->child[0]->value.str);
    printN("CREATEFRAME");
    printN("PUSHFRAME");

    printIfj();
    char *pname = node->child[1]->child[0]->value.str;
    printN("DEFVAR LF@%s", pname);
    printN("POPS LF@%s", pname);  // popne si ze zasobniku hodnotu parametru

    VarList *localVariables = varListCreate();
    collectLocalVars(node->child[2], localVariables, context);  // funkce uvnitr
    for (int i = 0; i < localVariables->count; i++) {
        printN("DEFVAR LF@%s", localVariables->varNames[i]);
    }

    for (size_t i = 0; i < node->child[2]->child[0]->arity; i++) {
        emitCommand(node->child[2]->child[0]->child[i], context);
    }
    printN("LABEL %s", context->labelEnd);
    printN("POPFRAME");
    printN("RETURN");

    varListDescrution(localVariables);
    while (!stacklabelStackIsEmpty(labelStack)) {
        char *item = stacklabelStackRead(labelStack);
        stacklabelStackPop(labelStack);
        free(item);
    }
    stacklabelStackDestruct(labelStack);
    destructorFuncContext(context);
}
/**
 * @brief Generuje IFJcode25 kód pro volání getter funkce.
 *
 * @param node Uzel AST s názvem getteru (AST_GETTER_CALL).
 */
void emitGetterCall(AstNode *node) { printN("CALL %s", node->value.str); }
/**
 * @brief Generuje IFJcode25 kód pro definici a tělo getter funkce.
 *
 * Getter je generován jako standardní funkce, která vrací hodnotu:
 * @param node Uzel AST pro definici getteru (AST_GETTER).
 */
void emitGetter(AstNode *node) {
    DynamiclabelStackStackPtr labelStack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);

    FuncContext *context = constuctorFuncContext();
    context->funcName   = node->child[0]->value.str;

    context->labelStack = labelStack;
    char *labelEnd       = generatorLable("getterEnd", context, true, 0);
    context->labelEnd   = labelEnd;
    printN("LABEL %s", node->child[0]->value.str);
    printN("CREATEFRAME");
    printN("PUSHFRAME");

    printIfj();
    VarList *localVariables = varListCreate();
    varListAdd(localVariables, "retval");
    collectLocalVars(node->child[2], localVariables, context);  // funkce uvnitr
    for (int i = 0; i < localVariables->count; i++) {
        printN("DEFVAR LF@%s", localVariables->varNames[i]);
    }
    printN("MOVE LF@retval nil@nil");
    for (size_t i = 0; i < node->child[2]->child[0]->arity; i++) {
        emitCommand(node->child[2]->child[0]->child[i], context);
    }
    printN("LABEL %s", context->labelEnd);
    printN("PUSHS LF@retval");
    printN("POPFRAME");
    printN("RETURN");

    varListDescrution(localVariables);
    while (!stacklabelStackIsEmpty(labelStack)) {
        char *item = stacklabelStackRead(labelStack);
        stacklabelStackPop(labelStack);
        free(item);
    }
    stacklabelStackDestruct(labelStack);
    destructorFuncContext(context);
}
/**
 * @brief Generuje IFJcode25 kód pro ternární operátor (podmíněný výraz: `podmínka ? hodnota_true : hodnota_false`).
 *
 * @param node Uzel AST reprezentující ternární operátor.
 * @param funcStruct Kontext funkce.
 */
void emitTernarni(AstNode *node, FuncContext *funcStruct) {
    char labelTrue[128], labelEnd[128];
    int  id = node->tmpId;
    snprintf(labelTrue, sizeof labelTrue, "%s$true$%d", funcStruct->funcName, id);
    snprintf(labelEnd, sizeof labelEnd, "%s$end$%d", funcStruct->funcName, id);
    emitCondition(node->child[0], funcStruct);
    printN("PUSHS bool@true");
    printN("JUMPIFEQS %s", labelTrue);
    emitCommand(node->child[2], funcStruct);
    printN("JUMP %s", labelEnd);
    printN("LABEL %s", labelTrue);
    emitCommand(node->child[1], funcStruct);
    printN("LABEL %s", labelEnd);
}
/**
 * @brief Hlavní switch pro generování IFJcode25 kódu z uzlů AST.
 *
 * Tato funkce prochází typ uzlu AST (`node->kind`) a volá odpovídající
 * emitovací funkci (např. `emitPlus`, `emitWhile`, `emitVar`) nebo generuje přímo
 * IFJcode25 instrukce pro jednoduché případy.
 *
 * @param node Uzel AST k emitování.
 * @param funcStruct Kontext funkce (pouze pro složité operace a uvnitř funkcí).
 */
void emitCommand(AstNode *node, FuncContext *funcStruct) {
    switch (node->kind) {
        case AST_PROLOG:
            emitProlog();
            break;
        case AST_MAIN_CLASS:
            emitGlobal(node);
            printN("JUMP main$0");
            emitChildren(node, funcStruct);

            break;
        case AST_PROGRAM:
        case AST_BLOCK:
        case AST_STATEMENT_LIST:
            emitChildren(node, funcStruct);
            break;
        case AST_MAIN_CLASS_BLOCK:
            emitFunctions(node, funcStruct);
            break;
        case AST_FLOAT_LIT:
            printN("PUSHS float@%a", node->value.floatVal);
            break;
        case AST_INT_LIT:
            printN("PUSHS int@%lld", node->value.intVal);
            printN("INT2FLOATS");
            break;

        case AST_STRING_LIT:
            printN("PUSHS string@%s", node->value.str);
            break;

        case AST_BOOL_LIT:
            if (strcmp(node->value.str, "true") == 0) {
                printN("PUSHS bool@true");
            } else {
                printN("PUSHS bool@false");
            }
            break;
        case AST_KEYWORD_NUM:
            printN("PUSHS string@Num");
            break;
        case AST_KEYWORD_STRING:
            printN("PUSHS string@string");
            break;
        case AST_SMALL_NULL:
            printN("PUSHS nil@nil");
            break;
        case AST_FLOAT_LIT_EXP:
            printN("PUSHS int@%s", node->value.floatVal);
            break;
        case AST_INT_LIT_EXP:
            printN("PUSHS int@%s", node->value.intVal);
            break;
        case AST_WHILE:
            emitWhile(node, funcStruct);
            break;
        case AST_FOR:
            emitFor(node, funcStruct);
            break;

        case AST_SETTER_CALL:
            // printN("AHOJ SETTER CALL");
            emitSetterCall(node);
            break;
        case AST_GETTER_CALL:
            // printN("AHOJ GETTER CALL");
            emitGetterCall(node);
            break;
        case AST_SETTER:
            // printN("AHOJ SETTER");
            emitSetter(node);
            break;
        case AST_GETTER:
            // printN("AHOJ GETTER");
            emitGetter(node);
            break;
        case AST_NOT:
            emitNegace(node, funcStruct);
            break;
        case AST_IF_CHAIN:
            emitIfChain(node, funcStruct);
            break;
        case AST_BREAK:
            emitBreak(funcStruct);
            break;
        case AST_CONTINUE:
            emitContinue(funcStruct);
            break;
        case AST_RETURN:
            // Nová implementace: navrat z funkce
            emitReturn(node, funcStruct);
            break;

        // Deklarace a volání
        case AST_FUNC_DEC:
            // definice funkce
            emitFuncDec(node);
            break;
        case AST_FUNC_CALL:
            emit_call_func(node, funcStruct);
            break;

        // Proměnné / Přiřazení
        case AST_VAR:
            // Nová implementace: deklarace/inicializace proměnné
            emitVar(node, funcStruct);
            break;
        case AST_ASSIGN:
            emitAssigned(node, funcStruct);
            break;
        case AST_IS:
            emitIs(node, funcStruct);
            break;
        // Listy
        case AST_IDENTIFIER:
            if (strcmp(node->value.str, "Program") != 0) {
                printN("PUSHS LF@%s", node->value.str);
                break;
            }
            break;
        case AST_GLOBAL_ID:
            // Nová implementace: nacteni globalni promenne
            if (node->arity != 0) {
                emitGLobal(node, funcStruct);
            } else {
                printN("PUSHS GF@%s", node->value.str);
            }
            break;

        case AST_EMPTY:
            break;

        // Operace (Expression)
        case AST_PLUS:
            emitPlus(node, funcStruct);
            break;
        case AST_MINUS:
            emitMinus(node, funcStruct);
            break;
        case AST_MUL:
            emitMul(node, funcStruct);
            break;
        case AST_DIV:
            emitDiv(node, funcStruct);
            break;
        case AST_EQ:
            emitEqual(node, funcStruct);
            break;
        case AST_LT:
            emitLess(node, funcStruct);
            break;
        case AST_GT:
            emitGreater(node, funcStruct);
            break;
        case AST_NEQ:
            emitEqualNot(node, funcStruct);
            break;
        case AST_AND:
            emitAnd(node, funcStruct);
            break;
        case AST_OR:
            emitOr(node, funcStruct);
            break;
        case AST_GTE:
            emit_greater_equal(node, funcStruct);
            break;
        case AST_LTE:
            emit_less_equal(node, funcStruct);
            break;
        case AST_LPAREN:
            break;
        case AST_RPAREN:
            break;
        case AST_NULL:
            printN("PUSHS string@Null");
            break;
        case AST_TERNARY:
            emitTernarni(node, funcStruct);
            break;
        case AST_CONDITION:
            emitCondition(node, funcStruct);
            break;
        case AST_IFJ_CALL:
            handleBuiltin(node, funcStruct);
            break;
        default:
            printLog("%s \n", astNodeTypeToString(node->kind));
            printError(INTERNAL_ERROR, "Chyba v generatoru neznamy uzel");
            memorySafeExit(INTERNAL_ERROR);
    }
}
/**
 * @brief Hlavní funkce pro generování IFJcode23 kódu.
 *
 * @param root Kořenový uzel abstraktního syntaktického stromu.
 */
void codegenGenerate(AstNode *root) {
    FuncContext *funcStruct = constuctorFuncContext();
    emitChildren(root, funcStruct);
    destructorFuncContext(funcStruct);
}
