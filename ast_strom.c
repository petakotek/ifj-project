#include "ast_strom.h"
#include "lexer.h"
AstNode* createAstNode(AstNodeType type) {
    AstNode* node = malloc(sizeof(AstNode));
    memset(node, 0, sizeof(AstNode));
    node->type = type;
    return node;
}
