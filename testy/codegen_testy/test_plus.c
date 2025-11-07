#include "../../codegen.h"
#include <stdbool.h>
#include <stdio.h>
#include "../../ast_strom.h"
#include "../../dynamic_stack.h"
#include "../../parser.h"
#include "../../support_functions.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
int main(){
	DynamiclabelStackStackPtr label_stack = stacklabelStackConstruct(DEFAULT_INIT_ELEMENT_COUNT);
	DynamicEmitOpStackPtr op_stack = stackEmitOpConstruct(DEFAULT_INIT_ELEMENT_COUNT);
	int counter = 1;

	AstNode *nodeLeftCh = calloc(1, sizeof(AstNode));
	AstNode *nodeRightCh = calloc(1, sizeof(AstNode));
	AstNode *nodeParent = calloc(1, sizeof(AstNode));
	AstNode *nodeVar = calloc(1, sizeof(AstNode));
	AstNode *nodeAssigned = calloc(1, sizeof(AstNode));
	if (!nodeLeftCh || !nodeRightCh || !nodeParent || !nodeAssigned) {
	    fprintf(stderr, "Allocation failed\n");
	    exit(1);
	}

	nodeLeftCh->kind = AST_INT_LIT;
	nodeLeftCh->value.int_val = 6;
	nodeLeftCh->is_leaf = true;

	nodeRightCh->kind = AST_INT_LIT;
	nodeRightCh->value.int_val = 7;
	nodeRightCh->is_leaf = true;

	nodeParent->kind = AST_PLUS;
	nodeParent->arity = 2;
	nodeParent->child = calloc(nodeParent->arity, sizeof(AstNode *));
	if (!nodeParent->child) {
	    fprintf(stderr, "Child array allocation failed\n");
	    exit(1);
	}
	nodeParent->child[0] = nodeLeftCh;
	nodeParent->child[1] = nodeRightCh;
	nodeParent->is_leaf = false;
	nodeAssigned->child[1] = nodeParent;
	nodeAssigned->child[0] = NULL;
	emitPlus(nodeParent, label_stack, counter, op_stack);
	emit_assigned(AstNode *node, DynamiclabelStackStackPtr label_stack, int counter, DynamicEmitOpStackPtr op_emit);
	free(nodeLeftCh);
	free(nodeRightCh);
	free(nodeParent);
}
