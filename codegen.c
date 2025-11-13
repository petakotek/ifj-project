#include "codegen.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast_strom.h"
#include "dynamic_stack.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analysis.h"
#include "support_functions.h"
#define MAX_LABELS 1000

/*
 *Print funkce na zajistneti \node na konci
 */
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
void emit_prolog() { printN(".IFJcode25"); }
/**
 *@buffer na jednotlive labely aby se nakonci uvolnovali
 */
char *buffer_pointer_label[MAX_LABELS];
char *resize_label_name(char *prefix, size_t *size) {
    *size *= 2;
    prefix = realloc(prefix, *size);
    if (prefix == NULL) {
        printError(INTERNAL_ERROR, "error in resize_label_reallock");
        memorySafeExit(INTERNAL_ERROR);
    }
    return prefix;
}
/**
 * @brief generator jednotlivych labelu. Label je tvoren z nazvu lablu ktere existovali ve vetvi
 * prednim priklad: fun$1$while$2 je label druheho while loopu ve funkci fun fun$1$while$2$end je
 * koncovy label toho while loopu
 * @param label_stack stack na ukladani prubezne lablu
 * @param func_name jmeno funcke ktere chceme pripojit
 * @return ukazatel na vysledny label
 */
char *generatorLable(char *func_name,FuncContext *funcStruckt,bool pridaniDoStacku) {
    size_t size_of_buffer = 128;
    char  *result_label   = (char *)malloc(size_of_buffer);
    if (result_label == NULL) {
        printError(INTERNAL_ERROR, "Error allocating memory for label");
        memorySafeExit(INTERNAL_ERROR);
    }

    int current_len = 0;

    // Získání existujícího prefixu ze zásobníku (pokud existuje)
    if (!stacklabelStackIsEmpty(funcStruckt->label_stack)) {
        char  *prefix     = stacklabelStackRead(funcStruckt->label_stack);
        size_t prefix_len = strlen(prefix);

        // ve chvili kdyby nas buffer byl uz na zacatku maly
        if (prefix_len + 1 + 10 >= size_of_buffer) {
            result_label = resize_label_name(result_label, &size_of_buffer);
            if (result_label == NULL) {
                printError(INTERNAL_ERROR, "Error resizing label buffer");
                memorySafeExit(INTERNAL_ERROR);
            }
        }

        // Zkopírování existujícího prefixu: "fun$1$while$1" to co jsme precetli ze stacku
        current_len = snprintf(result_label, size_of_buffer, "%s", prefix);
        if (current_len < 0 || (size_t)current_len >= size_of_buffer) {
            free(result_label);
            printError(INTERNAL_ERROR, "Error during initial label copy");
            memorySafeExit(INTERNAL_ERROR);
        }
    }

    // 2. Přidání nového bloku jako suffix: "$if$3"
    // Formát: $TypBloku$CisloBloku
    size_t remaining_space = size_of_buffer - current_len;

    int written =
        snprintf(result_label + current_len, remaining_space, "$%s$%d", funcStruckt->func_name, funcStruckt->counter);

    if (written <= 0 || (size_t)written >= remaining_space) {
        // potřebna resize
        if ((size_t)written >= remaining_space) {
            result_label = resize_label_name(result_label, &size_of_buffer);
            if (result_label == NULL) {
                printError(INTERNAL_ERROR, "Error resizing label buffer (suffix)");
                memorySafeExit(INTERNAL_ERROR);
            }
            remaining_space = size_of_buffer - current_len;
            // znovu zapsani
            written =
                snprintf(result_label + current_len, remaining_space, "$%s$%d", funcStruckt->func_name, funcStruckt->counter);
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
/*
 *@brief zajisti aby globalni promene byly inicializovany hned na zacatku
 *@param ukazatel na hash_table
 */
/*
 *@brief zajisti rozbaleni uzlu
 *@params ukazatel na uzel
 *@params ukazatel na hash_table
 */
void emitChildren(AstNode *root,FuncContext *funcStruct) {
    for (int i = 0; i < root->arity; i++) {
        emit_command(root->child[i],funcStruct);
    }
}
/*
* @brief funkce na zalozeni struktury na zapsani var promenych
*/
VarList* varListCreate() {
    VarList *list = malloc(sizeof(VarList));
    list->capacity = 16;
    list->count = 0;
    list->var_names = calloc(1,sizeof(char*) * list->capacity);
    return list;
}
/*
*@brief funcke na pridavani do varlistu i s resize
*/
void varlist_add(VarList *list, const char *name) {
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->var_names[i], name) == 0) {
            return;
        }
    }
   //az moc promenych resize
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->var_names = realloc(list->var_names, sizeof(char*) * list->capacity);
    }
    list->var_names[list->count] = strdup(name);
    list->count++;
}
/*
*@brief funkce na niceni listu volano na konci kazde funkce
*/
void varListDescrution(VarList *list){
	for(int i = 0;i <list->count;i++){
		free(list->var_names[i]);
	}
	free(list->var_names);
	free(list);
}

void add_binop_tmps(VarList* vars, const char* func, int id,AstNode *node) {
    char buf[128];

    if(node->kind == AST_PLUS){
   	snprintf(buf, sizeof buf, "%s_ln_%d", func, id);
       varlist_add(vars, buf);
       snprintf(buf, sizeof buf, "%s_rn_%d", func, id);
       varlist_add(vars, buf);
       snprintf(buf, sizeof buf, "%s_ty_%d", func, id);
       varlist_add(vars, buf);
	    snprintf(buf, sizeof buf, "%s_rs_%d", func, id);
	    varlist_add(vars, buf);
    }else if(node->kind == AST_LT){
   		snprintf(buf, sizeof buf, "%s_ln_%d", func, id);
       	varlist_add(vars, buf);
        snprintf(buf, sizeof buf, "%s_rn_%d", func, id);
        varlist_add(vars, buf);
   		snprintf(buf, sizeof buf, "%s_rty_%d", func, id);
	    varlist_add(vars, buf);
    	snprintf(buf, sizeof buf, "%s_lty_%d", func, id);
	    varlist_add(vars, buf);
    }else{
    	snprintf(buf, sizeof buf, "%s_ln_%d", func, id);
        varlist_add(vars, buf);
        snprintf(buf, sizeof buf, "%s_rn_%d", func, id);
        varlist_add(vars, buf);
        snprintf(buf, sizeof buf, "%s_ty_%d", func, id);
        varlist_add(vars, buf);
    }
}
FuncContext *ConstuctorFuncContext(){
	FuncContext *funcStruct = malloc(sizeof(FuncContext));
	if(!funcStruct){
		printError(INTERNAL_ERROR, "chyba v alokaci");
		memorySafeExit(INTERNAL_ERROR);
	}
	funcStruct->func_name = NULL;
	funcStruct->label_stack = NULL;
	funcStruct->label_end = NULL;
	funcStruct->counter = 0;
	funcStruct->value_counter = 0;
	return funcStruct;
}
void DestructorFuncContext(FuncContext *funcStruct){
	free(funcStruct);
}
void collectLocalVars(AstNode *node, VarList *vars,FuncContext *funcStruct) {
    if (node == NULL) return;
    switch (node->kind) {
          case AST_VAR:
              if (node->child[0] && node->child[0]->kind == AST_IDENTIFIER) {
                  varlist_add(vars, node->child[0]->value.str);
              }
              break;

          case AST_PARAM:
              break;

          case AST_PLUS:
          case AST_MINUS:
          case AST_MUL:
          case AST_DIV:
          case AST_EQ:
          case AST_LT:
          case AST_GT:
          case AST_LTE:
          case AST_GTE:
          case AST_AND:
          case AST_OR: {
              node->tmp_id = funcStruct->value_counter++;
              add_binop_tmps(vars, funcStruct->func_name, node->tmp_id,node);
              break;
          }

          case AST_NOT: {
              node->tmp_id = funcStruct->value_counter++;
              // např. jen typ a jedna hodnota:
              char buf[128];
              snprintf(buf, sizeof buf, "%s_val_%d", funcStruct->func_name, node->tmp_id);
              varlist_add(vars, buf);
              snprintf(buf, sizeof buf, "%s_ty_%d", funcStruct->func_name, node->tmp_id);
              varlist_add(vars, buf);
              break;
          }

          default:
              break;
      }

      for (int i = 0; i < node->arity; i++) {
          collectLocalVars(node->child[i], vars, funcStruct);
      }
  }
void emitReturn(AstNode *root,FuncContext *funcStrukt){
	if(root->child[0]->kind == AST_IDENTIFIER){
		printN("PUSHS LF@%s",root->child[0]->value.str);
	}else{
	emitChildren(root,funcStrukt);
	}
	printN("POPS LF@retval");
	printN("JUMP %s",funcStrukt->label_end);
}
void emitFuncDec(AstNode *root){
	DynamiclabelStackStackPtr label_stack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);
	bool main = false;
	char *label = NULL;
	FuncContext *context = ConstuctorFuncContext();
	context->func_name = root->child[0]->value.str;
	context->label_end = NULL;
	context->label_stack = label_stack;
	if(strcmp(root->child[0]->value.str,"main") == 0 && root->child[1]->child[0]->kind == AST_EMPTY){
		main = true;
		printN("LABEL $$main");
	}else{
	label = generatorLable(root->child[0]->value.str,context,true);
	printN("LABEL %s",label);
	}
	char *label_end = generatorLable("end", context,false);
	context->label_end = label_end;
	printN("CREATEFRAME");
	printN("PUSHFRAME");
	//sbirani lokalnich promenych na deklaraci
	VarList *local_vars = varListCreate();
    varlist_add(local_vars, "retval");
	collectLocalVars(root->child[2], local_vars,context);//funkce uvnitr
	for (int i = 0; i < local_vars->count; i++) {
	    printN("DEFVAR LF@%s", local_vars->var_names[i]);
	}
	AstNode *params = root->child[1];
	if(params->child[0]->kind != AST_EMPTY){
	for (int i = 0; i < params->arity-1; i++) {
    	const char *pname = params->child[i]->value.str;
     	printN("DEFVAR LF@%s", pname);
      	printN("POPS LF@%s", pname);
	}
	}
	printN("MOVE LF@retval nil@nil");
	emitChildren(root->child[2], context);
	if(!main){
		printN("LABEL %s",label_end);
		printN("PUSHS LF@retval");
		printN("POPFRAME");
		printN("RETURN");
		free(label);
	}else{
		printN("EXIT int@0");
	}
	varListDescrution(local_vars);
	stacklabelStackDestruct(label_stack);
	DestructorFuncContext(context);
	}
void emit_call_func(AstNode *node,FuncContext *funcStruct){
	printN("CREATEFRAME");
	for(int i = 0; i < node->child[1]->arity;i++){
	emit_command(node->child[1]->child[i],funcStruct);
	}
	printN("CALL $%s$0",node->child[0]->value.str);
}
void emitIfjFunc(AstNode *node,FuncContext *funcStruct) {
                 AstNode *callee = node->child[0];
                 AstNode *params = node->child[1];

    if (callee->kind == AST_IDENTIFIER && strcmp(callee->value.str, "write") == 0) {
        // Projdi všechny parametry
        for (int i = 0; i < params->arity; i++) {
            AstNode *arg = params->child[i];
            if (arg->kind == AST_EMPTY) continue;

            if (arg->kind == AST_IDENTIFIER) {
                printN("WRITE LF@%s", arg->value.str);
            } else if (arg->kind == AST_STRING_LIT) {
                printN("WRITE string@%s", arg->value.str);
            } else if (arg->kind == AST_INT_LIT) {
                printN("WRITE int@%lld", arg->value.int_val);
            } else if (arg->kind == AST_FLOAT_LIT) {
                printN("WRITE float@%a", arg->value.float_val);
            } else {
            /*
                emit_command(arg, label_stack, counter,  func_name);
                char tmp[64];
                snprintf(tmp, sizeof tmp, "%s$wtmp_%d_%d", func_name, node->tmp_id, i);
                printN("POPS LF@%s", tmp);
                printN("WRITE LF@%s", tmp);
                */
            }
        }
        return;
    }


}
/*
 * Dostaneme zasobnik zkontrolujeme zda je neco na vrchu pokud jo udelame types coz nam da na vrchol
 * typ na vrchol a potom si pushneme dite a dame na nej types zasobnik type promena (dite) type
 * promena ze zasobnika
 * JUMPIFEQS/JUMPIFNEQS ⟨label⟩
 */
 void emitPlus(AstNode *node,FuncContext *funcStruct) {
     if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStruct);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStruct);
     }
    int id = node->tmp_id;
    char ln[128], rn[128], ty[128],rs[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStruct->func_name, id);
    snprintf(rs, sizeof rs, "%s_rs_%d", funcStruct->func_name, id);

    int size_for_label = strlen(funcStruct->func_name)+20;
    char label_ok[size_for_label], label_add[size_for_label], label_concat[size_for_label], label_end[size_for_label], label_err[size_for_label];
    snprintf(label_ok,  sizeof label_ok,  "%s$ok$%d",     funcStruct->func_name, id);
    snprintf(label_add, sizeof label_add, "%s$add$%d",    funcStruct->func_name, id);
    snprintf(label_concat, sizeof label_concat, "%s$concat$%d", funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$end$%d",    funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$err$%d",    funcStruct->func_name, id);
    //  Ulozeni do docasnych promenychg
    printN("POPS LF@%s",rn);  // Uložíme pravou hodnotu (right)
    printN("POPS LF@%s",ln);  // Uložíme levou hodnotu (left)

    // vratime zpatky
    printN("PUSHS LF@%s",ln);
    printN("PUSHS LF@%s",rn);
    // Typová kontrola pomocí zásobníku
    // Stav zásobníku: [left_value, right_value]

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");  // zásobník: [left_value,	right_type]

    // Dočasně uložíme typ pravého operandu
    printN("POPS LF@%s",ty);  // zásobník: [left_value]

    // Zjistíme typ levého operandu (left)
    printN("TYPES");  // zásobník: [type_left]

    // Vrátíme typ pravého operandu zpět
    printN("PUSHS LF@%s",ty);  // zásobník: [type_left, type_right]
// Porovnání, že typy jsou stejné
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    printN("LABEL %s", label_ok);
    // Je to string?
    printN("PUSHS LF@%s", ty);          // [right_type]
    printN("PUSHS string@string");      // [right_type, "string"]
    printN("JUMPIFEQS %s", label_concat);

    // Je to int?
    printN("PUSHS LF@%s", ty);          // [right_type]
    printN("PUSHS string@int");         // [right_type, "int"]
    printN("JUMPIFEQS %s", label_add);

    // Je to float?
    printN("PUSHS LF@%s", ty);          // [right_type]
    printN("PUSHS string@float");       // [right_type, "float"]
    printN("JUMPIFEQS %s", label_add);

    printN("JUMP %s", label_err);
    // --- ADD větev (pro int a float) ---
    printN("LABEL %s", label_add);
    // Musíme znovu načíst původní hodnoty
    printN("PUSHS LF@%s",ln);
    printN("PUSHS LF@%s",rn);
    printN("ADDS");
    printN("JUMP %s", label_end);
    // --- CONCAT větev (pro string) ---
    printN("LABEL %s", label_concat);
    printN("CONCAT LF@%s LF@%s LF@%s",ln,rn,rs);
    printN("PUSHS LF@%s",rs);
    printN("JUMP %s", label_end);

    // --- CHYBA ---
    printN("LABEL %s", label_err);
    printN("EXIT int@4");
    // --- KONEC ---
    printN("LABEL %s", label_end);
	printN("");
}
void emitMinus(AstNode *node, FuncContext *funcStrukt) {
	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStrukt);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStrukt);
     }
        // Po emitBinaryOp máme na zásobníku: [hodnota_left, hodnota_right]
        int id = node->tmp_id;
        char ln[128], rn[128], ty[128];
        snprintf(ln, sizeof ln, "%s_ln_%d", funcStrukt->func_name, id);
        snprintf(rn, sizeof rn, "%s_rn_%d", funcStrukt->func_name, id);
        snprintf(ty, sizeof ty, "%s_ty_%d", funcStrukt->func_name, id);

        int size_for_label = strlen(funcStrukt->func_name)+20;
        char label_ok[size_for_label], label_add[size_for_label], label_concat[size_for_label], label_end[size_for_label], label_err[size_for_label];
        snprintf(label_ok,  sizeof label_ok,  "%s$ok$%d",     funcStrukt->func_name, id);
        snprintf(label_end, sizeof label_end, "%s$end$%d",    funcStrukt->func_name, id);
        snprintf(label_err, sizeof label_err, "%s$err$%d",    funcStrukt->func_name, id);
        //  Ulozeni do docasnych promenychg
        printN("POPS LF@%s",rn);  // Uložíme pravou hodnotu (right)
        printN("POPS LF@%s",ln);  // Uložíme levou hodnotu (left)

        // vratime zpatky
        printN("PUSHS LF@%s",ln);
        printN("PUSHS LF@%s",rn);
        // Typová kontrola pomocí zásobníku
        // Stav zásobníku: [left_value, right_value]

        // Zjistíme typ pravého operandu (right)
        printN("TYPES");  // zásobník: [left_value,	right_type]

        // Dočasně uložíme typ pravého operandu
        printN("POPS LF@%s",ty);  // zásobník: [left_value]

        // Zjistíme typ levého operandu (left)
        printN("TYPES");  // zásobník: [type_left]

        // Vrátíme typ pravého operandu zpět
        printN("PUSHS LF@%s",ty);  // zásobník: [type_left, type_right]
    // Porovnání, že typy jsou stejné
        printN("JUMPIFEQS %s", label_ok);
        printN("JUMP %s", label_err);

        printN("LABEL %s", label_ok);
        printN("PUSHS LF@%s", ty);
        printN("PUSHS string@string");
        printN("JUMPIFEQS %s", label_err);

        printN("PUSHS LF@%s",ln);
        printN("PUSHS LF@%s",rn);
        printN("SUBS");
        printN("JUMP %s", label_end);
        // --- CHYBA ---
        printN("LABEL %s", label_err);
        printN("EXIT int@4");
        // --- KONEC ---
        printN("LABEL %s", label_end);
	printN("");
}
void emitMul(AstNode *node,FuncContext *funcStrukt) {
	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStrukt);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStrukt);
     }
    // Po emitBinaryOp máme na zásobníku: [hodnota_left, hodnota_right]
        int id = node->tmp_id;
        char ln[128], rn[128], ty[128];
        snprintf(ln, sizeof ln, "%s_ln_%d", funcStrukt->func_name, id);
        snprintf(rn, sizeof rn, "%s_rn_%d", funcStrukt->func_name, id);
        snprintf(ty, sizeof ty, "%s_ty_%d", funcStrukt->func_name, id);

        int size_for_label = strlen(funcStrukt->func_name)+20;
        char label_ok[size_for_label], label_add[size_for_label], label_concat[size_for_label], label_end[size_for_label], label_err[size_for_label];
        snprintf(label_ok,  sizeof label_ok,  "%s$ok$%d",     funcStrukt->func_name, id);
        snprintf(label_end, sizeof label_end, "%s$end$%d",    funcStrukt->func_name, id);
        snprintf(label_err, sizeof label_err, "%s$err$%d",    funcStrukt->func_name, id);
        //  Ulozeni do docasnych promenychg
        printN("POPS LF@%s",rn);  // Uložíme pravou hodnotu (right)
        printN("POPS LF@%s",ln);  // Uložíme levou hodnotu (left)

        // vratime zpatky
        printN("PUSHS LF@%s",ln);
        printN("PUSHS LF@%s",rn);
        // Typová kontrola pomocí zásobníku
        // Stav zásobníku: [left_value, right_value]

        // Zjistíme typ pravého operandu (right)
        printN("TYPES");  // zásobník: [left_value,	right_type]

        // Dočasně uložíme typ pravého operandu
        printN("POPS LF@%s",ty);  // zásobník: [left_value]

        // Zjistíme typ levého operandu (left)
        printN("TYPES");  // zásobník: [type_left]

        // Vrátíme typ pravého operandu zpět
    // Porovnání, že typy jsou stejné
        printN("PUSHS LF@%s", ty);          // [type_left, type_right]
        printN("JUMPIFEQS %s", label_ok);
        printN("JUMP %s", label_err);
    //nesmito byt string a string
        printN("LABEL %s", label_ok);
        printN("PUSHS LF@%s", ty);
        printN("PUSHS string@string");
        printN("JUMPIFEQS %s", label_err);

        printN("PUSHS LF@%s",ln);
        printN("PUSHS LF@%s",rn);
        printN("MULS");
        printN("JUMP %s", label_end);
        // --- CHYBA ---
        printN("LABEL %s", label_err);
        printN("EXIT int@4");
        // --- KONEC ---
        printN("LABEL %s", label_end);
	printN("");
}
void emitDiv(AstNode *node,FuncContext *funcStrukt) {
	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStrukt);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStrukt);
     }
    // Příprava dočasných proměnných a labelů
    int id = node->tmp_id;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStrukt->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStrukt->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStrukt->func_name, id);

    char label_ok[128], label_int_div[128], label_div_zero[128], label_end[128], label_err[128];
    snprintf(label_ok,       sizeof label_ok,       "%s$ok$%d",      funcStrukt->func_name, id);
    snprintf(label_int_div,  sizeof label_int_div,  "%s$intdiv$%d",  funcStrukt->func_name, id);
    snprintf(label_div_zero, sizeof label_div_zero, "%s$divzero$%d", funcStrukt->func_name, id);
    snprintf(label_end,      sizeof label_end,      "%s$end$%d",     funcStrukt->func_name, id);
    snprintf(label_err,      sizeof label_err,      "%s$err$%d",     funcStrukt->func_name, id);

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
    printN("EXIT int@57");

    // Chyba: nekompatibilní typy
    printN("LABEL %s", label_err);
    printN("EXIT int@4");

    // Konec dělení
    printN("LABEL %s", label_end);
}
void emit_equal(AstNode *node, FuncContext *funcStrukt) {
	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStrukt);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStrukt);
     }
    // Příprava dočasných proměnných a labelů
    int id = node->tmp_id;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStrukt->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStrukt->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStrukt->func_name, id);

    char label_ok[128],label_end[128], label_err[128];
    snprintf(label_ok,       sizeof label_ok,       "%s$ok$%d",      funcStrukt->func_name, id);
    snprintf(label_end,      sizeof label_end,      "%s$end$%d",     funcStrukt->func_name, id);
    snprintf(label_err,      sizeof label_err,      "%s$err$%d",     funcStrukt->func_name, id);

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
    printN("PUSHD LF@%s",ln);
    printN("PUSHS LF@%s",rn);
    printN("EQS");
    printN("JUMP %s",label_end);

    printN("LABEL %s",label_err);
    printN("PUSHS bool@false");
    printN("LABEL %s", label_end);
}

void emit_less(AstNode *node,FuncContext *funcStruct) {
	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStruct);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStruct);
     }

    int id = node->tmp_id;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln,  sizeof ln,  "%s_ln_%d",  funcStruct->func_name, id);
    snprintf(rn,  sizeof rn,  "%s_rn_%d",  funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);

    char label_ok[128], label_end[128], label_err[128];
    snprintf(label_ok,  sizeof label_ok,  "%s$ltok$%d",  funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$ltend$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$lterr$%d", funcStruct->func_name, id);

    // Uložení operandů do dočasných proměnných
    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    // Vrátíme operandy zpět na zásobník pro typovou kontrolu
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");
    printN("POPS LF@%s", rty);

    // Zjistíme typ levého operandu (left)
    printN("TYPES");
    printN("POPS LF@%s", lty);

    // Kontrola: žádný operand nesmí být nil
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_err);

    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_err);

    // Kontrola: typy musí být stejné
    printN("PUSHS LF@%s", lty);
    printN("PUSHS LF@%s", rty);
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // Typy jsou stejné a nenil → porovnáme hodnoty
    printN("LABEL %s", label_ok);
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("LTS");
    printN("JUMP %s", label_end);

    // Chyba: nil nebo různé typy
    printN("LABEL %s", label_err);
    printN("EXIT int@6");

    printN("LABEL %s", label_end);
}

void emit_greater(AstNode *node,FuncContext *funcStruct) {

	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStruct);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStruct);
     }
    int id = node->tmp_id;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln,  sizeof ln,  "%s_ln_%d",  funcStruct->func_name, id);
    snprintf(rn,  sizeof rn,  "%s_rn_%d",  funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);

    char label_ok[128], label_end[128], label_err[128];
    snprintf(label_ok,  sizeof label_ok,  "%s$ltok$%d",  funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$ltend$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$lterr$%d", funcStruct->func_name, id);

    // Uložení operandů do dočasných proměnných
    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    // Vrátíme operandy zpět na zásobník pro typovou kontrolu
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");
    printN("POPS LF@%s", rty);

    // Zjistíme typ levého operandu (left)
    printN("TYPES");
    printN("POPS LF@%s", lty);

    // Kontrola: žádný operand nesmí být nil
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_err);

    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_err);

    // Kontrola: typy musí být stejné
    printN("PUSHS LF@%s", lty);
    printN("PUSHS LF@%s", rty);
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // Typy jsou stejné a nenil → porovnáme hodnoty
    printN("LABEL %s", label_ok);
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("GTS");
    printN("JUMP %s", label_end);

    // Chyba: nil nebo různé typy
    printN("LABEL %s", label_err);
    printN("EXIT int@6");

    printN("LABEL %s", label_end);
}

void emit_less_equal(AstNode *node, FuncContext *funcStruct) {

	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStruct);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStruct);
     }
    int id = node->tmp_id;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln,  sizeof ln,  "%s_ln_%d",  funcStruct->func_name, id);
    snprintf(rn,  sizeof rn,  "%s_rn_%d",  funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);

    char label_ok[128], label_end[128], label_err[128];
    snprintf(label_ok,  sizeof label_ok,  "%s$ltok$%d",  funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$ltend$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$lterr$%d", funcStruct->func_name, id);

    // Uložení operandů do dočasných proměnných
    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    // Vrátíme operandy zpět na zásobník pro typovou kontrolu
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");
    printN("POPS LF@%s", rty);

    // Zjistíme typ levého operandu (left)
    printN("TYPES");
    printN("POPS LF@%s", lty);

    // Kontrola: žádný operand nesmí být nil
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_err);

    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_err);

    // Kontrola: typy musí být stejné
    printN("PUSHS LF@%s", lty);
    printN("PUSHS LF@%s", rty);
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // Typy jsou stejné a nenil → porovnáme hodnoty
    printN("LABEL %s", label_ok);
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("LTS");

    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);
    printN("EQS");
    printN("ORS");
    printN("JUMP %s", label_end);

    // Chyba: nil nebo různé typy
    printN("LABEL %s", label_err);
    printN("EXIT int@6");

    printN("LABEL %s", label_end);
}
void emit_greater_equal(AstNode *node, FuncContext *funcStruct) {

	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStruct);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStruct);
     }
    int id = node->tmp_id;
    char ln[128], rn[128], rty[128], lty[128];
    snprintf(ln,  sizeof ln,  "%s_ln_%d",  funcStruct->func_name, id);
    snprintf(rn,  sizeof rn,  "%s_rn_%d",  funcStruct->func_name, id);
    snprintf(lty, sizeof lty, "%s_lty_%d", funcStruct->func_name, id);
    snprintf(rty, sizeof rty, "%s_rty_%d", funcStruct->func_name, id);

    char label_ok[128], label_end[128], label_err[128];
    snprintf(label_ok,  sizeof label_ok,  "%s$ltok$%d",  funcStruct->func_name, id);
    snprintf(label_end, sizeof label_end, "%s$ltend$%d", funcStruct->func_name, id);
    snprintf(label_err, sizeof label_err, "%s$lterr$%d", funcStruct->func_name, id);

    // Uložení operandů do dočasných proměnných
    printN("POPS LF@%s", rn);
    printN("POPS LF@%s", ln);

    // Vrátíme operandy zpět na zásobník pro typovou kontrolu
    printN("PUSHS LF@%s", ln);
    printN("PUSHS LF@%s", rn);

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");
    printN("POPS LF@%s", rty);

    // Zjistíme typ levého operandu (left)
    printN("TYPES");
    printN("POPS LF@%s", lty);

    // Kontrola: žádný operand nesmí být nil
    printN("PUSHS LF@%s", lty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_err);

    printN("PUSHS LF@%s", rty);
    printN("PUSHS string@nil");
    printN("JUMPIFEQS %s", label_err);

    // Kontrola: typy musí být stejné
    printN("PUSHS LF@%s", lty);
    printN("PUSHS LF@%s", rty);
    printN("JUMPIFEQS %s", label_ok);
    printN("JUMP %s", label_err);

    // Typy jsou stejné a nenil → porovnáme hodnoty
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

    // Chyba: nil nebo různé typy
    printN("LABEL %s", label_err);
    printN("EXIT int@6");

    printN("LABEL %s", label_end);
}
void emit_equal_not(AstNode *node, FuncContext *funcStruct) {
	if (node->child[0]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[0]->value.str);
     }else{
     emit_command(node->child[0], funcStruct);
     }
     if (node->child[1]->kind == AST_IDENTIFIER) {
         printN("PUSHS LF@%s",node->child[1]->value.str);
     }else{
     emit_command(node->child[1], funcStruct);
     }
    int id = node->tmp_id;
    char ln[128], rn[128], ty[128];
    snprintf(ln, sizeof ln, "%s_ln_%d", funcStruct->func_name, id);
    snprintf(rn, sizeof rn, "%s_rn_%d", funcStruct->func_name, id);
    snprintf(ty, sizeof ty, "%s_ty_%d", funcStruct->func_name, id);

    char label_ok[128],label_end[128], label_err[128];
    snprintf(label_ok,       sizeof label_ok,       "%s$ok$%d",      funcStruct->func_name, id);
    snprintf(label_end,      sizeof label_end,      "%s$end$%d",     funcStruct->func_name, id);
    snprintf(label_err,      sizeof label_err,      "%s$err$%d",     funcStruct->func_name, id);

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
    printN("PUSHD LF@%s",ln);
    printN("PUSHS LF@%s",rn);
    printN("EQS");
    printN("NOTS");
    printN("JUMP %s",label_end);

    printN("LABEL %s",label_err);
    printN("PUSHS bool@false");
    printN("LABEL %s", label_end);
}
//&& neni to obycejny operator
void emit_and(AstNode *node, FuncContext *funcContext) {
    int id = node->tmp_id;
    char label_not_false[128], label_end[128];
    snprintf(label_not_false, sizeof label_not_false, "%s$and_not_false$%d", funcContext->func_name, id);
    snprintf(label_end,    sizeof label_end,    "%s$and_end$%d",    funcContext->func_name, id);

    emit_command(node->child[0],funcContext);
    // Stack: [left_value]

    // 2. Zkontrolovat, zda je false nebo nil
    char left_tmp[128];
    snprintf(left_tmp, sizeof left_tmp, "%s_and_left_%d", funcContext->func_name, id);

    printN("POPS LF@%s", left_tmp);        // Uložit levou hodnotu
    printN("PUSHS LF@%s", left_tmp);       // Vrátit zpět pro test
    printN("PUSHS bool@false");
    printN("JUMPIFEQS %s", label_end);     // Pokud false → konec (vrátí false)

    printN("PUSHS LF@%s", left_tmp);       // Test na nil
    printN("PUSHS nil@nil");
    printN("JUMPIFEQS %s", label_end);     // Pokud nil → konec (vrátí nil)

    // 3. Levý neni false/nil tak se vrati pravy
    printN("LABEL %s", label_not_false);
    emit_command(node->child[0], funcContext);
    // Stack: [right_value] → to je výsledek

    printN("LABEL %s", label_end);
    // Stack: [result] (buď left pokud false/nil, nebo right)
}
void emit_or(AstNode *node, FuncContext *funcStruct) {
    int id = node->tmp_id;
    char label_not_false[128], label_end[128];
    snprintf(label_not_false, sizeof label_not_false, "%s$and_not_false$%d", funcStruct->func_name, id);
    snprintf(label_end,    sizeof label_end,    "%s$and_end$%d",    funcStruct->func_name, id);

    emit_command(node->child[0], funcStruct);
    // Stack: [left_value]

    // 2. Zkontrolovat, zda je false nebo nil
    char left_tmp[128];
    snprintf(left_tmp, sizeof left_tmp, "%s_and_left_%d", funcStruct->func_name, id);

    printN("POPS LF@%s", left_tmp);        // Uložit levou hodnotu
    printN("PUSHS LF@%s", left_tmp);       // Vrátit zpět pro test
    printN("PUSHS bool@false");
    printN("JUMPIFEQS %s", label_end);     // Pokud false → konec (vrátí false)

    printN("PUSHS LF@%s", left_tmp);       // Test na nil
    printN("PUSHS nil@nil");
    printN("JUMPIFEQS %s", label_end);     // Pokud nil → konec (vrátí nil)
    printN("PUSHS LF@%s",left_tmp);			// neni nil/false tak ho vratime
    printN("JUMP %s",label_end);

    // 3. Levý je false/nil tak se vrati pravy
    printN("LABEL %s", label_not_false);
    emit_command(node->child[0], funcStruct);
    // Stack: [right_value] → to je výsledek

    printN("LABEL %s", label_end);
    // Stack: [result] (buď left pokud false/nil, nebo right)
}
void emit_negace(AstNode *node,FuncContext *funcStruct) {
    int id = node->tmp_id;
    char value[128];
    snprintf(value, sizeof value, "%s_ln_%d", funcStruct->func_name, id);

    char label_false[128], label_end[128];
    snprintf(label_false, sizeof label_false, "%s$and_not_false$%d", funcStruct->func_name, id);
    snprintf(label_end,    sizeof label_end,    "%s$and_end$%d",    funcStruct->func_name, id);

    emit_command(node->child[0], funcStruct);
    // Stack: [left_value]
    printN("POPS %s",value);

    printN("NOT");
    // Stack: [result] (buď left pokud false/nil, nebo right)
}
void emit_assigned(AstNode *node, FuncContext *funcStruct) {
    emit_command(node->child[1], funcStruct);
    printN("POPS LF@%s",node->child[0]->value.str);
}
void emit_var(AstNode *node, FuncContext *funcStruct){
	emit_command(node->child[1]->child[0], funcStruct);
	printN("POPS LF@%s",node->child[0]->value.str);
}
/*
void emit_while(AstNode *node, DynamiclabelStackStackPtr label_stack, int while_counter) {
    while_counter++;
    emit_command(node->child[0], label_stack, while_counter, NULL);
    char *label = generatorLable("while", label_stack, while_counter, true);
    printf("LABEL %s \node", label);
    emit_command(node->child[1], label_stack, while_counter, NULL);
     * nejak se dostat k podmince ve strome
    char *label_end = generatorLable("end", label_stack, while_counter, false);
    printf("LABEL %s \node", label_end);
    printf("JUMPIFEQ  %s GF@cond bool@false", label_end);
}
*/

void emitIfChain(AstNode *node, FuncContext *funcContext)
{

    funcContext->counter++;
    char *endLabel = NULL;
    /**
     * vytvorenie lablu na spode if chainu na ktory v pripade splnenia podmienky
     * budeme z jednotlivych podmienok skakat
    */
    endLabel = generatorLable("ifChainEnd", funcContext, false);

    for (int i = 0; i < node->arity; i++)
    {
        if (node->child[i]->kind == AST_IF)
        {
            emitIf(node->child[i], funcContext,endLabel);
        }
        else if (node->child[i]->kind == AST_ELSE)
        {
            emitElse(node->child[i], funcContext);
            printN("JUMP %s", endLabel);
            break;
        }
        else
        {
            printError(INTERNAL_ERROR,"Chyba vo vystaveni AST stromu");
            memorySafeExit(INTERNAL_ERROR);
        }
    }
    printN("LABEL %s", endLabel);

}
void emitIf(AstNode *node, FuncContext *funcContext, char *endLabel)
{
    funcContext->counter++;
    char *label = NULL;
    label = generatorLable("ifEnd",funcContext,false);
    //ifchain -> if -> condition
    emit_command(node->child[0], funcContext);
    // po emit command mame vysledok podmienky true/false na zasobniku
    // pridame si tam umelo false
    printN("PUSHS bool@false");
    // porovname prve 2 hodnoty na zasobniku, nas umelo pridany false a aj vysledok cond
    // ak sa budu rovnat jumpne na novovytvoreny label
    printN("JUMPIFEQS %s", label);

    emitChildren(node->child[1], funcContext);

    // po vykonani if blocku skocime na koniec if chainu kedze bola splnena if podmienka
    printN("JUMP %s", endLabel);


    // vytvorenie lablu v ifjcode na ktory budeme skakat pri vyhodnoteni podmienky ako false
    printN("LABEL %s", label);

}

void emitElse(AstNode *node, FuncContext *funcContext)
{
    emitChildren(node->child[0], funcContext);
}


// /**
//  * @brief Funkce pro zpracovani while cyklu
//  * @param node Ukazatel na aktualne
//  */
// void emitWhile(AstNode* node) {

// }
/*
 *@brief switch zajisti zavolani sablon
 *@params ukazatel na uzel
 *@params ukazatel na hash_table
 */
void emit_command(AstNode *node,FuncContext *funcStruct) {
    ast_node_type_to_string(node->kind);

    switch (node->kind) {
        // High-level a seznamy (už byly implementovány s emitChildren)
        case AST_PROLOG:
            emit_prolog();
            break;
        case AST_MAIN_CLASS:
          		printN("JUMP $$main");
            	emitChildren(node, funcStruct);
           	break;
        case AST_PROGRAM:
        case AST_BLOCK:
        case AST_STATEMENT_LIST:
            emitChildren(node, funcStruct);
            break;
        case AST_MAIN_CLASS_BLOCK:
        	for(int i = node->arity-1;i >= 0;i--){
         	emit_command(node->child[i],funcStruct);
         }
         	break;
          case AST_FLOAT_LIT:
              printN("PUSHS float@%a", node->value.float_val);
              break;

        case AST_INT_LIT:
            printN("PUSHS int@%lld", node->value.int_val);
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
        case AST_WHILE:
            // emit_while(node);
            break;
        case AST_FOR:
            // emit_for();
            break;
        case AST_IF_CHAIN:
            emitIfChain(node, funcStruct);
            break;
        case AST_BREAK:
            // Nová implementace: preruseni cyklu
            // emit_break(node, label_stack);
            break;
        case AST_CONTINUE:
            // Nová implementace: pokracovani cyklu
            // emit_continue(node, label_stack);
            break;
        case AST_RETURN:
            // Nová implementace: navrat z funkce
            emitReturn(node,funcStruct);
            break;

        // Deklarace a volání
        case AST_FUNC_DEC:
            // definice funkce
            emitFuncDec(node);
            break;
        case AST_FUNC_CALL:
       		emit_call_func(node,funcStruct);
            break;

        // Proměnné / Přiřazení
        case AST_VAR:
            // Nová implementace: deklarace/inicializace proměnné
            emit_var(node, funcStruct);
            break;
        case AST_ASSIGN:
            emit_assigned(node,funcStruct);
            break;

        // Listy
        case AST_IDENTIFIER:
           // printN("PUSHS LF@%s", node->value.str);
            break;
        case AST_GLOBAL_ID:
            // Nová implementace: nacteni globalni promenne
            // emit_global_id(node);
            break;

        case AST_EMPTY:
            // Nová implementace: Prazdny uzel (nic nedelat)
            break;

        // Operace (Expression)
        case AST_PLUS:
            emitPlus(node, funcStruct);  // Byla implementována, jen pro úplnost.
            break;
        case AST_MINUS:
            // Nová implementace: Odčítání
            emitMinus(node,funcStruct);  // Byla implementována, jen pro úplnost.
            // emitMinus(node, op_emit);
            break;
        case AST_MUL:
        	emitMul(node, funcStruct);
            // Nová implementace: Násobení
            // emitMul(node, op_emit);
            break;
        case AST_DIV:
       		emitDiv(node, funcStruct);
            // Nová implementace: Dělení
            // emitDiv(node, op_emit);
            break;
        case AST_EQ:
            // Nová implementace: Porovnání ==
            // emitEq(node, op_emit);
            break;
        case AST_LT:
            // Nová implementace: Porovnání <
            // emitLt(node, op_emit);
            break;
        case AST_GT:
            // Nová implementace: Porovnání >
            // emitGt(node, op_emit);
            break;
        case AST_NOT:
            // Nová implementace: Logická negace !
            // emitNot(node, op_emit);
            break;
        case AST_AND:
            // Nová implementace: Logické AND &&
            // emitAnd(node, op_emit);
            break;
        case AST_OR:
            // Nová implementace: Logické OR ||
            // emitOr(node, op_emit);
            break;
        case AST_IS:
            // Nová implementace: Porovnání 'is'
            // emitIs(node, op_emit);
            break;
        case AST_GTE:
            // Nová implementace: Porovnání >=
            // emitGte(node, op_emit);
            break;
        case AST_LTE:
            // Nová implementace: Porovnání <=
            // emitLte(node, op_emit);
            break;
        case AST_LPAREN:
            // Nová implementace: Levá závorka (Obvykle zpracováváno jako součást expression)
            // emitChildren(node, label_stack, counter, op_emit);
            break;
        case AST_RPAREN:
            // Nová implementace: Pravá závorka (Obvykle zpracováváno jako součást expression)
            // emitChildren(node, label_stack, counter, op_emit);
            break;
        case AST_NULL:
            // Nová implementace: Reprezentace literálu NULL
            // emit_null(node);
            break;
        case AST_IFJ_CALL:
       		emitIfjFunc(node, funcStruct);
         	break;
        default:
        	printf("%s \n",ast_node_type_to_string(node->kind));
            printError(INTERNAL_ERROR, "Chyba v generatoru neznamy uzel");
            memorySafeExit(INTERNAL_ERROR);
    }
}
void codegen_generate(AstNode *root) {
    int counter = 0;
    FuncContext *funcStruct = NULL;
    emitChildren(root,funcStruct);
    /*
    printN("EXIT %s", NO_ERROR);
    printN("LABEL error %s", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);
    printN("EXIT %s", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);
    printN("LABEL error%s", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    printN("EXIT %s", RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
    */
}
