#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include "ast_strom.h"
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
void emit_globals(HashTable *hash_table){
	/*
	* Nejakym zpusobem se dostane na globalni promene
	*/
}
/*
*@brief zajisti rozbaleni uzlu
*@params ukazatel na uzel
*@params ukazatel na hash_table
*/
void emit_deti(AstNode *root,HashTable *hash_table){
	for(int i = 0;i < root->arity;i++){
		emit_command(root->child[i],hash_table);
	}
}

void emit_while(AstNode *node, HashTable *hash_table){
	emit_deti(node, hash_table);
	/*
	* nejak se dostat k podmince ve strome
	*/
	printf("JUMPIFEQ ");
	printf("LABEL %s",generatorLable("while"));
}

/*
*@brief switch zajisti zavolani sablon
*@params ukazatel na uzel
*@params ukazatel na hash_table
*/
void emit_command(AstNode *node, HashTable *hash_table) {
    switch (node->kind) {
        case AST_PROLOG:
        	emit_prolog();
        break;
        case AST_PROGRAM:
            emit_deti(node, hash_table);
            break;
        case AST_WHILE:
            emit_while(node, hash_table);
            break;
        case AST_FUNC_CALL:
            emit_funcall(node, hash_table);
            break;
        case AST_BLOCK:
        	emit_deti(node,hash_table);
         	break;
        default:
        	printError(INTERNAL_ERROR, "Chyba v generatoru neznamy uzel");
         	memorySafeExit(INTERNAL_ERROR);
    }
}
void codegen_generate(AstNode *root, HashTable *hash_table){
	emit_globals(hash_table);
	emit_deti(root, hash_table);
}
