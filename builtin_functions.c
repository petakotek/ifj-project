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
        printLog("write\n");
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
    printf("here\n");
    //context->counter;
    /*
    static Ifj.write(term) → Null
Vypíše hodnotu termu term (libovolného typu) na standardní výstup ihned a bez žádných
oddělovačů dle typu v patřičném formátu: Termy typu String vypíše tak, jak jsou. Termy
typu Num s celočíselnou hodnotou (tj. s nulovou desetinnou částí) vypíše jako celé číslo
11
obvyklým způsobem (odpovídá specifikátoru '%d'10), desetinné hodnoty termu typu Num
vypíše ve formátu, který odpovídá specifikátoru '%a'11. Hodnota null je tištěna jako
literál "null". Tyto požadavky odpovídají chování instrukce WRITE v IFJcode25 pro
symboly typu string, int (!), float a nil.
    */
    AstNode* param = functionCallNode->child[1]->child[0];

    if(param->kind == AST_EMPTY) return;

    
            if (param->kind == AST_IDENTIFIER) {
                printN("WRITE LF@%s", param->value.str);
            } else if (param->kind == AST_STRING_LIT) {
                printN("WRITE string@%s", param->value.str);
            } else if (param->kind == AST_INT_LIT) {
                printN("WRITE int@%lld", param->value.int_val);
            } else if (param->kind == AST_FLOAT_LIT) {
                printN("WRITE float@%a", param->value.float_val);
            } else if (param->kind == AST_SMALL_NULL) {
            } else {
                emit_command(param, context);
            }
   // char* label = generatorLable("write", context, false);

    printN("TADY: %s");
}
