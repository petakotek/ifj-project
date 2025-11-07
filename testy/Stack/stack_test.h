/**
 * Fakulta informačních technologií VUT Brno
 *
 * Hlavička testů pro dynamicStack
 * @file stackTest.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#ifndef STAKC_TEST
#define STAKC_TEST
#include "dynamic_stack.h"
#include "support_functions.h"

#include <stdio.h>
#include <string.h>

DynamicStringStackPtr testInit(unsigned size, char* testName);
void testDestruct(DynamicStringStackPtr stack);
void printStack(DynamicStringStackPtr stack);
#endif
 /*** konec stackTest.c ***/