#include "codegen.h"
#include <stdio.h>
#include "ast_strom.h"
#include "support_functions.h"
void emit_prolog(){
	printf(".IFJcode25");
}
void emit_globals(HashTable *hash_table){
	/*
	* Nejakym zpusobem najit globalni promene
	* Netusim jak se ukladaji
	*/
}
void emit_seznam(AstNode *root,HashTable *hash_table){
	for(int i = 0;i < root->arity;i++){
		emit_command(root->child[i],hash_table);
	}
}
void emit_while(AstNode *node, HashTable *hash_table){
	printf("JUMPIFEQ ");
	printf("LABEL while_")
}
void emit_command(AstNode *node, HashTable *hash_table) {
    switch (node->kind) {
        case AST_PROLOG:
        break;
        case AST_PROGRAM:
            emit_seznam(node, hash_table);
            break;
        case AST_WHILE:
            emit_while(node, hash_table);
            break;
        case AST_FUNC_CALL:
            emit_funcall(node, hash_table);
            break;
        case AST_BLOCK:
        	emit_seznam(node,hash_table);
         	break;
        default:
        	printError(INTERNAL_ERROR, "Chyba v generatoru neznamy uzel");
         	memorySafeExit(INTERNAL_ERROR);
    }
}
void codegen_generate(AstNode *root, HashTable *hash_table){
	emit_globals(hash_table);
	emit_seznam(root, hash_table);
}
