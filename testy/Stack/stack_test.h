/**
 * Fakulta informačních technologií VUT Brno
 *
 * Hlavička testů pro dynamicStack
 * @file stackTest.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "dynamic_stack.h"
#include "support_functions.h"

#include <stdio.h>
#include <string.h>

DynamicStackPtr testInit(unsigned size, char* testName);
void testDestruct(DynamicStackPtr stack);
void printStack(DynamicStackPtr stack);

 /*** konec stackTest.c ***/