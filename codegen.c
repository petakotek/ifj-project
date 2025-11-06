#include "codegen.h"
#include <stdbool.h>
#include <stdio.h>
#include "ast_strom.h"
#include "dynamic_stack.h"
#include "parser.h"
#include "support_functions.h"
#define MAX_LABELS 1000
#define macro
/*
*Print funkce na zajistneti \n na konci
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
void emit_prolog(){
	printN(".IFJcode25");
}
/**
*@buffer na jednotlive labely aby se nakonci uvolnovali
*/
char *buffer_pointer_label[MAX_LABELS];
char *resize_label_name(char *prefix,size_t *size){
	*size *= 2;
    prefix = realloc(prefix, *size);
    if(prefix == NULL){
    printError(INTERNAL_ERROR, "error in resize_label_reallock");
    memorySafeExit(INTERNAL_ERROR);
    }
    return prefix;
}
/**
* @brief generator jednotlivych labelu. Label je tvoren z nazvu lablu ktere existovali ve vetvi prednim
* priklad: fun$1$while$2 je label druheho while loopu ve funkci fun fun$1$while$2$end je koncovy label toho while loopu
* @param label_stack stack na ukladani prubezne lablu
* @param func_name jmeno funcke ktere chceme pripojit
* @return ukazatel na vysledny label
*/
char *generatorLable(char *func_name, DynamiclabelStackStackPtr label_stack, int counter, bool pridaniDoStacku) {
        size_t size_of_buffer = 128;
        char *result_label = (char *)malloc(size_of_buffer);
        if (result_label == NULL) {
            printError(INTERNAL_ERROR, "Error allocating memory for label");
            memorySafeExit(INTERNAL_ERROR);
        }

        int current_len = 0;

        //Získání existujícího prefixu ze zásobníku (pokud existuje)
        if (!stacklabelStackIsEmpty(label_stack)) {
            char *prefix = stacklabelStackRead(label_stack);
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

        int written = snprintf(result_label + current_len, remaining_space, "$%s$%d", func_name, counter);

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
                 written = snprintf(result_label + current_len, remaining_space, "$%s$%d", func_name, counter);
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
        stacklabelStackPush(label_stack, result_label);
        }
        //a jeho vraceni
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
void emitChildren(AstNode *root,DynamiclabelStackStackPtr label_stack,int counter,DynamicEmitOpStackPtr op_stack){
	for(int i = 0;i < root->arity;i++){
		emit_command(root->child[i],label_stack,counter,op_stack);
	}
}
void emit_push_value(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter,DynamicEmitOpStackPtr op_stack) {
    if (node->kind == AST_INT_LIT) {
        printN("PUSHS int@%lld", node->value.int_val);
    } else if (node->kind == AST_STRING_LIT) {
        printN("PUSHS string@%s", node->value.str);
    } else if (node->kind == AST_FLOAT_LIT) {
        printN("PUSHS float@%f", node->value.float_val);
    } else if (node->kind == AST_VAR) {
        printN("PUSHS LF@%s", node->value.str);
    }
    stackEmitOpPush(op_stack, node);
}
char *checkTopStack(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter, DynamicEmitOpStackPtr op_stack) {
	if(!stackEmitOpIsEmpty(op_stack)){
    AstNode *tmp_check = stackEmitOpRead(op_stack);
    if (tmp_check == node->child[0]) {
        stackEmitOpPush(op_stack, tmp_check);
        return "LEFT_TOP";
    }
    if (tmp_check == node->child[1]) {
    	stackEmitOpPush(op_stack, tmp_check);
        return "RIGHT_TOP";
    }
	}
    return "NONE_ON_STACK";
}

char *emitBinaryOp(AstNode *node,  DynamiclabelStackStackPtr labels, int counter,DynamicEmitOpStackPtr op_stack,int flag_to_stack) {
	char *tmp = checkTopStack(node,labels,counter,op_stack);
	if(strcmp(tmp,"LEFT_TOP") == 0){
		emit_push_value(node->child[0], labels,counter,op_stack);
	}else if(strcmp(tmp,"RIGH_TOP") == 0){
		emit_push_value(node->child[1],labels,counter,op_stack);
	}else{
		if(flag_to_stack == 1){
			//TODO nejaky exit ve chvili kdyz se s funkci musi pracovat ve vyrazu
			printN("error");
		}
		emit_push_value(node->child[0],labels,counter,op_stack);
		emit_push_value(node->child[1],labels,counter,op_stack);
	}
    return "STACK_TOP";
}
/*
* Dostaneme zasobnik zkontrolujeme zda je neco na vrchu pokud jo udelame types coz nam da na vrchol typ na vrchol a potom si pushneme dite a dame na nej types
* zasobnik
* type
* promena (dite)
* type
* promena ze zasobnika
* JUMPIFEQS/JUMPIFNEQS ⟨label⟩
*/
void emitPlus(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter, DynamicEmitOpStackPtr op_stack) {
	int flag_to_stack = 0;
	if(node->child[0]->kind == AST_FUNC_CALL){
		emit_command(node->child[0], label_stack, counter, op_stack);
		flag_to_stack = 1;
	}
	if(node->child[1]->kind == AST_FUNC_CALL){
		emit_command(node->child[1], label_stack, counter, op_stack);
		flag_to_stack = 1;
	}
    emitBinaryOp(node, label_stack, counter, op_stack,flag_to_stack);
    // Po emitBinaryOp máme na zásobníku: [hodnota_left, hodnota_right]

    char *label_ok = generatorLable("ok", label_stack, counter, false);
    char *label_concat = generatorLable("concat", label_stack, counter, false);
    char *label_add = generatorLable("add", label_stack, counter, false);
    char *label_end = generatorLable("end", label_stack, counter, false);
    char *label_err = generatorLable("err", label_stack, counter, false);

    // Typová kontrola pomocí zásobníku
    // Stav zásobníku: [left_value, right_value]

    // Zjistíme typ pravého operandu (right)
    printN("TYPES");  // zásobník: [left_value,	right_type]

    // Dočasně uložíme typ pravého operandu
    printN("POPS TF@type_right");  // zásobník: [left_value]

    // Zjistíme typ levého operandu (left)
    printN("TYPES");  // zásobník: [type_left]

    // Vrátíme typ pravého operandu zpět
    printN("PUSHS TF@type_right");  // zásobník: [type_left, type_right]

    // Porovnání, zda jsou typy stejné
    printN("JUMPIFEQS %s", label_ok);  // pokud type_left == type_right
    printN("JUMP %s", label_err);  // jinak chyba

    // --- Kontrola konkrétního typu ---
    printN("LABEL %s", label_ok);
    // Máme na zásobníku pořád type_right (nebo type_left, jsou stejné)
    printN("PUSHS string@string");
    printN("JUMPIFEQS %s", label_concat);

    // Obnovíme typ pro další porovnání
    printN("PUSHS TF@type_right");
    printN("PUSHS string@int");
    printN("JUMPIFEQS %s", label_add);

    printN("PUSHS TF@type_right");
    printN("PUSHS string@float");
    printN("JUMPIFEQS %s", label_add);

    // Pokud to není ani string, ani int, ani float
    printN("JUMP %s", label_err);

    // --- ADD větev (pro int a float) ---
    printN("LABEL %s", label_add);
    // Musíme znovu načíst původní hodnoty
    printN("PUSHS TF@lhs");
    printN("PUSHS TF@rhs");
    printN("ADDS");
    printN("JUMP %s", label_end);

    // --- CONCAT větev (pro string) ---
    printN("LABEL %s", label_concat);
    printN("PUSHS TF@lhs");
    printN("PUSHS TF@rhs");
    printN("CONCAT");
    printN("JUMP %s", label_end);

    // --- CHYBA ---
    printN("LABEL %s", label_err);
    printN("EXIT int@4");

    // --- KONEC ---
    printN("LABEL %s", label_end);

}
void emitMinus(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter, DynamicEmitOpStackPtr op_stack) {
	int flag_to_stack = 0;
	if(node->child[0]->kind == AST_FUNC_CALL){
		emit_command(node->child[0], label_stack, counter, op_stack);
		flag_to_stack = 1;
	}
	if(node->child[1]->kind == AST_FUNC_CALL){
		emit_command(node->child[1], label_stack, counter, op_stack);
		flag_to_stack = 1;
	}
    emitBinaryOp(node, label_stack, counter, op_stack,flag_to_stack);
    // Po emitBinaryOp máme na zásobníku: [hodnota_left, hodnota_right]
    char *label_ok = generatorLable("ok", label_stack, counter, false);
    char *label_err = generatorLable("error", label_stack, counter, false);
    char *label_end = generatorLable("end",label_stack, counter, false);
    // Zjistíme typ pravého operandu (right)
    printN("TYPES");  // zásobník: [left_value,	right_type]

    // Dočasně uložíme typ pravého operandu
    printN("POPS TF@type_right");  // zásobník: [left_value]

    // Zjistíme typ levého operandu (left)
    printN("TYPES");  // zásobník: [type_left]

    // Vrátíme typ pravého operandu zpět
    printN("PUSHS TF@type_right");  // zásobník: [type_left, type_right]

    // Porovnání, zda jsou typy stejné
    printN("JUMPIFEQS %s", label_ok);  // pokud type_left == type_right
    printN("JUMP %s", label_err);  // jinak chyba
    printN("LABEL %s", label_ok);
    printN("PUSHS TF@lhs");
    printN("PUSHS TF@rhs");
    printN("SUBS");
    printN("JUMP %s", label_end);
    // --- CHYBA ---
    printN("LABEL %s", label_err);
    printN("EXIT int@4");

    // --- KONEC ---
    printN("LABEL %s", label_end);

}
void emit_equal(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter,char *end) {
    emit_command(node->child[0], label_stack, counter,NULL);
    printN("JUMPIFNEQ %s %s %s", end);
}
void emit_assigned(AstNode *node,DynamiclabelStackStackPtr label_stack,int counter,DynamicEmitOpStackPtr op_emit){
	emit_command(node->child[1],label_stack,counter,op_emit);
	printN("POPS LF@%s",node->child[0]->value.str);
}
void emit_while(AstNode *node,DynamiclabelStackStackPtr label_stack,int while_counter){
	while_counter ++;
	emit_command(node->child[0],label_stack,while_counter,NULL);
	char * label = generatorLable("while",label_stack,while_counter,true);
	printf("LABEL %s \n",label);
	emit_command(node->child[1], label_stack, while_counter,NULL);
	/*
	* nejak se dostat k podmince ve strome
	*/
	char *label_end = generatorLable("end",label_stack,while_counter,false);
	printf("LABEL %s \n",label_end);
	printf("JUMPIFEQ  %s GF@cond bool@false",label_end);


}

/*
*@brief switch zajisti zavolani sablon
*@params ukazatel na uzel
*@params ukazatel na hash_table
*/
void emit_command(AstNode *node,DynamiclabelStackStackPtr label_stack,int counter,DynamicEmitOpStackPtr op_emit) {
	ast_node_type_to_string(node->kind);
    switch (node->kind) {
        case AST_PROLOG:
        	emit_prolog();
        break;
        case AST_PROGRAM:
        case AST_BLOCK:
        case AST_MAIN_CLASS:
        case AST_MAIN_CLASS_BLOCK:
        case AST_STATEMENT_LIST:
        	emitChildren(node,label_stack,counter,op_emit);
         	break;
        case AST_PLUS:
        	emitPlus(node, label_stack,counter,op_emit);
        case AST_FLOAT_LIT:
        	//emit_float(node);
         	break;
        case AST_INT_LIT:
       	//	emit_int(node);
        	break;
        case AST_STRING_LIT:
        	//emit_string(node);
         	break;
        case AST_WHILE:
            //  emit_while(node,label_stack,counter);
              break;
        case AST_FUNC_CALL:
          //  emit_funcall(node);
            break;
        case AST_FOR:
       //		emit_for();
        	break;
        default:
        	printError(INTERNAL_ERROR, "Chyba v generatoru neznamy uzel");
         	memorySafeExit(INTERNAL_ERROR);
    }
}
void codegen_generate(AstNode *root){
	DynamiclabelStackStackPtr label_stack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);
	DynamicEmitOpStackPtr op_stack = stackEmitOpConstruct(DEFAULT_INIT_ELEMENT_COUNT);
	int counter;
	emitChildren(root,label_stack,counter,op_stack);
	printN("EXIT %s",NO_ERROR);
	printN("LABEL error%s",RUNTIME_SEMANTICS_ERROR_BAD_TYPE);
	printN("EXIT %s",RUNTIME_SEMANTICS_ERROR_BAD_TYPE);
	printN("LABEL error%s",RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
	printN("EXIT %s",RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE);
}
