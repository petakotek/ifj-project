/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Implementaci výpisu kódu pro vestavěné funkce
 * @file builtin_functions.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

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
 * @brief Funkce pro výpis libovolné hodnoty na výstup
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
 * @brief Funkce pro převod libovolné vstupní hodnoty na odpovídající řetězec
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
 * @brief Funkce pro vytvoření podřetězce
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
    printN("READ LF@temp$var$readStr string");
    printN("PUSHS LF@temp$var$readStr");
}

void emitReadNum() {
    printN("READ LF@temp$var$readNum float");
    printN("PUSHS LF@temp$var$readNum");
}

void emitReadBool() {
    printN("READ LF@temp$var$readBool bool");
    printN("PUSHS LF@temp$var$readBool");
}

void emitWrite(AstNode* functionCallNode, FuncContext* context) {
    static unsigned writeCounter = 0;

    emitCommand(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("POPS LF@temp$var$write");

    // Podmínka
    printN("TYPE LF@temp$var$writeType LF@temp$var$write");
    printN("JUMPIFNEQ temp$label$write$write$%u LF@temp$var$writeType string@float", writeCounter);

    // Výstup je desetinné číslo, ale možná celočíselné
    printN("ISINT LF@temp$var$writeType LF@temp$var$write");
    printN("JUMPIFNEQ temp$label$write$write$%u LF@temp$var$writeType bool@true", writeCounter);
    printN("FLOAT2INT LF@temp$var$write LF@temp$var$write");

    printN("LABEL temp$label$write$write$%u", writeCounter);
    printN("WRITE LF@temp$var$write");
    printN("PUSHS nil@nil");

    writeCounter++;
}

void emitFloor(AstNode* functionCallNode, FuncContext* context) {
    static unsigned floorCounter = 0;

    emitCommand(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("POPS LF@temp$var$floorVal");

    // Podmínka
    printN("TYPE LF@temp$var$floorType LF@temp$var$floorVal");
    printN("JUMPIFEQ temp$label$floor$operation$%u LF@temp$var$floorType string@float", floorCounter);
    printN("JUMPIFEQ temp$label$floor$noOp$%u LF@temp$var$floorType string@int", floorCounter);
    // Chyba
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Desetinné číslo
    printN("LABEL temp$label$floor$operation$%u", floorCounter);
    printN("PUSHS LF@temp$var$floorVal");
    printN("FLOAT2INTS");
    printN("INT2FLOATS");
    printN("JUMP temp$label$floor$end$%u", floorCounter);

    // Celé číslo
    printN("LABEL temp$label$floor$noOp$%u", floorCounter);
    printN("PUSHS LF@temp$var$floorVal");
    printN("INT2FLOATS");
    printN("JUMP temp$label$floor$end$%u", floorCounter);

    printN("LABEL temp$label$floor$end$%u", floorCounter);

    floorCounter++;
}

void emitStr(AstNode* functionCallNode, FuncContext* context) {
    static unsigned strCounter = 0;

    emitCommand(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("POPS LF@temp$var$strVal");

    // Podmínka
    printN("TYPE LF@temp$var$strType LF@temp$var$strVal");
    printN("JUMPIFEQ temp$label$str$float$%u LF@temp$var$strType string@float", strCounter);
    printN("JUMPIFEQ temp$label$str$int$%u LF@temp$var$strType string@int", strCounter);
    printN("JUMPIFEQ temp$label$str$string$%u LF@temp$var$strType string@string", strCounter);
    printN("JUMPIFEQ temp$label$str$bool$%u LF@temp$var$strType string@bool", strCounter);
    printN("JUMPIFEQ temp$label$str$null$%u LF@temp$var$strType string@nil", strCounter);

    // Chyba
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Desetinné číslo
    printN("LABEL temp$label$str$float$%u", strCounter);
    // Desetinné číslo je možná celočíselné
    printN("ISINT LF@temp$var$strType LF@temp$var$strVal");
    printN("JUMPIFEQ temp$label$str$floatInt$%u LF@temp$var$strType bool@true", strCounter);

    printN("PUSHS LF@temp$var$strVal");
    printN("FLOAT2STRS");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Celé číslo
    printN("LABEL temp$label$str$floatInt$%u", strCounter);
    printN("FLOAT2INT LF@temp$var$strVal LF@temp$var$strVal");
    printN("LABEL temp$label$str$int$%u", strCounter);
    printN("PUSHS LF@temp$var$strVal");
    printN("INT2STRS");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Řetězec
    printN("LABEL temp$label$str$string$%u", strCounter);
    printN("PUSHS LF@temp$var$strVal");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Bool
    printN("LABEL temp$label$str$bool$%u", strCounter);
    printN("JUMPIFEQ temp$label$str$bool$true$%u LF@temp$var$strType string@bool", strCounter);

    // Bool - false
    printN("PUSHS string@false");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Bool - true
    printN("LABEL temp$label$str$bool$true$%u", strCounter);
    printN("PUSHS string@true");
    printN("JUMP temp$label$str$end$%u", strCounter);

    // Null
    printN("LABEL temp$label$str$null$%u", strCounter);
    printN("PUSHS string@nil");
    printN("JUMP temp$label$str$end$%u", strCounter);

    printN("LABEL temp$label$str$end$%u", strCounter);

    strCounter++;
}

void emitLength(AstNode* functionCallNode, FuncContext* context) {
    static unsigned lengthCounter = 0;

    emitCommand(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("POPS LF@temp$var$lengthVal");

    // Podmínka
    printN("TYPE LF@temp$var$lengthType LF@temp$var$lengthVal");
    printN("JUMPIFEQ temp$label$length$operation$%u LF@temp$var$lengthType string@string", lengthCounter);
    // Chyba
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Řetězec
    printN("LABEL temp$label$length$operation$%u", lengthCounter);
    printN("STRLEN LF@temp$var$lengthRet LF@temp$var$lengthVal");
    printN("PUSHS LF@temp$var$lengthRet");
    printN("INT2FLOATS");

    lengthCounter++;
}

void emitSubstring(AstNode* functionCallNode, FuncContext* context) {
    static unsigned substringCounter = 0;

    emitCommand(functionCallNode->child[1]->child[0], context);
    emitCommand(functionCallNode->child[1]->child[1], context);
    emitCommand(functionCallNode->child[1]->child[2], context);

    printN("PUSHS string@");
    printN("POPS LF@temp$var$substringSout");

    printN("POPS LF@temp$var$substringEnd");
    printN("POPS LF@temp$var$substringStart");
    printN("POPS LF@temp$var$substringSin");

    // Podmínka pro vstupní řetězec
    printN("TYPE LF@temp$var$substringType LF@temp$var$substringSin");
    printN("JUMPIFNEQ temp$label$substring$error$%u LF@temp$var$substringType string@string", substringCounter);
    printN("STRLEN LF@temp$var$substringSLen LF@temp$var$substringSin");
    printN("ADD LF@temp$var$substringSLen LF@temp$var$substringSLen int@1");
    // Podmínka pro počátek podřetězce
    printN("TYPE LF@temp$var$substringType LF@temp$var$substringStart");
    printN("JUMPIFEQ temp$label$substring$startInt$%u LF@temp$var$substringType string@int", substringCounter);
    // Začátek je desetinné číslo, ale možná celočíselné
    printN("ISINT LF@temp$var$substringType LF@temp$var$substringStart");
    printN("JUMPIFNEQ temp$label$substring$error$%u LF@temp$var$substringType bool@true", substringCounter);
    printN("FLOAT2INT LF@temp$var$substringStart LF@temp$var$substringStart");

    // Podmínka pro konec podřetězce
    printN("LABEL temp$label$substring$startInt$%u", substringCounter);
    printN("TYPE LF@temp$var$substringType LF@temp$var$substringEnd");
    printN("JUMPIFEQ temp$label$substring$endInt$%u LF@temp$var$substringType string@int", substringCounter);
    // Konec je desetinné číslo, ale možná celočíselné
    printN("ISINT LF@temp$var$substringType LF@temp$var$substringEnd");
    printN("JUMPIFNEQ temp$label$substring$error$%u LF@temp$var$substringType bool@true", substringCounter);
    printN("FLOAT2INT LF@temp$var$substringEnd LF@temp$var$substringEnd");

    // začátek < 0 ?
    printN("LABEL temp$label$substring$endInt$%u", substringCounter);
    printN("LT LF@temp$var$substringCmpResult LF@temp$var$substringStart int@0");
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult bool@true", substringCounter);
    // konec < 0 ?
    printN("LT LF@temp$var$substringCmpResult LF@temp$var$substringEnd int@0");
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult bool@true", substringCounter);
    // začátek > konec ?
    printN("GT LF@temp$var$substringCmpResult LF@temp$var$substringStart LF@temp$var$substringEnd");
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult bool@true", substringCounter);
    // začátek >= délka řetězce ? (>= je negace <)
    printN("LT LF@temp$var$substringCmpResult LF@temp$var$substringEnd LF@temp$var$substringSLen");
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult bool@false", substringCounter);
    // konec > délka řetězce ?
    printN("GT LF@temp$var$substringCmpResult LF@temp$var$substringEnd LF@temp$var$substringSLen");
    printN("JUMPIFEQ temp$label$substring$badInput$%u LF@temp$var$substringCmpResult bool@true", substringCounter);

    printN("JUMP temp$label$substring$operation$%u", substringCounter);
    // Chyba
    printN("LABEL temp$label$substring$error$%u", substringCounter);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Operace
    printN("LABEL temp$label$substring$operation$%u", substringCounter);
    // Iterátor pro výstupní řetězec

    printN("LABEL temp$label$substring$loopStart$%u", substringCounter);
    printN("JUMPIFEQ temp$label$substring$save$%u LF@temp$var$substringStart LF@temp$var$substringEnd",
           substringCounter);
    printN("GETCHAR LF@temp$var$substringStemp LF@temp$var$substringSin LF@temp$var$substringStart");
    printN("CONCAT LF@temp$var$substringSout LF@temp$var$substringSout LF@temp$var$substringStemp");
    printN("ADD LF@temp$var$substringStart LF@temp$var$substringStart int@1");
    printN("JUMP temp$label$substring$loopStart$%u", substringCounter);

    printN("LABEL temp$label$substring$badInput$%u", substringCounter);
    printN("PUSHS nil@nil");
    printN("JUMP temp$label$substring$end$%u", substringCounter);

    printN("LABEL temp$label$substring$save$%u", substringCounter);
    printN("PUSHS LF@temp$var$substringSout");

    printN("LABEL temp$label$substring$end$%u", substringCounter);
    substringCounter++;
}

void emitStrcmp(AstNode* functionCallNode, FuncContext* context) {
    static unsigned strcmpCounter = 0;

    emitCommand(functionCallNode->child[1]->child[0], context);
    emitCommand(functionCallNode->child[1]->child[1], context);

    printN("POPS LF@temp$var$strcmpS2");
    printN("POPS LF@temp$var$strcmpS1");

    // Podmínka pro řetězec 1
    printN("TYPE LF@temp$var$strcmpType LF@temp$var$strcmpS1");
    printN("JUMPIFNEQ temp$label$strcmp$error$%u LF@temp$var$strcmpType string@string", strcmpCounter);

    // Podmínka pro řetězec 2
    printN("TYPE LF@temp$var$strcmpType LF@temp$var$strcmpS2");
    printN("JUMPIFNEQ temp$label$strcmp$error$%u LF@temp$var$strcmpType string@string", strcmpCounter);

    printN("JUMP temp$label$strcmp$operation$%u", strcmpCounter);
    // Chyba
    printN("LABEL temp$label$strcmp$error$%u", strcmpCounter);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Operace
    printN("LABEL temp$label$strcmp$operation$%u", strcmpCounter);
    // Operace - s1 < s2
    printN("LT LF@temp$var$strcmpCmpResult LF@temp$var$strcmpS1 LF@temp$var$strcmpS2");
    printN("JUMPIFEQ temp$label$strcmp$smaller$%u LF@temp$var$strcmpCmpResult bool@true", strcmpCounter);
    // Operace - s1 > s2
    printN("GT LF@temp$var$strcmpCmpResult LF@temp$var$strcmpS1 LF@temp$var$strcmpS2");
    printN("JUMPIFEQ temp$label$strcmp$higher$%u LF@temp$var$strcmpCmpResult bool@true", strcmpCounter);
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

    emitCommand(functionCallNode->child[1]->child[0], context);
    emitCommand(functionCallNode->child[1]->child[1], context);

    // Parametry
    printN("POPS LF@temp$var$ordNum");
    printN("POPS LF@temp$var$ordStr");

    // Podmínka pro řetězec
    printN("TYPE LF@temp$var$ordType LF@temp$var$ordStr");
    printN("JUMPIFNEQ temp$label$ord$error$%u LF@temp$var$ordType string@string", ordCounter);

    // Podmínka pro číslo
    printN("TYPE LF@temp$var$ordType LF@temp$var$ordNum");
    printN("JUMPIFEQ temp$label$ord$operation$%u LF@temp$var$ordType string@int", ordCounter);
    // Desetinné číslo, ale možná celočíselné
    printN("ISINT LF@temp$var$ordIsInt LF@temp$var$ordNum");
    printN("JUMPIFEQ temp$label$ord$float$%u LF@temp$var$ordIsInt bool@true", ordCounter);
    // Chyba
    printN("LABEL temp$label$ord$error$%u", ordCounter);
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Celé desetinné číslo
    printN("LABEL temp$label$ord$float$%u", ordCounter);
    printN("FLOAT2INT LF@temp$var$ordNum LF@temp$var$ordNum");

    // Celé číslo
    printN("LABEL temp$label$ord$operation$%u", ordCounter);
    printN("STRI2INT LF@temp$var$ordOut LF@temp$var$ordStr LF@temp$var$ordNum");
    printN("PUSHS LF@temp$var$ordOut");
    printN("INT2FLOATS");

    ordCounter++;
}

void emitChr(AstNode* functionCallNode, FuncContext* context) {
    static unsigned chrCounter = 0;

    emitCommand(functionCallNode->child[1]->child[0], context);

    // Parametry
    printN("POPS LF@temp$var$chrVal");

    // Podmínka
    printN("TYPE LF@temp$var$chrType LF@temp$var$chrVal");
    printN("JUMPIFEQ temp$label$chr$operation$%u LF@temp$var$chrType string@int", chrCounter);
    // Desetinné číslo, ale možná celočíselný
    printN("ISINT LF@temp$var$chrIsInt LF@temp$var$chrVal");
    printN("JUMPIFEQ temp$label$chr$float$%u LF@temp$var$chrIsInt bool@true", chrCounter);
    // Chyba
    printN("EXIT int@%i", RUNTIME_SEMANTICS_ERROR_BAD_TYPE);

    // Celé desetinné číslo
    printN("LABEL temp$label$chr$float$%u", chrCounter);
    printN("FLOAT2INT LF@temp$var$chrVal LF@temp$var$chrVal");

    // Celé číslo
    printN("LABEL temp$label$chr$operation$%u", chrCounter);
    printN("PUSHS LF@temp$var$chrVal");
    printN("INT2CHARS");

    chrCounter++;
}

/*** konec builtin_functions.c ***/
