#include "builtin_functions.h"

#include <string.h>

#include "support_functions.h"

void handleBuiltin(AstNode *ifjFunctionCall) {

    char * functionName = ifjFunctionCall->child[0]->value.str;
    if (strncmp(functionName, "read_str", 9) == 0) {
        printLog("read_str\n");
    }

    if (strncmp(functionName, "read_num", 9) == 0) {
        printLog("read_num\n");
    }
    if (strncmp(functionName, "write", 6) == 0) {
        printLog("write\n");
    }
    if (strncmp(functionName, "floor", 6) == 0) {
        printLog("floor\n");
    }
    if (strncmp(functionName, "str", 4) == 0) {
        printLog("str\n");
    }
    if (strncmp(functionName, "substring", 10) == 0) {
        printLog("substring\n");
    }
    if (strncmp(functionName, "strcmp", 7) == 0) {
        printLog("strcmp\n");
    }

    if (strncmp(functionName, "ord", 4) == 0) {
        printLog("ord\n");
    }

    if (strncmp(functionName, "chr", 4) == 0) {
        printLog("chr\n");
    }
}

void emitReadStr() {
    
}

void emitReadNum() {

}

void emitReadBool() {

}

