#include "builtin_functions.h"

#include <string.h>

#include "support_functions.h"

void emitWrite(AstNode* functionCallNode, FuncContext* context);

void handleBuiltin(AstNode *ifjFunctionCall, FuncContext* context) {

    char * functionName = ifjFunctionCall->child[0]->value.str;
    if (strncmp(functionName, "read_str", 9) == 0) {
        printLog("read_str\n");
    }

    if (strncmp(functionName, "read_num", 9) == 0) {
        printLog("read_num\n");
    }
    if (strncmp(functionName, "write", 6) == 0) {
     //   printLog("write\n");
        emitWrite(ifjFunctionCall, context);
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


void emitWrite(AstNode* functionCallNode, FuncContext* context) {
  //  printN("\n#ZACATEK:");
    static unsigned writeCounter = 0;
    //AstNode* param = functionCallNode->child[1]->child[0];

    emit_command(functionCallNode->child[1]->child[0], context);
    printN("DEFVAR LF@temp$var$write$%u",writeCounter);
    printN("POPS LF@temp$var$write$%u", writeCounter);
    printN("WRITE LF@temp$var$write$%u", writeCounter);

    // TODO upravit aby to vždy šlo přes emit_command()

   /*  if(param->kind == AST_EMPTY) return;

            // TODO bool
            if (param->kind == AST_IDENTIFIER) {
                printN("WRITE LF@%s", param->value.str);
            } else if (param->kind == AST_STRING_LIT) {
                printN("WRITE string@%s", param->value.str);
            } else if (param->kind == AST_INT_LIT) {
                printN("WRITE int@%lld", param->value.int_val);
            } else if (param->kind == AST_BOOL_LIT) {
                printN("WRITE bool@%s", param->value.str);
            } else if (param->kind == AST_FLOAT_LIT) {
                printN("WRITE float@%a", param->value.float_val);
            } else if (param->kind == AST_SMALL_NULL) {
                printN("WRITE nil@nil");
            } else {
                emit_command(param, context);
                printN("DEFVAR LF@temp$var$write$%u",writeCounter);
                printN("POPS LF@temp$var$write$%u", writeCounter);
                printN("WRITE LF@temp$var$write$%u", writeCounter);
                //printN("PUSHS LF@temp$var$write$%u", writeCounter);
            } */
    
   writeCounter++;
  //  printN("#:KONEC\n");
}
