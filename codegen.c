#include "codegen.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast_strom.h"
#include "dynamic_stack.h"
#include "parser.h"
#include "support_functions.h"
#define MAX_LABELS 1000
/**
* @brief print na .IFJcode25 ktery je musi byt na zacatku
*/
void emit_prolog(){
	printf(".IFJcode25");
}
/**
* @brief counter na cislo pro label
*/
static int label_counter = 0;
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
void emitChildren(AstNode *root){
	for(int i = 0;i < root->arity;i++){
		emit_command(root->child[i]);
	}
}

void emit_float(AstNode *node){
	printf("float@%f \n",node->value.float_val);
}
void emit_int(AstNode *node){
	printf("int@%lld \n",node->value.int_val);
}
void emit_string(AstNode *node){
	printf("string@%s \n",node->value.str);
}
void emit_bool(AstNode *node){
	printf("bool@%s \n",ast_node_type_to_string(node->kind));
}
void emit_push_value(AstNode *node){
	if(node->kind == AST_INT_LIT){
		printf("PUSHS LF@%lld",node->value.int_val);
	}else if(node->kind == AST_STRING_LIT){
		printf("PUSHS LF@%s",node->value.str);
	}else if(node->kind == AST_FLOAT_LIT){
		printf("PUSHS LF@%f",node->value.float_val);
	}
}
void emitPlus(AstNode *node){
	emit_push_value(node->child[0]);
	emit_push_value(node->child[1]);
	printf("ADDS");
}
void emitMultiply(AstNode *node){
	emit_push_value(node->child[0]);
	emit_push_value(node->child[1]);
	printf("MULS");
}
/*
* funkce na checkovani typu vraci true pokud se muze operace provest false kdyz jsou typy spatne
*/
bool check_type(AstNode *node1,AstNode *node2,AstNode *nodeOperator){
	switch(nodeOperator->kind){
		case AST_PLUS:{

		}
		break;

	}
}
void emit_assigne(AstNode *node){
	emit_command(node->child[1]);
	char *pravaStrana = node->child[0]->value.str;
	printf("POPS LF@%s",pravaStrana);
}
void emit_while(AstNode *node){
	emit_command(node->child[0]);
	char * label = generatorLable("while");
	emitChildren(node);
	/*
	* nejak se dostat k podmince ve strome
	*/
	char *label_end = generatorLable("while_end",);
	printf("LABEL %s",label);
	printf("JUMPIFEQ  %s GF@cond bool@false",label_end);


}

/*
*@brief switch zajisti zavolani sablon
*@params ukazatel na uzel
*@params ukazatel na hash_table
*/
void emit_command(AstNode *node) {
    switch (node->kind) {
        case AST_PROLOG:
        	emit_prolog();
        break;
        case AST_PROGRAM:
            emitChildren(node);
            break;
        case AST_BLOCK:
        	emitChildren(node);
         	break;
        case AST_FLOAT_LIT:
        	emit_float(node);
         	break;
        case AST_INT_LIT:
       		emit_int(node);
        	break;
        case AST_STRING_LIT:
        	emit_string(node);
         	break;
        case AST_WHILE:
              emit_while(node);
              break;
        case AST_FUNC_CALL:
            emit_funcall(node);
            break;
        case AST_FOR:
       		emit_for();
        	break;
        default:
        	printError(INTERNAL_ERROR, "Chyba v generatoru neznamy uzel");
         	memorySafeExit(INTERNAL_ERROR);
    }
}
void codegen_generate(AstNode *root){
	DynamiclabelStackStackPtr label_stack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);

	emitChildren(root);
}
