// TODO hlavička

#include "builtin_functions.h"

#include <string.h>

#include "support_functions.h"

/**
 * @brief Funkce pro přečtení vstupu jako řetězce
 *
 */
void emitReadStr();

/**
 * @brief Funkce pro přečtení vstupu jako čísla
 *
 */
void emitReadNum();

/**
 * @brief Funkce pro přečtení vstupu jako bool hodnoty
 *
 */
void emitReadBool();

/**
 * @brief Funkce por výpis libovolné hodnoty na výstup
 *
 * @param functionCallNode Uzel s voláním funkce
 * @param context Kontext
 */
void emitWrite(AstNode* functionCallNode, FuncContext* context);

/**
 * @brief Funkce pro převod desetinného čísla na celé, useknutím desetinné části
 *
 * @param functionCallNode Uzel s voláním funkce
 * @param context Kontext
 */
void emitFloor(AstNode* functionCallNode, FuncContext* context);

/**
 * @brief VFunkce pro převod libovolné vstupní hodnoty na odpovídající řetězec
 *
 * @param functionCallNode Uzel s voláním funkce
 * @param context Kontext
 */
void emitStr(AstNode* functionCallNode, FuncContext* context);

/**
 * @brief Funkce pro získání délky řetězce
 *
 * @param functionCallNode Uzel s voláním funkce
 * @param context Kontext
 */
void emitLength(AstNode* functionCallNode, FuncContext* context);

/**
 * @brief TODO
 *
 * @param functionCallNode Uzel s voláním funkce
 * @param context Kontext
 */
void emitSubstring(AstNode* functionCallNode, FuncContext* context);

/**
 * @brief Funkce pro porovnání dvou řetězců
 *
 * @param functionCallNode Uzel s voláním funkce
 * @param context Kontext
 */
void emitStrcmp(AstNode* functionCallNode, FuncContext* context);

/**
 * @brief Funkce pro převod znaku zadaného indexem v řetězci na číslo
 *
 * @param functionCallNode Uzel s voláním funkce
 * @param context Kontext
 */
void emitOrd(AstNode* functionCallNode, FuncContext* context);

/**
 * @brief Funkce pro převod celého čísla na znak
 *
 * @param functionCallNode Uzel s voláním funkce
 * @param context Kontext
 */
void emitChr(AstNode* functionCallNode, FuncContext* context);

void handleBuiltin(AstNode* ifjFunctionCall, FuncContext* context) {
    char* functionName = ifjFunctionCall->child[0]->value.str;
    if (strncmp(functionName, "read_str", 9) == 0)
        emitReadStr();
    else if (strncmp(functionName, "read_num", 9) == 0)
        emitReadNum();
    else if (strncmp(functionName, "read_bool", 10) == 0)
        emitReadBool();
    else if (strncmp(functionName, "write", 6) == 0)
        emitWrite(ifjFunctionCall, context);
    else if (strncmp(functionName, "floor", 6) == 0)
        emitFloor(ifjFunctionCall, context);
    else if (strncmp(functionName, "str", 4) == 0)
        emitStr(ifjFunctionCall, context);
    else if (strncmp(functionName, "length", 7) == 0)
        emitLength(ifjFunctionCall, context);
    else if (strncmp(functionName, "substring", 10) == 0)
        emitSubstring(ifjFunctionCall, context);
    else if (strncmp(functionName, "strcmp", 7) == 0)
        emitStrcmp(ifjFunctionCall, context);
    else if (strncmp(functionName, "ord", 4) == 0)
        emitOrd(ifjFunctionCall, context);
    else if (strncmp(functionName, "chr", 4) == 0)
        emitChr(ifjFunctionCall, context);
}

void emitReadStr() {
    static unsigned readStrCounter = 0;

    printN("DEFVAR LF@temp$var$readStr$%u", readStrCounter);
    printN("READ LF@temp$var$readStr$%u string", readStrCounter);
    printN("PUSHS LF@temp$var$readStr$%u", readStrCounter);

    readStrCounter++;
}

void emitReadNum() {
    static unsigned readNumCounter = 0;

    printN("DEFVAR LF@temp$var$readNum$%u", readNumCounter);
    printN("READ LF@temp$var$readNum$%u float", readNumCounter);
    printN("PUSHS LF@temp$var$readNum$%u", readNumCounter);

    readNumCounter++;
}

void emitReadBool() {
    static unsigned readBoolCounter = 0;

    printN("DEFVAR LF@temp$var$readBool$%u", readBoolCounter);
    printN("READ LF@temp$var$readBool$%u bool", readBoolCounter);
    printN("PUSHS LF@temp$var$readBool$%u", readBoolCounter);

    readBoolCounter++;
}

void emitWrite(AstNode* functionCallNode, FuncContext* context) {
    static unsigned writeCounter = 0;

    emit_command(functionCallNode->child[1]->child[0], context);
    printN("DEFVAR LF@temp$var$write$%u", writeCounter);
    printN("DEFVAR LF@temp$var$writeType$%u", writeCounter);
    printN("POPS LF@temp$var$write$%u", writeCounter);

    printN("TYPE LF@temp$var$writeType$%u LF@temp$var$write$%u", writeCounter, writeCounter);
    printN("JUMPIFNEQ temp$label$write$write$%u LF@temp$var$writeType$%u string@float", writeCounter, writeCounter);

    // Výstup je desetinné číslo, ale možná celočíselné
    printN("ISINT LF@temp$var$writeType$%u  LF@temp$var$write$%u", writeCounter, writeCounter);
    printN("JUMPIFNEQ temp$label$write$write$%u LF@temp$var$writeType$%u  bool@true", writeCounter, writeCounter);
    printN("FLOAT2INT LF@temp$var$write$%u LF@temp$var$write$%u", writeCounter, writeCounter);

    printN("LABEL temp$label$write$write$%u", writeCounter);
    printN("WRITE LF@temp$var$write$%u", writeCounter);
    printN("PUSHS nil@nil");

    writeCounter++;
}

void emitFloor(AstNode* functionCallNode, FuncContext* context) {
    static unsigned floorCounter = 0;

    emit_command(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("DEFVAR LF@temp$var$floorVal$%u", floorCounter);
    printN("DEFVAR LF@temp$var$floorType$%u", floorCounter);
    printN("POPS LF@temp$var$floorVal$%u", floorCounter);

    // Podmínka
    printN("TYPE LF@temp$var$floorType$%u LF@temp$var$floorVal$%u", floorCounter, floorCounter);
    printN("JUMPIFEQ temp$label$floor$operation$%u LF@temp$var$floorType$%u string@float", floorCounter, floorCounter);
    printN("JUMPIFEQ temp$label$floor$noOp$%u LF@temp$var$floorType$%u string@int", floorCounter, floorCounter);
    // Chyba
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Desetinné číslo
    printN("LABEL temp$label$floor$operation$%u", floorCounter);
    printN("PUSHS LF@temp$var$floorVal$%u", floorCounter);
    printN("FLOAT2INTS");
    printN("INT2FLOATS");
    printN("JUMP temp$label$floor$end$%u", floorCounter);

    // Celé číslo
    printN("LABEL temp$label$floor$noOp$%u", floorCounter);
    printN("PUSHS LF@temp$var$floorVal$%u", floorCounter);
    printN("INT2FLOATS");
    printN("JUMP temp$label$floor$end$%u", floorCounter);

    printN("LABEL temp$label$floor$end$%u", floorCounter);

    floorCounter++;
}

void emitStr(AstNode* functionCallNode, FuncContext* context) {
    static unsigned strCounter = 0;

    emit_command(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("DEFVAR LF@temp$var$strVal$%u", strCounter);
    printN("DEFVAR LF@temp$var$strType$%u", strCounter);
    printN("POPS LF@temp$var$strVal$%u", strCounter);

    // Podmínka
    printN("TYPE LF@temp$var$strType$%u LF@temp$var$strVal$%u", strCounter, strCounter);
    printN("JUMPIFEQ temp$label$str$float$%u LF@temp$var$strType$%u string@float", strCounter, strCounter);
    printN("JUMPIFEQ temp$label$str$int$%u LF@temp$var$strType$%u string@int", strCounter, strCounter);
    printN("JUMPIFEQ temp$label$str$string$%u LF@temp$var$strType$%u string@string", strCounter, strCounter);
    printN("JUMPIFEQ temp$label$str$bool$%u LF@temp$var$strType$%u string@bool", strCounter, strCounter);
    printN("JUMPIFEQ temp$label$str$null$%u LF@temp$var$strType$%u string@nil", strCounter, strCounter);

    // Chyba
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Desetinné číslo
    printN("LABEL temp$label$str$float$%u", strCounter);
    printN("PUSHS LF@temp$var$strVal$%u", strCounter);
    printN("FLOAT2STRS");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Celé číslo
    printN("LABEL temp$label$str$int$%u", strCounter);
    printN("PUSHS LF@temp$var$strVal$%u", strCounter);
    printN("INT2STRS");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Řetězec
    printN("LABEL temp$label$str$string$%u", strCounter);
    printN("PUSHS LF@temp$var$strVal$%u", strCounter);
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Bool
    printN("LABEL temp$label$str$bool$%u", strCounter);
    printN("JUMPIFEQ  temp$label$str$bool$true$%u LF@temp$var$strType$%u string@bool", strCounter, strCounter);

    // Bool - false
    printN("PUSHS string@false");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Bool - true
    printN("LABEL temp$label$str$bool$true$%u", strCounter);
    printN("PUSHS string@true");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Null
    printN("LABEL temp$label$str$null$%u", strCounter);
    printN("PUSHS string@nil", strCounter);
    printN("JUMP temp$label$str$end$%u", strCounter);

    printN("LABEL temp$label$str$end$%u", strCounter);

    strCounter++;
}

void emitLength(AstNode* functionCallNode, FuncContext* context) {
    static unsigned lengthCounter = 0;

    emit_command(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("DEFVAR LF@temp$var$lengthVal$%u", lengthCounter);
    printN("DEFVAR LF@temp$var$lengthType$%u", lengthCounter);
    printN("POPS LF@temp$var$lengthVal$%u", lengthCounter);

    // Podmínka
    printN("TYPE LF@temp$var$lengthType$%u LF@temp$var$lengthVal$%u", lengthCounter, lengthCounter);
    printN("JUMPIFEQ temp$label$length$operation$%u LF@temp$var$lengthType$%u string@string", lengthCounter,
           lengthCounter);
    // Chyba
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Řetězec
    printN("LABEL temp$label$length$operation$%u", lengthCounter);
    printN("DEFVAR LF@temp$var$lengthRet$%u", lengthCounter);
    printN("STRLEN LF@temp$var$lengthRet$%u LF@temp$var$lengthVal$%u", lengthCounter, lengthCounter);
    printN("PUSHS LF@temp$var$lengthRet$%u", lengthCounter);
    printN("INT2FLOATS");

    lengthCounter++;
}

void emitSubstring(AstNode* functionCallNode, FuncContext* context) {
    static unsigned substringCounter = 0;

    emit_command(functionCallNode->child[1]->child[0], context);
    emit_command(functionCallNode->child[1]->child[1], context);
    emit_command(functionCallNode->child[1]->child[2], context);

    printN("DEFVAR LF@temp$var$substringSin$%u", substringCounter);
    printN("DEFVAR LF@temp$var$substringStart$%u", substringCounter);
    printN("DEFVAR LF@temp$var$substringEnd$%u", substringCounter);

    printN("DEFVAR LF@temp$var$substringSLen$%u", substringCounter);

    printN("DEFVAR LF@temp$var$substringSout$%u", substringCounter);
    printN("PUSHS string@");
    printN("POPS LF@temp$var$substringSout$%u", substringCounter);

    printN("DEFVAR LF@temp$var$substringType$%u", substringCounter);
    printN("DEFVAR LF@temp$var$substringCmpResult$%u", substringCounter);

    printN("POPS LF@temp$var$substringEnd$%u", substringCounter);
    printN("POPS LF@temp$var$substringStart$%u", substringCounter);
    printN("POPS LF@temp$var$substringSin$%u", substringCounter);

    // Podmínka pro vstupní řetězec
    printN("TYPE LF@temp$var$substringType$%u LF@temp$var$substringSin$%u", substringCounter, substringCounter);
    printN("JUMPIFNEQ temp$label$substring$error$%u LF@temp$var$substringType$%u string@string", substringCounter,
           substringCounter);
    printN("STRLEN LF@temp$var$substringSLen$%u LF@temp$var$substringSin$%u", substringCounter, substringCounter);
    printN("ADD LF@temp$var$substringSLen$%u  LF@temp$var$substringSLen$%u  int@1", substringCounter, substringCounter);
    // Podmínka pro počátek podřetězce
    printN("TYPE LF@temp$var$substringType$%u LF@temp$var$substringStart$%u", substringCounter, substringCounter);
    printN("JUMPIFEQ temp$label$substring$startInt$%u LF@temp$var$substringType$%u string@int", substringCounter,
           substringCounter);
    // Začátek je desetinné číslo, ale možná celočíselné
    printN("ISINT LF@temp$var$substringType$%u  LF@temp$var$substringStart$%u", substringCounter, substringCounter);
    printN("JUMPIFNEQ temp$label$substring$error$%u LF@temp$var$substringType$%u  bool@true", substringCounter,
           substringCounter);
    printN("FLOAT2INT LF@temp$var$substringStart$%u LF@temp$var$substringStart$%u", substringCounter, substringCounter);

    // Podmínka pro konec podřetězce
    printN("LABEL temp$label$substring$startInt$%u", substringCounter);
    printN("TYPE LF@temp$var$substringType$%u LF@temp$var$substringEnd$%u", substringCounter, substringCounter);
    printN("JUMPIFEQ temp$label$substring$endInt$%u LF@temp$var$substringType$%u string@int", substringCounter,
           substringCounter);
    // Konec je desetinné číslo, ale možná celočíselné
    printN("ISINT LF@temp$var$substringType$%u  LF@temp$var$substringEnd$%u", substringCounter, substringCounter);
    printN("JUMPIFNEQ temp$label$substring$error$%u LF@temp$var$substringType$%u  bool@true", substringCounter,
           substringCounter);
    printN("FLOAT2INT LF@temp$var$substringEnd$%u LF@temp$var$substringEnd$%u", substringCounter, substringCounter);

    // začátek < 0 ?
    printN("LABEL temp$label$substring$endInt$%u", substringCounter);
    printN("LT LF@temp$var$substringCmpResult$%u LF@temp$var$substringStart$%u int@0", substringCounter,
           substringCounter);
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult$%u bool@true", substringCounter,
           substringCounter);
    // konec < 0 ?
    printN("LT LF@temp$var$substringCmpResult$%u LF@temp$var$substringEnd$%u int@0", substringCounter,
           substringCounter);
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult$%u bool@true", substringCounter,
           substringCounter);
    // začátek > konec ?
    printN("GT LF@temp$var$substringCmpResult$%u LF@temp$var$substringStart$%u LF@temp$var$substringEnd$%u",
           substringCounter, substringCounter, substringCounter);
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult$%u bool@true", substringCounter,
           substringCounter);
    // začátek >= délka řetězce ? (>= je negace <)
    printN("LT LF@temp$var$substringCmpResult$%u LF@temp$var$substringEnd$%u LF@temp$var$substringSLen$%u",
           substringCounter, substringCounter, substringCounter);
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult$%u bool@false", substringCounter,
           substringCounter);
    // konec > délka řetězce ?
    printN("GT LF@temp$var$substringCmpResult$%u LF@temp$var$substringEnd$%u LF@temp$var$substringSLen$%u",
           substringCounter, substringCounter, substringCounter);
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult$%u bool@true", substringCounter,
           substringCounter);

    printN("JUMP temp$label$substring$operation$%u", substringCounter);
    // Chyba
    printN("LABEL temp$label$substring$error$%u", substringCounter);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Operace
    printN("LABEL temp$label$substring$operation$%u", substringCounter);
    // Iterátor pro výstupní řetězec
    printN("DEFVAR LF@temp$var$substringStemp$%u", substringCounter);

    printN("LABEL temp$label$substring$loopStart$%u", substringCounter);
    printN("JUMPIFEQ temp$label$substring$save$%u LF@temp$var$substringStart$%u LF@temp$var$substringEnd$%u ",
           substringCounter, substringCounter, substringCounter);
    printN("GETCHAR LF@temp$var$substringStemp$%u LF@temp$var$substringSin$%u LF@temp$var$substringStart$%u",
           substringCounter, substringCounter, substringCounter);
    printN("CONCAT LF@temp$var$substringSout$%u LF@temp$var$substringSout$%u LF@temp$var$substringStemp$%u",
           substringCounter, substringCounter, substringCounter);
    printN("ADD LF@temp$var$substringStart$%u LF@temp$var$substringStart$%u int@1", substringCounter, substringCounter,
           substringCounter);
    printN("JUMP temp$label$substring$loopStart$%u", substringCounter);

    printN("LABEL temp$label$substring$badInput$%u", substringCounter);
    printN("PUSHS nil@nil");
    printN("JUMP temp$label$substring$end$%u", substringCounter);

    printN("LABEL temp$label$substring$save$%u", substringCounter);
    printN("PUSHS LF@temp$var$substringSout$%u", substringCounter);

    printN("LABEL temp$label$substring$end$%u", substringCounter);
    substringCounter++;
}

void emitStrcmp(AstNode* functionCallNode, FuncContext* context) {
    static unsigned strcmpCounter = 0;

    emit_command(functionCallNode->child[1]->child[0], context);
    emit_command(functionCallNode->child[1]->child[1], context);

    printN("DEFVAR LF@temp$var$strcmpS1$%u", strcmpCounter);
    printN("DEFVAR LF@temp$var$strcmpS2$%u", strcmpCounter);
    printN("DEFVAR LF@temp$var$strcmpType$%u", strcmpCounter);
    printN("DEFVAR LF@temp$var$strcmpCmpResult$%u", strcmpCounter);

    printN("POPS LF@temp$var$strcmpS2$%u", strcmpCounter);
    printN("POPS LF@temp$var$strcmpS1$%u", strcmpCounter);

    // Podmínka pro řetězec 1
    printN("TYPE LF@temp$var$strcmpType$%u LF@temp$var$strcmpS1$%u", strcmpCounter, strcmpCounter);
    printN("JUMPIFNEQ temp$label$strcmp$error$%u LF@temp$var$strcmpType$%u string@string", strcmpCounter,
           strcmpCounter);

    // Podmínka pro řetězec 2
    printN("TYPE LF@temp$var$strcmpType$%u LF@temp$var$strcmpS2$%u", strcmpCounter, strcmpCounter);
    printN("JUMPIFNEQ temp$label$strcmp$error$%u LF@temp$var$strcmpType$%u string@string", strcmpCounter,
           strcmpCounter);

    printN("JUMP temp$label$strcmp$operation$%u", strcmpCounter);
    // Chyba
    printN("LABEL temp$label$strcmp$error$%u", strcmpCounter);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Operace
    printN("LABEL temp$label$strcmp$operation$%u", strcmpCounter);
    // Operace - s1 < s2
    printN("LT LF@temp$var$strcmpCmpResult$%u LF@temp$var$strcmpS1$%u LF@temp$var$strcmpS2$%u", strcmpCounter,
           strcmpCounter, strcmpCounter);
    printN("JUMPIFEQ temp$label$strcmp$smaller$%u LF@temp$var$strcmpCmpResult$%u bool@true", strcmpCounter,
           strcmpCounter);
    // Operace - s1 > s2
    printN("GT LF@temp$var$strcmpCmpResult$%u LF@temp$var$strcmpS1$%u LF@temp$var$strcmpS2$%u", strcmpCounter,
           strcmpCounter, strcmpCounter);
    printN("JUMPIFEQ temp$label$strcmp$higher$%u LF@temp$var$strcmpCmpResult$%u bool@true", strcmpCounter,
           strcmpCounter);
    // Operace - s1 = s2
	printN("JUMP temp$label$strcmp$equal$%u", strcmpCounter);

    // s1 < s2
    printN("LABEL temp$label$strcmp$smaller$%u", strcmpCounter);
    printN("PUSHS int@-1");
    printN("INT2FLOATS");
    printN("JUMP temp$label$strcmp$end$%u", strcmpCounter);
    // s1 > s2
    printN("LABEL temp$label$strcmp$higher$%u", strcmpCounter);
    printN("PUSHS int@1");
    printN("INT2FLOATS");
    printN("JUMP temp$label$strcmp$end$%u", strcmpCounter);
    // s1 = s2
    printN("LABEL temp$label$strcmp$equal$%u", strcmpCounter);
    printN("PUSHS int@0");
	printN("INT2FLOATS");
    printN("JUMP temp$label$strcmp$end$%u", strcmpCounter);

    printN("LABEL temp$label$strcmp$end$%u", strcmpCounter);

    strcmpCounter++;
}

void emitOrd(AstNode* functionCallNode, FuncContext* context) {
    static unsigned ordCounter = 0;

    emit_command(functionCallNode->child[1]->child[0], context);
    emit_command(functionCallNode->child[1]->child[1], context);

    // Parametry
    printN("DEFVAR LF@temp$var$ordStr$%u", ordCounter);
    printN("DEFVAR LF@temp$var$ordNum$%u", ordCounter);
    printN("DEFVAR LF@temp$var$ordType$%u", ordCounter);
    printN("POPS LF@temp$var$ordNum$%u", ordCounter);
    printN("POPS LF@temp$var$ordStr$%u", ordCounter);

    // Podmínka pro řetězec
    printN("TYPE LF@temp$var$ordType$%u LF@temp$var$ordStr$%u", ordCounter, ordCounter);
    printN("JUMPIFNEQ temp$label$ord$error$%u LF@temp$var$ordType$%u string@string", ordCounter, ordCounter);

    // Podmínka pro číslo
    printN("TYPE LF@temp$var$ordType$%u LF@temp$var$ordNum$%u", ordCounter, ordCounter);
    printN("JUMPIFEQ temp$label$ord$operation$%u LF@temp$var$ordType$%u string@int", ordCounter, ordCounter);
    // Desetinné číslo, ale možná celočíselné
    printN("DEFVAR LF@temp$var$ordIsInt$%u", ordCounter);
    printN("ISINT LF@temp$var$ordIsInt$%u LF@temp$var$ordNum$%u", ordCounter, ordCounter);
    printN("JUMPIFEQ temp$label$ord$float$%u LF@temp$var$ordIsInt$%u bool@true", ordCounter, ordCounter);
    // Chyba
    printN("LABEL temp$label$ord$error$%u", ordCounter);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Celé desetinné číslo
    printN("LABEL temp$label$ord$float$%u", ordCounter);
    printN("FLOAT2INT LF@temp$var$ordNum$%u LF@temp$var$ordNum$%u", ordCounter, ordCounter);

    // Celé číslo
    printN("LABEL temp$label$ord$operation$%u", ordCounter);
    printN("DEFVAR LF@temp$var$ordOut$%u", ordCounter);
    printN("STRI2INT LF@temp$var$ordOut$%u LF@temp$var$ordStr$%u LF@temp$var$ordNum$%u", ordCounter, ordCounter,
           ordCounter);
    printN("PUSHS LF@temp$var$ordOut$%u", ordCounter);
	printN("INT2FLOATS");

    ordCounter++;
}

void emitChr(AstNode* functionCallNode, FuncContext* context) {
    static unsigned chrCounter = 0;

    emit_command(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("DEFVAR LF@temp$var$chrVal$%u", chrCounter);
    printN("DEFVAR LF@temp$var$chrType$%u", chrCounter);
    printN("POPS LF@temp$var$chrVal$%u", chrCounter);

    // Podmínka
    printN("TYPE LF@temp$var$chrType$%u LF@temp$var$chrVal$%u", chrCounter, chrCounter);
    printN("JUMPIFEQ temp$label$chr$operation$%u LF@temp$var$chrType$%u string@int", chrCounter, chrCounter);
    // Desetinné číslo, ale možná celočíselný

    printN("DEFVAR LF@temp$var$chrIsInt$%u", chrCounter);
    printN("ISINT LF@temp$var$chrIsInt$%u LF@temp$var$chrVal$%u", chrCounter, chrCounter);
    printN("JUMPIFEQ temp$label$chr$float$%u LF@temp$var$chrIsInt$%u bool@true", chrCounter, chrCounter);
    // Chyba
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Celé desetinné číslo
    printN("LABEL temp$label$chr$float$%u", chrCounter);
    printN("FLOAT2INT LF@temp$var$chrVal$%u LF@temp$var$chrVal$%u", chrCounter, chrCounter);

    // Celé číslo
    printN("LABEL temp$label$chr$operation$%u", chrCounter);
    printN("PUSHS LF@temp$var$chrVal$%u", chrCounter);
    printN("INT2CHARS");

    chrCounter++;
}

// TODO Patička
