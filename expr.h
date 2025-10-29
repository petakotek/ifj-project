
#ifndef EXPR_H
#define EXPR_H

#include "ast_strom.h"
#include "parser.h"
#include "dynamic_stack.h"



AstNode* parseExpression(Token *token);

#endif