/**
 * Fakulta informačních technologií VUT Brno
 *
 * Implementace pro dynamický zásobník (prostor se zvětšuje i zmenšuje dle naplnění) s podporou
 * určení datového typu před překladem
 * @file dynamic_stack.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "dynamic_stack.h"

/**
 * @brief Makro pro definici zásobníkových funkcí pro určitý datový typ
 *
 * Definuje funkcí pro práci se zásobníkem s formátem stackNAMEOperace
 * (stackIntPush)
 * @param TYPE Datový typ data v poli zásobníku
 * @param NAME Řetězec pro odlišení funkcí prodaný typ zásobníku
 */
#define DEFINE_STACK(TYPE, NAME)                                                                  \
    /**                                                                                           \
     * @brief Obstarává změnu velikosti zásobníku a změnu nextDownsizeAt a nextUpsizeAt     \
     *                                                                                            \
     * @param stack Ukazatel na zásobník                                                        \
     * @param wantedSize Požadovaný počet prvků                                               \
     * @return true Operace proběhla v pořádku                                                 \
     * @return false Došlo k chybě                                                              \
     */                                                                                           \
    bool stack##NAME##Resize(Dynamic##NAME##StackPtr stack, unsigned wantedSize);                 \
                                                                                                  \
    Dynamic##NAME##StackPtr stack##NAME##Construct(unsigned initStackSize) {                      \
        if (initStackSize < 2) {                                                                  \
            printError(INTERNAL_ERROR, "(%s) Velikost zasobniku nemuze byt mensi nez 2!\n",       \
                       __func__);                                                                 \
            return NULL;                                                                          \
        }                                                                                         \
                                                                                                  \
        Dynamic##NAME##StackPtr newStack = malloc(sizeof(Dynamic##NAME##Stack));                  \
        if (newStack == NULL) {                                                                   \
            printError(INTERNAL_ERROR, "(%s) Pri alokaci zasobniku doslo k chybe!\n", __func__);  \
            return NULL;                                                                          \
        }                                                                                         \
                                                                                                  \
        newStack->array = malloc(sizeof(TYPE) * initStackSize);                                   \
        if (newStack->array == NULL) {                                                            \
            printError(                                                                           \
                INTERNAL_ERROR,                                                                   \
                "(%s) Pri alokaci prostoru pro data zasobniku doslo k chybe. (Zadana velikost "   \
                "prostoru: %u \n",                                                                \
                __func__, initStackSize * sizeof(TYPE));                                          \
            free(newStack);                                                                       \
            return NULL;                                                                          \
        }                                                                                         \
                                                                                                  \
        newStack->nextTopElement = 0;                                                             \
        newStack->maxSize        = initStackSize;                                                 \
        newStack->nextDownsizeAt = 0;                                                             \
        newStack->nextUpsizeAt   = newStack->maxSize / 2;                                         \
        newStack->resized        = false;                                                         \
                                                                                                  \
        return newStack;                                                                          \
    }                                                                                             \
                                                                                                  \
    bool stack##NAME##Push(Dynamic##NAME##StackPtr stack, TYPE data) {                            \
        if (stack == NULL) {                                                                      \
            printError(INTERNAL_ERROR, "(%s) Nebyl predan zasobnik!\n", __func__);                \
            return false;                                                                         \
        }                                                                                         \
                                                                                                  \
        stack->resized = false;                                                                   \
                                                                                                  \
        if (stack->nextTopElement >= stack->nextUpsizeAt) {                                       \
            if (stack##NAME##Resize(stack, stack->maxSize * 2) == false) {                        \
                /* Selhání zvětšení je sice problém, ale neznemožňuje přidání dat do zásobníku */ \
            }                                                                                     \
        }                                                                                         \
                                                                                                  \
        if (stack->nextTopElement == stack->maxSize) {                                            \
            printWarning("(%s) Zasobnik je plny!\n", __func__);                                   \
            return false;                                                                         \
        }                                                                                         \
                                                                                                  \
        stack->array[stack->nextTopElement] = data;                                               \
        stack->nextTopElement++;                                                                  \
        return true;                                                                              \
    }                                                                                             \
                                                                                                  \
    TYPE stack##NAME##Read(Dynamic##NAME##StackPtr stack) {                                       \
        return stack->array[stack->nextTopElement - 1];                                           \
    }                                                                                             \
                                                                                                  \
    bool stack##NAME##Pop(Dynamic##NAME##StackPtr stack) {                                        \
        if (stack == NULL) {                                                                      \
            printError(INTERNAL_ERROR, "(%s) Nebyl predan zasobnik!\n", __func__);                \
            return false;                                                                         \
        }                                                                                         \
                                                                                                  \
        if (stack->nextTopElement == 0) {                                                         \
            printWarning("(%s) Zasobnik je prazdny!\n", __func__);                                \
            return false;                                                                         \
        }                                                                                         \
                                                                                                  \
        stack->resized = false;                                                                   \
                                                                                                  \
        stack->nextTopElement--;                                                                  \
                                                                                                  \
        if (stack->nextTopElement < stack->nextDownsizeAt) {                                      \
            if (stack##NAME##Resize(stack, stack->maxSize / 2) == false) {                        \
                /* Selhání realokace je sice problém ale smazání proběhlo*/                       \
            }                                                                                     \
        }                                                                                         \
                                                                                                  \
        return true;                                                                              \
    }                                                                                             \
                                                                                                  \
    bool stack##NAME##IsEmpty(Dynamic##NAME##StackPtr stack) {                                    \
        if (stack == NULL) {                                                                      \
            printError(INTERNAL_ERROR, "(%s) Nebyl predan zasobnik!\n", __func__);                \
            return false;                                                                         \
        }                                                                                         \
                                                                                                  \
        return stack->nextTopElement == 0;                                                        \
    }                                                                                             \
                                                                                                  \
    void stack##NAME##Destruct(Dynamic##NAME##StackPtr stack) {                                   \
        if (stack != NULL) {                                                                      \
            free(stack->array);                                                                   \
            free(stack);                                                                          \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    bool stack##NAME##Resize(Dynamic##NAME##StackPtr stack, unsigned wantedSize) {                \
        unsigned memorySize = wantedSize * sizeof(TYPE);                                          \
        if (stack == NULL) {                                                                      \
            printError(INTERNAL_ERROR, "(%s) Nebyl predan zasobnik!\n", __func__);                \
            return false;                                                                         \
        }                                                                                         \
                                                                                                  \
        if (wantedSize < stack->maxSize && stack->nextTopElement > wantedSize) {                  \
            printWarning("(%s) Zmenseni zasobniku zpusobilo ztratu dat!\n", __func__);            \
        }                                                                                         \
                                                                                                  \
        void* newStackarray = realloc(stack->array, memorySize);                                  \
        if (newStackarray == NULL) {                                                              \
            printWarning(                                                                         \
                "(%s) Realokace zasobniku selhala! Pozadovany pocet prvku: %u. Pozadovana "       \
                "velikost v "                                                                     \
                "pameti: %u.\n",                                                                  \
                __func__, wantedSize, memorySize);                                                \
            return false;                                                                         \
        }                                                                                         \
                                                                                                  \
        stack->array = newStackarray;                                                             \
                                                                                                  \
        stack->maxSize      = wantedSize;                                                         \
        stack->nextUpsizeAt = stack->maxSize / 2;                                                 \
                                                                                                  \
        if (stack->maxSize == 2) {                                                                \
            stack->nextDownsizeAt = 0;                                                            \
        } else {                                                                                  \
            stack->nextDownsizeAt = stack->maxSize / 4;                                           \
        }                                                                                         \
                                                                                                  \
        if (stack->nextTopElement > wantedSize) {                                                 \
            stack->nextTopElement = wantedSize;                                                   \
        }                                                                                         \
                                                                                                  \
        stack->resized = true;                                                                    \
        return true;                                                                              \
    }

// DEFINE_STACK(char*, String) // Slouží pro testování

DEFINE_STACK(Symbol, Symbol)
DEFINE_STACK(AstNode*, AstNode)
DEFINE_STACK(Token, Precedence)
/*** konec dynamic_stack.c ***/
