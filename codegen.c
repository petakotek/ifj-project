#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include "ast_strom.h"
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
/**
* @brief generator jednotlivych labelu
* @param ukazatel na prefix labelu
* @return ukazatel na vysledny label
*/
char *generatorLable(char * prefix){
	char *buffer = malloc(64);
	snprintf(buffer, sizeof(buffer), "%s_%d", prefix, label_counter);
	buffer_pointer_label[label_counter++] = buffer;
	return buffer;
}
/*
* @brief uvolnovaci funkce na jednotlive lably
*/
void free_labels() {
    for (int i = 0; i < label_counter; i++) {
        free(buffer_pointer_label[i]);
    }
    label_counter = 0;
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
void emit_deti(AstNode *root){
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
/*
* funkce na checkovani typu
*/
void check_type(AstNode *node1,AstNode *node2,AstNode *nodeOperator){
	switch(nodeOperator->kind){
		case AST_PLUS:{

		}
		break;

	}
}
void emit_assigne(AstNode *node){
	char * pravaStrana = node->value.str;
	for(int i = 0;i < node->arity;i++){
		AstNode *levaStrana = node->child[i];
		if(levaStrana->kind == AST_PLUS){
			emit_push_value(node->child[0]);
			emit_push_value(node->child[1]);
			printf("ADDS");
		}
		if(levaStrana->kind == AST_MINUS){
			emit_push_value(node->child[0]);
			emit_push_value(node->child[1]);
			printf("MINUS");
		}
	}
}
void emit_while(AstNode *node){
	emit_command(node->child[0]);
	char * label = generatorLable("while");
	emit_deti(node);
	/*
	* nejak se dostat k podmince ve strome
	*/
	char *label_end = generatorLable("while_end");
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
            emit_deti(node);
            break;
        case AST_BLOCK:
        	emit_deti(node);
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
	emit_deti(root);
}
