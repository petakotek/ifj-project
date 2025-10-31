#include "codegen.h"
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
char *generatorLable(char *func_name, DynamiclabelStackStackPtr label_stack, int counter) {
    size_t size_of_buffer = 128;
    char *result_label = (char *)malloc(size_of_buffer);
    if (result_label == NULL) {
    printError(INTERNAL_ERROR, "error behem tvoreni lablu ");
    memorySafeExit(INTERNAL_ERROR);
    }
    //vlozeni func_name do bufferu kdyz nevejde hazime error
    int current_len = snprintf(result_label, size_of_buffer, "%s", func_name);
    if (current_len < 0 || (size_t)current_len >= size_of_buffer) {
        free(result_label);
        printError(INTERNAL_ERROR, "error behem tvoreni lablu ");
        memorySafeExit(INTERNAL_ERROR);
    }
    while (!stacklabelStackIsEmpty(label_stack)) {
        char *prefix = stacklabelStackRead(label_stack);
        size_t prefix_len = strlen(prefix);
        if (current_len + prefix_len + 5 >= size_of_buffer) {
            result_label = resize_label_name(result_label, &size_of_buffer);
            if (result_label == NULL) {
            printError(INTERNAL_ERROR, "error behem tvoreni lablu ");
            memorySafeExit(INTERNAL_ERROR);
            }
        }
        int written = snprintf(result_label + current_len, size_of_buffer - current_len, "%s", prefix);
        if (written > 0) {
            current_len += written;
        } else {
            break;
        }
    }
    if (current_len + 1 + 10 >= size_of_buffer) {
        result_label = resize_label_name(result_label, &size_of_buffer);
        if (result_label == NULL) {
        printError(INTERNAL_ERROR, "error behem tvoreni lablu ");
        memorySafeExit(INTERNAL_ERROR);
        }
    }
    result_label[current_len] = '$';
    current_len++;

    size_t remaining_space = size_of_buffer - current_len;

    int written_counter = snprintf(result_label + current_len,remaining_space,"%d",counter);

    if (written_counter > 0) {
        current_len += written_counter;
    }
    stacklabelStackPush(label_stack, result_label);
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
    AstNode *tmp_check = stackEmitOpRead(op_stack);

    if (tmp_check == node->child[0]) {
        stackEmitOpPush(op_stack, tmp_check);
        return "LEFT_TOP";
    }
    if (stacklabelStackRead(label_stack) == node->child[1]->value.str) {
    stackEmitOpPush(op_stack, tmp_check);
        return "RIGHT_TOP";
    }

    stackEmitOpPush(op_stack, tmp_check);
    return "NONE_ON_STACK";
}

char *emitBinaryOp(AstNode *node,  DynamiclabelStackStackPtr labels, int counter,DynamicEmitOpStackPtr op_stack) {
	char *tmp = checkTopStack(node,labels,counter,op_stack);
	if(strcmp(tmp,"LEFT_TOP") == 0){
		emit_push_value(node->child[0], labels,counter,op_stack);
	}else if(strcmp(tmp,"RIGH_TOP") == 0){
		emit_push_value(node->child[1],labels,counter,op_stack);
	}else{
		emit_push_value(node->child[0],labels,counter,op_stack);
		emit_push_value(node->child[1],labels,counter,op_stack);
	}
    return "STACK_TOP";
}
void emitPlus(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter, DynamicEmitOpStackPtr op_stack) {
    emitBinaryOp(node, label_stack, counter, op_stack);

    // Labely generované pomocí tvojí funkce
    char *label_ok = generatorLable("ok", label_stack, counter);
    char *label_concat = generatorLable("concat", label_stack, counter);
    char *label_add     = generatorLable("add", label_stack, counter);
    char *label_end     = generatorLable("end", label_stack, counter);
    char *label_err     = generatorLable("err", label_stack, counter);

    // Typová kontrola
    printN("POPS TF@rhs");
    printN("POPS TF@lhs");
    printN("TYPE TF@t1 TF@lhs");
    printN("TYPE TF@t2 TF@rhs");

    printN("JUMPIFEQ %s TF@t1 TF@t2", label_ok);
    printN("JUMP %s", label_err);

    printN("LABEL %s", label_ok);
    printN("JUMPIFEQ %s TF@t1 string@string", label_concat);
    printN("JUMPIFEQ %s TF@t1 string@int", label_add);
    printN("JUMPIFEQ %s TF@t1 string@float", label_add);
    printN("JUMP %s", label_err);

    // --- ADD větev ---
    printN("LABEL %s", label_add);
    printN("PUSHS TF@lhs");
    printN("PUSHS TF@rhs");
    printN("ADDS");
    printN("JUMP %s", label_end);

    // --- CONCAT větev ---
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
void emit_equal(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter,char *end) {
    emit_command(node->child[0], label_stack, counter,NULL);
    printN("JUMPIFNEQ %s %s %s", end);
}
void emit_while(AstNode *node,DynamiclabelStackStackPtr label_stack,int while_counter){
	while_counter ++;
	emit_command(node->child[0],label_stack,while_counter,NULL);
	char * label = generatorLable("while",label_stack,while_counter);
	printf("LABEL %s \n",label);
	emit_command(node->child[1], label_stack, while_counter,NULL);
	/*
	* nejak se dostat k podmince ve strome
	*/
	char *label_end = generatorLable("end",label_stack,while_counter);
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
