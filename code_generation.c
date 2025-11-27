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
static size_t loop_counter = 0;
static size_t if_counter   = 0;

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
 * @param func_name Jméno bloku (např. "fun", "if", "while").
 * @param funcStruckt Kontext funkce obsahující zásobník labelů (`label_stack`).
 * @param pridaniDoStacku Logická hodnota určující, zda se má vygenerovaný label uložit na zásobník.
 * @param global_counter Globální čítač pro unikátní číslování aktuálního bloku.
 * @return Ukazatel na nově alokovaný řetězec obsahující výsledný label.
 */
char *generatorLable(char *func_name, FuncContext *funcStruckt, bool pridaniDoStacku, size_t global_counter) {
    size_t size_of_buffer = 128;
    char  *result_label   = (char *)malloc(size_of_buffer);
    if (result_label == NULL) {
        printError(INTERNAL_ERROR, "Error allocating memory for label");
        memorySafeExit(INTERNAL_ERROR);
    }

    int current_len = 0;

    // Získání existujícího prefixu ze zásobníku (pokud existuje)
    if (!stacklabelStackIsEmpty(funcStruckt->label_stack)) {
        char *prefix = stacklabelStackRead(funcStruckt->label_stack);
        if (prefix != NULL) {
            size_t prefix_len = strlen(prefix);

            if (prefix_len + 1 + 10 >= size_of_buffer) {
                result_label = resizeLabelName(result_label, &size_of_buffer);
                if (result_label == NULL) {
                    printError(INTERNAL_ERROR, "Error resizing label buffer");
                    memorySafeExit(INTERNAL_ERROR);
                }
            }
            if (prefix_len + 1 + 10 >= size_of_buffer) {
                result_label = resizeLabelName(result_label, &size_of_buffer);
                if (result_label == NULL) {
                    printError(INTERNAL_ERROR, "Error resizing label buffer");
                    memorySafeExit(INTERNAL_ERROR);
                }
            }

            current_len = snprintf(result_label, size_of_buffer, "%s", prefix);
            if (current_len < 0 || (size_t)current_len >= size_of_buffer) {
                free(result_label);
                printError(INTERNAL_ERROR, "Error during initial label copy");
                memorySafeExit(INTERNAL_ERROR);
                return NULL;
            }
        }
    }

    // 2. Přidání nového bloku jako suffix: "$if$3"
    // Formát: $TypBloku$CisloBloku
    size_t remaining_space = size_of_buffer - current_len;
    int    written         = 0;
    if (funcStruckt->main == false) {
        written = snprintf(result_label + current_len, remaining_space, "$%s$%zu", func_name, global_counter);
    } else if (!pridaniDoStacku) {
        written = snprintf(result_label + current_len, remaining_space, "$%s", func_name);
    } else {
        written = snprintf(result_label + current_len, remaining_space, "%s", func_name);
    }
    if (written <= 0 || (size_t)written >= remaining_space) {
        // potřebna resize
        if ((size_t)written >= remaining_space) {
            result_label = resizeLabelName(result_label, &size_of_buffer);
            if (result_label == NULL) {
                printError(INTERNAL_ERROR, "Error resizing label buffer (suffix)");
                memorySafeExit(INTERNAL_ERROR);
            }
            remaining_space = size_of_buffer - current_len;
            // znovu zapsani
            written = snprintf(result_label + current_len, remaining_space, "$%s$%zu", funcStruckt->func_name,
                               global_counter);
        }

        if (written <= 0) {
            free(result_label);
            printError(INTERNAL_ERROR, "Error during suffix creation");
            memorySafeExit(INTERNAL_ERROR);
        }
    }
    current_len += written;

    if (pridaniDoStacku) {
        // Uložení plného label na zásobník
        // pro funkce a if-else while-for pro dalsi zanoreni
        stacklabelStackPush(funcStruckt->label_stack, result_label);
    }
    // a jeho vraceni
    return result_label;
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
    list->var_names = calloc(1, sizeof(char *) * list->capacity);
    return list;
}
/**
 * @brief Přidá jméno proměnné do seznamu (listu) s automatickou změnou velikosti.
 *
 * Pokud je seznam plný, kapacita se zdvojnásobí pomocí `realloc()`.
 * Funkce kontroluje duplicitu: pokud proměnná s daným jménem již existuje,
 * operace je ignorována. Jméno proměnné je zkopírováno (duplikováno) a uloženo.
 *
 * @param list Ukazatel na strukturu VarList, do které se má proměnná přidat.
 * @param name Ukazatel na řetězec obsahující jméno proměnné, které se má přidat.
 */
void varListAdd(VarList *list, char *name) {
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->var_names[i], name) == 0) {
            return;
        }
    }
    // az moc promenych resize
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->var_names = realloc(list->var_names, sizeof(char *) * list->capacity);
    }
    list->var_names[list->count] = strdup(name);
    list->count++;
}
/**
 * @brief Uvolní (zničí) seznam proměnných a veškerou alokovanou paměť.
 *
 * Funkce uvolní paměť alokovanou pro jednotlivá jména proměnných i pro samotné pole
 * ukazatelů.
 *
 * @param list Ukazatel na strukturu VarList, která má být zničena.
 */
void varListDescrution(VarList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->var_names[i]);
    }
    free(list->var_names);
    free(list);
}
/**
 * @brief Generuje kód pro všechny deklarované funkce.
 *
 * Zajišťuje, že hlavní funkce (`main$0`) je emitována jako poslední,
 * aby byla umístěna na konec programu a ostatní funkce
 * jsou emitovány v pořadí, v jakém se nacházejí v AST.
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
 * Tato funkce generuje unikátní jména dočasných proměnných na základě typu binární
 * operace (uzlu AST) a aktuálního kontextu (jména funkce a ID). Jména jsou
 * uložena do seznamu `vars`.
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
    funcStruct->func_name     = NULL;
    funcStruct->label_stack   = NULL;
    funcStruct->label_end     = NULL;
    funcStruct->main          = false;
    funcStruct->value_counter = 0;
    funcStruct->loop_counter  = 0;
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
            node->tmpId = funcStruct->value_counter++;
            addBinopTmps(vars, funcStruct->func_name, node->tmpId, node);
            break;
        }
        default:
            break;
    }
}
/**
 * @brief Rekurzivně sbírá jména globálních proměnných z AST.
 *
 * Prochází Abstract Syntax Tree (AST) a přidává jména uzlů typu `AST_GLOBAL_ID`
 * do seznamu `global`. Jména globálních proměnných jsou potřebná pro jejich
 * následnou inicializaci.
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
 * Funkce nejprve rekurzivně posbírá všechna jména globálních proměnných z celého AST
 * pomocí `emitLookGlobal`. Poté pro každou proměnnou vygeneruje IFJcode23
 * instrukce pro její definici (`DEFVAR GF@...`) a inicializaci na `nil@nil`
 * (`MOVE GF@... nil@nil`).
 *
 * @param root Kořenový uzel AST celého programu.
 */
void emitGlobal(AstNode *root) {
    VarList *globals = varListCreate();
    emitLookGlobal(root, globals);
    for (int i = 0; i < globals->count; i++) {
        printN("DEFVAR GF@%s", globals->var_names[i]);
        printN("MOVE GF@%s nil@nil", globals->var_names[i]);
    }
    varListDescrution(globals);
}
/**
 * @brief Generuje kód pro návrat z funkce (return statement).
 *
 * Funkce nejprve zpracuje výraz, který má být vrácen (`emitChildren`).
 * Návratová hodnota je uložena na zásobník a následně přemístěna do
 * lokální proměnné `LF@retval`. Na závěr generuje skok (`JUMP`) na koncový
 * label funkce (`label_end`).
 *
 * @param root Ukazatel na uzel AST reprezentující příkaz return.
 * @param funcStrukt Kontext funkce (FuncContext) obsahující koncový label.
 */
void emitReturn(AstNode *root, FuncContext *funcStrukt) {
    emitChildren(root, funcStrukt);
    if (root->child[0]->kind == AST_EMPTY) {
        printN("PUSHS nil@nil");
    }
    printN("POPS LF@retval");
    printN("JUMP %s", funcStrukt->label_end);
}
/**
 * @brief Generuje IFJcode23 kód pro celou deklaraci funkce (funkční tělo).
 *
 * Funkce je zodpovědná za kompletní emitování funkčního těla, včetně:
 * 1. **Inicializace kontextu** (stacky labelů, čítače dočasných proměnných).
 * 2. **Definování počátečního a koncového labelu** funkce (LABEL).
 * 3. **Správy rámců** (`CREATEFRAME`, `PUSHFRAME`).
 * 4. **Sbírání lokálních proměnných** (včetně `retval`) a jejich definice (`DEFVAR`).
 * 5. **Zpracování parametrů** voláním `POPS` pro jejich přesun z datového zásobníku do lokálních proměnných.
 * 6. **Emitování těla funkce** (`emitChildren`).
 * 7. **Správného ukončení** – buď `RETURN` (s návratovou hodnotou na zásobníku) pro běžné funkce, nebo `EXIT` pro hlavní funkci (`main$0`).
 * 8. **Uvolnění veškeré alokované paměti** pro kontext, zásobníky a labely.
 *
 * @param root Ukazatel na uzel AST (`AST_FUNC_DEC`), který reprezentuje celou deklaraci funkce.
 */
void emitFuncDec(AstNode *root) {
    DynamiclabelStackStackPtr label_stack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    bool                      main        = false;
    char                     *label       = NULL;
    size_t                    tmp_counter = 0;
    FuncContext              *context     = constuctorFuncContext();
    context->func_name                    = root->child[0]->value.str;
    context->label_end                    = NULL;
    context->label_stack                  = label_stack;
    if (strcmp(root->child[0]->value.str, "main$0") == 0 && root->child[1]->child[0]->kind == AST_EMPTY) {
        main          = true;
        context->main = true;
        label         = generatorLable(root->child[0]->value.str, context, true, tmp_counter);
        printN("LABEL %s", label);
    } else {
        label = generatorLable(root->child[0]->value.str, context, true, tmp_counter);
        printN("LABEL %s", label);
    }
    context->main      = false;
    char *label_end    = generatorLable("end", context, false, tmp_counter);
    context->label_end = label_end;
    printN("CREATEFRAME");
    printN("PUSHFRAME");
    printIfj();
    // sbirani lokalnich promenych na deklaraci
    VarList *local_vars = varListCreate();
    varListAdd(local_vars, "retval");
    collectLocalVars(root->child[2], local_vars, context);  // funkce uvnitr
    for (int i = 0; i < local_vars->count; i++) {
        printN("DEFVAR LF@%s", local_vars->var_names[i]);
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
    printN("LABEL %s", label_end);
    if (!main) {
        printN("PUSHS LF@retval");
        printN("POPFRAME");
        printN("RETURN");
    } else {
        printN("EXIT int@0");
    }
    varListDescrution(local_vars);
    while (!stacklabelStackIsEmpty(label_stack)) {
        char *item = stacklabelStackRead(label_stack);
        stacklabelStackPop(label_stack);
        free(item);
    }
    stacklabelStackDestruct(label_stack);
    destructorFuncContext(context);
    free(label_end);
}
/**
 * @brief Generuje IFJcode23 instrukce pro volání funkce.
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
 * @brief Generuje IFJcode23 kód pro operaci sčítání/konkatenace (+).
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
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStruct->func_name, id);
    snprintf(rs, sizeof rs, "%s_rs_%d", funcStruct->func_name, id);

    int  size_for_label = strlen(funcStruct->func_name) + 20;
    char label_ok[size_for_label], label_add[size_for_label], label_concat[size_for_label], label_end[size_for_label],
        label_err[size_for_label];
    snprintf(label_ok, sizeof label_ok, "%s$ok_add$%d", funcStruct->func_name, id);
    snprintf(label_add, sizeof label_add, "%s$add$%d", funcStruct->func_name, id);
    snprintf(label_concat, sizeof label_concat, "%s$concat$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end_add$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$err$%d", funcStruct->func_name, id);
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
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    printN("LABEL %s", label_ok);
    // Je to string?
    printN("PUSHS LF@%s", ty);      // [right_type]
    printN("PUSHS string@string");  // [right_type, "string"]
    printN("JUMPIFEQS %s", label_concat);

    // Je to int?
    printN("PUSHS LF@%s", ty);   // [right_type]
    printN("PUSHS string@int");  // [right_type, "int"]
    printN("JUMPIFEQS %s", label_add);

    // Je to float?
    printN("PUSHS LF@%s", ty);     // [right_type]
    printN("PUSHS string@float");  // [right_type, "float"]
    printN("JUMPIFEQS %s", label_add);

    printN("JUMP %s", label_err);
    // --- ADD větev (pro int a float) ---
    printN("LABEL %s", label_add);
    // Musíme znovu načíst původní hodnoty
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("ADDS");
    printN("JUMP %s", label_end);
    // --- CONCAT větev (pro string) ---
    printN("LABEL %s", label_concat);
    printN("MOVE LF@%s string@\"\"", rs);
    printN("CONCAT LF@%s LF@%s LF@%s", rs, ln, rn);
    printN("PUSHS LF@%s", rs);
    printN("JUMP %s", label_end);

    // --- CHYBA ---
    printN("LABEL %s", label_err);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    // --- KONEC ---
    printN("LABEL %s", label_end);
    printN("");
}
/**
 * @brief Generuje IFJcode23 kód pro binární operaci odčítání (-).
 *
 * Kontroluje, zda jsou operandy stejného numerického typu, a provádí `SUBS`.
 * Na unární minus je zavolan `emitUnaryMinus`.
 *
 * @param node Uzel AST pro operaci AST_MINUS.
 * @param funcStrukt Kontext funkce.
 */
void emitMinus(AstNode *node, FuncContext *funcStrukt) {
    if (node->arity == 1) {
        emitUnaryMinus(node, funcStrukt);
        return;
    }
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStrukt);
    }
    // Po emitBinaryOp máme na zásobníku: [hodnota_left, hodnota_right]
    int  id = node->tmpId;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStrukt->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStrukt->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStrukt->func_name, id);

    int  size_for_label = strlen(funcStrukt->func_name) + 20;
    char label_ok[size_for_label], label_end[size_for_label], label_err[size_for_label];
    snprintf(label_ok, sizeof label_ok, "%s$ok_minus$%d", funcStrukt->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end_minus$%d", funcStrukt->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$err_minus$%d", funcStrukt->func_name, id);
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
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    printN("LABEL %s", label_ok);
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", label_err);

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("SUBS");
    printN("JUMP %s", label_end);
    // --- CHYBA ---
    printN("LABEL %s", label_err);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    // --- KONEC ---
    printN("LABEL %s", label_end);
    printN("");
}
/*
dodelat * operator na string a cislo
*/
void emitMul(AstNode *node, FuncContext *funcStruct) {
    // Vyhodnoť obě děti
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }

    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128], rs[128], counter[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rs, sizeof rs, "%s_rs_%d", funcStruct->func_name, id);
    snprintf(counter, sizeof counter, "%s_counter_%d", funcStruct->func_name, id);

    char label_mul[128], label_concat[128], label_end[128], label_err[128], label_start[128], label_loop_end[128];
    snprintf(label_mul, sizeof label_mul, "%s$mul$%d", funcStruct->func_name, id);
    snprintf(label_concat, sizeof label_concat, "%s$concat$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end_mul$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$err$%d", funcStruct->func_name, id);
    snprintf(label_start, sizeof label_start, "%s$start$%d", funcStruct->func_name, id);
    snprintf(label_loop_end, sizeof label_loop_end, "%s$loop_end$%d", funcStruct->func_name, id);

    // Ulož hodnoty
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
    printN("JUMPIFEQS %s", label_concat);

    // Kontrola: pravý je string?
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", label_concat);

    // Jinak násobení float * float
    printN("LABEL %s", label_mul);

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("MULS");
    printN("JUMP %s", label_end);

    // CONCAT větev
    printN("LABEL %s", label_concat);

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

    printN("JUMP %s", label_end);

    printN("LABEL %s", label_err);
    printN("EXIT int@%d", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    printN("LABEL %s", label_end);
}

void emitDiv(AstNode *node, FuncContext *funcStrukt) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStrukt);
    }
    // Příprava dočasných proměnných a labelů
    int  id = node->tmpId;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStrukt->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStrukt->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStrukt->func_name, id);

    char label_ok[128], label_int_div[128], label_div_zero[128], label_end[128], label_err[128];
    snprintf(label_ok, sizeof label_ok, "%s$ok_div$%d", funcStrukt->func_name, id);
    snprintf(label_int_div, sizeof label_int_div, "%s$intdiv_div$%d", funcStrukt->func_name, id);
    snprintf(label_div_zero, sizeof label_div_zero, "%s$divzero_div$%d", funcStrukt->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end_div$%d", funcStrukt->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$err_div$%d", funcStrukt->func_name, id);

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
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // Typy jsou stejné - nyní zkontrolujeme konkrétní typ
    printN("LABEL %s", label_ok);

    // Zakázat string (string nelze dělit)
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", label_err);

    // Je to int? → potřeba konverze na float
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_int_div);

    // Je to float → kontrola dělení float nulou
    printN("PUSHS LF@%s", rn);
    printN("PUSHS float@0x0p+0");
    printN("JUMPIFEQS %s", label_div_zero);

    // Float dělení (ln / rn)
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("DIVS");
    printN("JUMP %s", label_end);

    // Int dělení - kontrola int/0, pak konverze na float
    printN("LABEL %s", label_int_div);

    // Kontrola dělení int nulou
    printN("PUSHS LF@%s", rn);
    printN("PUSHS int@0");
    printN("JUMPIFEQS %s", label_div_zero);

    // Konverze int → float pro oba operandy
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    // Dělení po konverzi (ln / rn)
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("DIVS");
    printN("JUMP %s", label_end);

    // Chyba: dělení nulou
    printN("LABEL %s", label_div_zero);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    // Chyba: nekompatibilní typy
    printN("LABEL %s", label_err);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    // Konec dělení
    printN("LABEL %s", label_end);
}
void emitEqual(AstNode *node, FuncContext *funcStrukt) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStrukt);
    }
    // Příprava dočasných proměnných a labelů
    int  id = node->tmpId;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStrukt->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStrukt->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStrukt->func_name, id);

    char label_ok[128], label_end[128], label_err[128];
    snprintf(label_ok, sizeof label_ok, "%s$ok$%d", funcStrukt->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end_equal$%d", funcStrukt->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$err$%d", funcStrukt->func_name, id);

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
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // Typy jsou stejné - nyní zkontrolujeme konkrétní typ
    printN("LABEL %s", label_ok);
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("EQS");
    printN("JUMP %s", label_end);

    printN("LABEL %s", label_err);
    printN("PUSHS bool@false");
    printN("LABEL %s", label_end);
}

void emitLess(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);

    char label_ok[128], label_end[128], label_err[128], label_l_float[128], label_r_float[128], label_r_check[128];

    snprintf(label_ok, sizeof label_ok, "%s$le_ok$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$le_end$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$le_err$%d", funcStruct->func_name, id);
    snprintf(label_l_float, sizeof label_l_float, "%s$le_l_f$%d", funcStruct->func_name, id);
    snprintf(label_r_float, sizeof label_r_float, "%s$le_r_f$%d", funcStruct->func_name, id);
    snprintf(label_r_check, sizeof label_r_check, "%s$le_r_chk$%d", funcStruct->func_name, id);

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
    printN("JUMPIFEQS %s", label_r_check);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_l_float);

    printN("JUMP %s", label_err);

    printN("LABEL %s", label_l_float);
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);

    printN("LABEL %s", label_r_check);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", label_ok);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_r_float);
    printN("JUMP %s", label_err);

    printN("LABEL %s", label_r_float);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    printN("LABEL %s", label_ok);

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("LTS");
    printN("JUMP %s", label_end);

    // Chyba: nil nebo různé typy
    printN("LABEL %s", label_err);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    printN("LABEL %s", label_end);
}
void emitGreater(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);

    char label_ok[128], label_end[128], label_err[128], label_l_float[128], label_r_float[128], label_r_check[128];

    snprintf(label_ok, sizeof label_ok, "%s$le_ok$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$le_end$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$le_err$%d", funcStruct->func_name, id);
    snprintf(label_l_float, sizeof label_l_float, "%s$le_l_f$%d", funcStruct->func_name, id);
    snprintf(label_r_float, sizeof label_r_float, "%s$le_r_f$%d", funcStruct->func_name, id);
    snprintf(label_r_check, sizeof label_r_check, "%s$le_r_chk$%d", funcStruct->func_name, id);

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
    printN("JUMPIFEQS %s", label_r_check);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_l_float);

    printN("JUMP %s", label_err);

    printN("LABEL %s", label_l_float);
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);

    printN("LABEL %s", label_r_check);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", label_ok);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_r_float);
    printN("JUMP %s", label_err);

    printN("LABEL %s", label_r_float);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    printN("LABEL %s", label_ok);

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("GTS");
    printN("JUMP %s", label_end);

    // Chyba: nil nebo různé typy
    printN("LABEL %s", label_err);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    printN("LABEL %s", label_end);
}

void emit_less_equal(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);

    char label_ok[128], label_end[128], label_err[128], label_l_float[128], label_r_float[128], label_r_check[128];

    snprintf(label_ok, sizeof label_ok, "%s$le_ok$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$le_end$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$le_err$%d", funcStruct->func_name, id);
    snprintf(label_l_float, sizeof label_l_float, "%s$le_l_f$%d", funcStruct->func_name, id);
    snprintf(label_r_float, sizeof label_r_float, "%s$le_r_f$%d", funcStruct->func_name, id);
    snprintf(label_r_check, sizeof label_r_check, "%s$le_r_chk$%d", funcStruct->func_name, id);

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
    printN("JUMPIFEQS %s", label_r_check);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_l_float);

    printN("JUMP %s", label_err);

    printN("LABEL %s", label_l_float);
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);

    printN("LABEL %s", label_r_check);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", label_ok);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_r_float);
    printN("JUMP %s", label_err);

    printN("LABEL %s", label_r_float);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    printN("LABEL %s", label_ok);

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
    printN("JUMP %s", label_end);

    printN("LABEL %s", label_err);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    printN("LABEL %s", label_end);
}
void emit_greater_equal(AstNode *node, FuncContext *funcStruct) {
    for (int i = 0; i < 2; i++) {
        emitCommand(node->child[i], funcStruct);
    }
    int  id = node->tmpId;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);

    char label_ok[128], label_end[128], label_err[128], label_l_float[128], label_r_float[128], label_r_check[128];

    snprintf(label_ok, sizeof label_ok, "%s$le_ok$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$le_end$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$le_err$%d", funcStruct->func_name, id);
    snprintf(label_l_float, sizeof label_l_float, "%s$le_l_f$%d", funcStruct->func_name, id);
    snprintf(label_r_float, sizeof label_r_float, "%s$le_r_f$%d", funcStruct->func_name, id);
    snprintf(label_r_check, sizeof label_r_check, "%s$le_r_chk$%d", funcStruct->func_name, id);

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
    printN("JUMPIFEQS %s", label_r_check);

    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_l_float);

    printN("JUMP %s", label_err);

    printN("LABEL %s", label_l_float);
    printN("INT2FLOAT LF@%s LF@%s", ln, ln);

    printN("LABEL %s", label_r_check);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", label_ok);
    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_r_float);
    printN("JUMP %s", label_err);

    printN("LABEL %s", label_r_float);
    printN("INT2FLOAT LF@%s LF@%s", rn, rn);

    printN("LABEL %s", label_ok);
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("GTS");

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("EQS");
    printN("ORS");
    printN("JUMP %s", label_end);
    printN("JUMP %s", label_end);

    printN("LABEL %s", label_err);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    printN("LABEL %s", label_end);
}

void emitEqualNot(AstNode *node, FuncContext *funcStruct) {
    emitEqual(node, funcStruct);
    printN("NOTS");
}

//&& neni to obycejny operator
void emitAnd(AstNode *node, FuncContext *funcContext) {
    int  id = node->tmpId;
    char label_false[128], label_end[128];
    snprintf(label_false, sizeof label_false, "%s$and_false$%d", funcContext->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$and_end$%d", funcContext->func_name, id);

    char left_tmp[128];
    snprintf(left_tmp, sizeof left_tmp, "%s_and_left_%d", funcContext->func_name, id);

    emitCondition(node->child[0], funcContext);
    printN("POPS LF@%s", left_tmp);

    // short circuit: if LHS == false → false
    printN("PUSHS LF@%s", left_tmp);
    printN("PUSHS bool@false");
    printN("JUMPIFEQS %s", label_false);

    // short circuit: if LHS == nil → false
    printN("PUSHS LF@%s", left_tmp);
    printN("PUSHS nil@nil");
    printN("JUMPIFEQS %s", label_false);

    // LHS == true → evaluate RHS
    emitCondition(node->child[1], funcContext);
    printN("JUMP %s", label_end);

    printN("LABEL %s", label_false);
    printN("PUSHS bool@false");

    printN("LABEL %s", label_end);
}
void emitOr(AstNode *node, FuncContext *funcContext) {
    int  id = node->tmpId;
    char label_true[128], label_end[128];
    snprintf(label_true, sizeof label_true, "%s$or_success$%d", funcContext->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$or_end$%d", funcContext->func_name, id);

    char left_tmp[128];
    snprintf(left_tmp, sizeof left_tmp, "%s_or_left_%d", funcContext->func_name, id);

    emitCondition(node->child[0], funcContext);
    printN("POPS LF@%s", left_tmp);

    printN("PUSHS LF@%s", left_tmp);
    printN("PUSHS bool@true");
    printN("JUMPIFEQS %s", label_true);

    emitCondition(node->child[1], funcContext);
    printN("JUMP %s", label_end);

    printN("LABEL %s", label_true);
    printN("PUSHS bool@true");

    printN("LABEL %s", label_end);
}
void emitNegace(AstNode *node, FuncContext *funcStruct) {
    int  id = node->tmpId;
    char value[128], value_type[128];
    snprintf(value, sizeof value, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(value_type, sizeof value, "%s_ty_%d", funcStruct->func_name, id);

    char label_false[128], label_end[128];
    snprintf(label_false, sizeof label_false, "%s$and_not_false$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$and_end$%d", funcStruct->func_name, id);
    emitCondition(node->child[0], funcStruct);
    // Stack: [left_value]
    printN("NOTS");
    // Stack: [result] (buď left pokud false/nil, nebo right)
}

void emitUnaryMinus(AstNode *node, FuncContext *funcStrukt) {
    emitCommand(node->child[0], funcStrukt);
    int id = node->tmpId;

    char ln[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStrukt->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStrukt->func_name, id);

    char label_ok[128];
    char label_ok_float[128];
    char label_end[128];
    snprintf(label_ok, sizeof label_ok, "%s$ok$%d", funcStrukt->func_name, id);
    snprintf(label_ok_float, sizeof label_ok_float, "%s$ok_float$%d", funcStrukt->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end_Minus$%d", funcStrukt->func_name, id);

    // Vyndej hodnotu a ulož ji
    printN("POPS LF@%s", ln);

    // Zkontroluj typ
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", ty);

    // Kontrola na int
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_ok);

    // Kontrola na float
    printN("PUSHS LF@%s", ty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", label_ok_float);

    // Error - nejde o int ani float
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);

    // OK label - proveď operaci
    printN("LABEL %s", label_ok);
    printN("PUSHS int@0");
    printN("PUSHS LF@%s", ln);
    printN("SUBS");
    printN("JUMP %s", label_end);

    printN("LABEL %s", label_ok_float);
    double nula = 0.0;
    printN("PUSHS float@%a", nula);
    printN("PUSHS LF@%s", ln);
    printN("SUBS");
    printN("LABEL %s", label_end);
}
void emitIs(AstNode *node, FuncContext *funcStruct) {
    emitCommand(node->child[0], funcStruct);
    int  id = node->tmpId;
    char ln[128], lty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    char label_num[128], label_string[128], label_null[128];
    char label_ok[128], label_err[128], label_end[128];
    snprintf(label_num, sizeof label_num, "%s$Num$%d", funcStruct->func_name, id);
    snprintf(label_string, sizeof label_string, "%s$String$%d", funcStruct->func_name, id);
    snprintf(label_null, sizeof label_null, "%s$Null$%d", funcStruct->func_name, id);
    snprintf(label_ok, sizeof label_ok, "%s$ok$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$err$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end_is$%d", funcStruct->func_name, id);

    // Pop levého operandu do LF@ln
    printN("POPS LF@%s", ln);

    // Zjisti typ levého operandu
    printN("PUSHS LF@%s", ln);
    printN("TYPES");
    printN("POPS LF@%s", lty);

    switch (node->child[1]->kind) {
        case AST_KEYWORD_NUM:  // "Num"
            printN("JUMP %s", label_num);
            break;
        case AST_KEYWORD_STRING:  // "String"
            printN("JUMP %s", label_string);
            break;
        case AST_NULL:  // "Null"
            printN("JUMP %s", label_null);
            break;
        default:
            printN("PUSHS bool@false");
            return;
    }

    // ---- Num ----
    printN("LABEL %s", label_num);
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_ok);
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // ---- String ----
    printN("LABEL %s", label_string);
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // ---- Null ----
    printN("LABEL %s", label_null);
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // ---- výsledek ----
    printN("LABEL %s", label_ok);
    printN("PUSHS bool@true");
    printN("JUMP %s", label_end);

    printN("LABEL %s", label_err);
    printN("PUSHS bool@false");

    printN("LABEL %s", label_end);
}
void emitAssigned(AstNode *node, FuncContext *funcStruct) {
    if (node->child[0]->kind == AST_SETTER_CALL) {
        emitCommand(node->child[1], funcStruct);
        emitSetterCall(node->child[0]);
        return;
    }

    emitCommand(node->child[1], funcStruct);
    printN("POPS LF@%s", node->child[0]->value.str);
}
/*
 *opravi
 */
void emitVar(AstNode *node, FuncContext *funcStruct) {
    if (node->child[1]->kind != AST_EMPTY) {
        emitCommand(node->child[1]->child[0], funcStruct);
        printN("POPS LF@%s", node->child[0]->value.str);
    }
}
void emitGLobal(AstNode *node, FuncContext *funcStruct) {
    if (node->child[1]->kind != AST_EMPTY) {
        emitCommand(node->child[1]->child[0], funcStruct);
        printN("POPS GF@%s", node->child[0]->value.str);
    }
}
void emitIfChain(AstNode *node, FuncContext *funcContext) {
    /**
     * vytvorenie lablu na spode if chainu na ktory v pripade splnenia podmienky
     * budeme z jednotlivych podmienok skakat
     */
    char *endLabel = generatorLable("ifchainend", funcContext, false, if_counter);

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
void emitIf(AstNode *node, FuncContext *funcContext, char *endLabel) {
    if_counter++;
    char *label = NULL;
    label       = generatorLable("ifEnd", funcContext, false, if_counter);
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

void emitElse(AstNode *node, FuncContext *funcContext) { emitChildren(node->child[0], funcContext); }

/**
 * @brief Funkce pro zpracovani while cyklu
 * @param node Ukazatel na aktualne
 */
void emitWhile(AstNode *node, FuncContext *funcContext) {
    loop_counter++;
    char *block_prefix = generatorLable("while", funcContext, true, loop_counter);
    char *labelStart   = generatorLable("start", funcContext, false, loop_counter);
    printN("LABEL %s", labelStart);
    char *endLabel = NULL;
    endLabel       = generatorLable("end", funcContext, false, loop_counter);
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
    char *popped_prefix = stacklabelStackRead(funcContext->label_stack);
    if (popped_prefix == block_prefix) {
        stacklabelStackPop(funcContext->label_stack);
        free(block_prefix);
    } else {
        printError(INTERNAL_ERROR, "nepovedene uvolneni lablu na konci while loopu");
        memorySafeExit(INTERNAL_ERROR);
    }
    loop_counter--;
}

void emitBreak(FuncContext *funcContext) {
    char *label = generatorLable("end", funcContext, false, loop_counter);
    printN("JUMP %s", label);
    free(label);
}

void emitContinue(FuncContext *funcContext) {
    char *label = generatorLable("start", funcContext, false, loop_counter);
    printN("JUMP %s", label);
    free(label);
}
void emitBool(AstNode *node, FuncContext *funcContext) {
    if (node->kind == AST_IDENTIFIER || node->kind == AST_GLOBAL_ID) {
        char ty_var[128], val_var[128];
        char is_false_label[128], end_label[128];
        char is_type_bool_label[128];
        int  id = node->tmpId;

        snprintf(ty_var, sizeof ty_var, "%s_ty_bool_%d", funcContext->func_name, id);
        snprintf(val_var, sizeof val_var, "%s_val_bool_%d", funcContext->func_name, id);
        snprintf(is_false_label, sizeof is_false_label, "%s$bool_is_false$%d", funcContext->func_name, id);
        snprintf(end_label, sizeof end_label, "%s$bool_end$%d", funcContext->func_name, id);
        snprintf(is_type_bool_label, sizeof is_type_bool_label, "%s$bool_type_ok$%d", funcContext->func_name, id);

        const char *var_name = node->value.str;

        // Získání hodnoty a typu
        if (node->kind == AST_IDENTIFIER) {
            printN("PUSHS LF@%s", var_name);
        } else {
            printN("PUSHS GF@%s", var_name);
        }
        printN("POPS LF@%s", val_var);
        printN("PUSHS LF@%s", val_var);
        printN("TYPES");
        printN("POPS LF@%s", ty_var);

        // --- KONTROLA 1: NIL ---
        printN("PUSHS LF@%s", ty_var);
        printN("PUSHS string@nil");
        printN("JUMPIFEQS %s", is_false_label);

        // --- KONTROLA 2: TYP NENÍ BOOL ---
        printN("PUSHS LF@%s", ty_var);
        printN("PUSHS string@bool");
        printN("JUMPIFEQS %s", is_type_bool_label);

        // --- TRUE případ (jiný typ než nil a bool) ---
        printN("PUSHS bool@true");
        printN("JUMP %s", end_label);

        // --- KONTROLA 3: HODNOTA BOOL@FALSE
        printN("LABEL %s", is_type_bool_label);
        printN("PUSHS LF@%s", val_var);
        printN("PUSHS bool@false");
        printN("JUMPIFEQS %s", is_false_label);

        // --- TRUE případ (bool@true) ---
        printN("PUSHS bool@true");
        printN("JUMP %s", end_label);

        // --- FALSE případy (nil nebo bool@false) ---
        printN("LABEL %s", is_false_label);
        printN("PUSHS bool@false");

        // --- Konec ---
        printN("LABEL %s", end_label);
    }
}
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
 *
 */
void emitFor(AstNode *node, FuncContext *funcContext) {
    loop_counter++;

    char *block_prefix = generatorLable("for", funcContext, true, loop_counter);

    char *errorLabel         = generatorLable("TypesError", funcContext, false, loop_counter);
    bool  generateErrorLabel = false;

    char *labelStart = generatorLable("start", funcContext, false, loop_counter);

    char *endLabel = generatorLable("end", funcContext, false, loop_counter);

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
    char *popped_prefix = stacklabelStackRead(funcContext->label_stack);
    if (popped_prefix == block_prefix) {
        stacklabelStackPop(funcContext->label_stack);
        free(block_prefix);  // Tímto uvolňujeme alokovanou paměť
    } else {
        printError(INTERNAL_ERROR, "nepovedene uvolneni lablu na konci while loopu");
        memorySafeExit(INTERNAL_ERROR);
    }
    loop_counter--;
}

void emitSetterCall(AstNode *node) { printN("CALL %s", node->value.str); }

void emitSetter(AstNode *node) {
    DynamiclabelStackStackPtr labelStack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    FuncContext              *context    = constuctorFuncContext();
    context->func_name                   = node->child[0]->value.str;

    context->label_stack = labelStack;
    char *labelEnd       = generatorLable("setterEnd", context, true, 0);
    context->label_end   = labelEnd;
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
        printN("DEFVAR LF@%s", localVariables->var_names[i]);
    }

    for (size_t i = 0; i < node->child[2]->child[0]->arity; i++) {
        emitCommand(node->child[2]->child[0]->child[i], context);
    }
    printN("LABEL %s", context->label_end);
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

void emitGetterCall(AstNode *node) { printN("CALL %s", node->value.str); }

void emitGetter(AstNode *node) {
    DynamiclabelStackStackPtr labelStack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);

    FuncContext *context = constuctorFuncContext();
    context->func_name   = node->child[0]->value.str;

    context->label_stack = labelStack;
    char *labelEnd       = generatorLable("getterEnd", context, true, 0);
    context->label_end   = labelEnd;
    printN("LABEL %s", node->child[0]->value.str);
    printN("CREATEFRAME");
    printN("PUSHFRAME");

    printIfj();
    VarList *localVariables = varListCreate();
    varListAdd(localVariables, "retval");
    collectLocalVars(node->child[2], localVariables, context);  // funkce uvnitr
    for (int i = 0; i < localVariables->count; i++) {
        printN("DEFVAR LF@%s", localVariables->var_names[i]);
    }
    printN("MOVE LF@retval nil@nil");
    for (size_t i = 0; i < node->child[2]->child[0]->arity; i++) {
        emitCommand(node->child[2]->child[0]->child[i], context);
    }
    printN("LABEL %s", context->label_end);
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
void emitTernarni(AstNode *node, FuncContext *funcStruct) {
    char label_true[128], label_end[128];
    int  id = node->tmpId;
    snprintf(label_true, sizeof label_true, "%s$true$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end$%d", funcStruct->func_name, id);
    emitCondition(node->child[0], funcStruct);
    printN("PUSHS bool@true");
    printN("JUMPIFEQS %s", label_true);
    emitCommand(node->child[2], funcStruct);
    printN("JUMP %s", label_end);
    printN("LABEL %s", label_true);
    emitCommand(node->child[1], funcStruct);
    printN("LABEL %s", label_end);
}
/*
 * @brief switch zajisti zavolani sablon
 * @params ukazatel na uzel
 * @params ukazatel na hash_table
 */
/*
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
void codegenGenerate(AstNode *root) {
    FuncContext *funcStruct = constuctorFuncContext();
    emitChildren(root, funcStruct);
    destructorFuncContext(funcStruct);
    /*
    printN("EXIT %s", NO_ERROR);
    printN("LABEL error %s", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    printN("EXIT %s", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    printN("LABEL error%s", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    printN("EXIT %s", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    */
}
