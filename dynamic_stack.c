/**
 * Fakulta informačních technologií VUT Brno
 *
 * Implementace zásobníku s dynamickou alokací paměti
 * @file dynamic_stack.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "dynamic_stack.h"

static DynamicStackPtr grammarStack = NULL;  // TODO a/nebo global?

/**
 * @brief Obstarává změno velikosti zásobníku a změnu nextDownsizeAt a nextUpsizeAt
 *
 * @param stack Ukazatel na zásobník
 * @param wantedSize Požadovaná velikost
 * @return true Operace proběhla v pořádku
 * @return false Došlo k chybě
 */
bool stackResize(DynamicStackPtr stack, unsigned wantedSize);

DynamicStackPtr stackConstruct(unsigned initStackSize) {
    if (initStackSize < 2) {
        printError(INTERNAL_ERROR, "Velikost zasobniku nemuze byt mensi nez 2!\n");
        return NULL;
    }

    DynamicStackPtr newStack = malloc(sizeof(DynamicStack));
    if (newStack == NULL) {
        printError(INTERNAL_ERROR, "Pri alokaci zasobniku doslo k chybe!\n");
        return NULL;
    }

    newStack->array = malloc(sizeof(StackElementPtr) * initStackSize);
    if (newStack->array == NULL) {
        printError(INTERNAL_ERROR,
                   "Pri alokaci prostoru pro data zasobniku doslo k chybe. (Zadana velikost "
                   "prostoru: %u \n",
                   initStackSize * sizeof(StackElementPtr));
        free(newStack);
        return NULL;
    }

    newStack->nextTopElement = 0;
    newStack->maxSize        = initStackSize;
    newStack->nextDownsizeAt = 0;
    newStack->nextUpsizeAt   = newStack->maxSize / 2;
    newStack->resized        = false;

    newStack->array[0] = NULL;

    return newStack;
}

bool stackPush(DynamicStackPtr stack, StackElementPtr data) {
    if (stack == NULL) {
        printError(INTERNAL_ERROR, "(stackPush) Nebyl predan zasobnik!\n");
        return false;
    }
    if (data == NULL) {
        printError(INTERNAL_ERROR, "(stackPush) Nebyl predan prvek!\n");
        return false;
    }

    stack->resized = false;

    if (stack->nextTopElement >= stack->nextUpsizeAt) {
        if (stackResize(stack, stack->maxSize * 2) == false) {
            // Selhání zvětšení je sice problém, ale neznemožňuje pracovat se zásobníkem
        }
    }

    if (stack->nextTopElement == stack->maxSize) {
        printWarning("Zasobnik je plny!\n");
        return false;
    }

    stack->array[stack->nextTopElement] = data;
    stack->nextTopElement++;
    stack->array[stack->nextTopElement] = NULL;
    return true;
}

StackElementPtr stackRead(DynamicStackPtr stack) {
    if (stack == NULL) {
        printError(INTERNAL_ERROR, "(stackRead) Nebyl predan zasobnik!\n");
        return NULL;
    }
    if (stack->nextTopElement == 0) {
        printWarning("Cteni z prazdneho zasobniku!\n");
        return NULL;
    }

    return stack->array[stack->nextTopElement - 1];
}

bool stackPop(DynamicStackPtr stack) {
    if (stack == NULL) {
        printError(INTERNAL_ERROR, "(stackPop) Nebyl predan zasobnik!\n");
        return false;
    }

    if (stack->nextTopElement == 0) {
        printWarning("Zasobnik je prazdny!\n");
        return false;
    }

    stack->resized = false;

    stack->nextTopElement--;
    stack->array[stack->nextTopElement] = NULL;

    if (stack->nextTopElement < stack->nextDownsizeAt) {
        if (stackResize(stack, stack->maxSize / 2) == false) {
            // Selhání realokace je sice problém ale smazání proběhlo
        }
    }

    return true;
}

void stackDestruct(DynamicStackPtr stack) {
    if (stack != NULL) {
        free(stack->array);
        free(stack);
    }
}

bool stackResize(DynamicStackPtr stack, unsigned wantedSize) {
    if (stack == NULL) {
        printError(INTERNAL_ERROR, "(stackResize) Nebyl predan zasobnik!\n");
        return false;
    }

    if (wantedSize < stack->maxSize && stack->nextTopElement > wantedSize) {
        printWarning("Zmenseni zasobniku zpusobilo ztratu dat!\n");
    }

    StackElementPtr* newStackarray = realloc(stack->array, sizeof(StackElementPtr) * wantedSize);
    if (newStackarray == NULL) {
        printWarning("Realokace zasobniku selhala! Pozadovana velikost: %u.\n", wantedSize*sizeof(StackElementPtr));
        return false;
    }

    stack->array = newStackarray;

    stack->maxSize      = wantedSize;
    stack->nextUpsizeAt = stack->maxSize / 2;

    if (stack->maxSize == 2) {
        stack->nextDownsizeAt = 0;
    } else {
        stack->nextDownsizeAt = stack->maxSize / 4;
    }

    if (stack->nextTopElement > wantedSize) {
        stack->nextTopElement = wantedSize;
    }

    stack->resized = true;
    return true;
}

/*** konec dynamic_stack.c ***/