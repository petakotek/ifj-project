/**
 * Fakulta informačních technologií VUT Brno
 *
 * Hlavička zásobníku s dynamickou alokací paměti
 * @file dynamic_stack.h
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#ifndef DYNAMIC_STACK
#define DYNAMIC_STACK 1

#include <stdbool.h>
#include <stddef.h>
#include "support_functions.h"
#include "symbol.h"

/**
 * @brief Základní velikost zásobníku (možné použít při volání stackConstruct)
 * 
 */
#define DEFAULT_INIT_SIZE 20

/**
 * @brief Datový typ pro jednotlivé položky zásobníku
 * 
 */
typedef char* StackElementPtr; // TODO asi potřeba udělat jinak, záleží na použití
//OD KUBA VSADE KDE BOL StackElementPtr JE SYMBOL NIC INE SOM NEMENIL

/**
 * @brief Zásobník s automatickým zvětšením/zmenšením
 * 
 */
typedef struct{
    /**
     * @brief Uložení dat
     * 
     */
    Symbol* array;
    /**
     * @brief Nejnižší prázdné pole zásobníku (hned nad vrcholem zásobníku)
     * 
     */
    unsigned nextTopElement;
    /**
     * @brief Alokovaná zásobníku
     * 
     */
    unsigned maxSize;
    /**
     * @brief Při překročení hodnoty dojde k zdvojnásobení velikosti zásobníku
     * 
     * maxSize/2
     */
    unsigned nextUpsizeAt;
    /**
     * @brief Při podkročení hodnoty dojde k zmenšení zásobníku o polovinu
     * 
     * maxSize/4
     */
    unsigned nextDownsizeAt;
    /**
     * @brief Značí zda během poslední operace došlo ke změně velikosti
     * 
     */
    bool resized;
} DynamicStack;

/**
 * @brief Syntaktické pozlátko pro ukazatel na strukturu zásobníku
 * 
 */
typedef DynamicStack * DynamicStackPtr;

/**
 * @brief Konstruktor zásobníku
 * 
 * Alokuje strukturu zásobníku a pole pro data, nastaví počáteční hodnoty 
 * @param initStackSize Počáteční velikost zásobníku (>1)
 * @return DynamicStackPtr 
 */
DynamicStackPtr stackConstruct(unsigned initStackSize);

/**
 * @brief Přidá hodnotu na zásobník, v případě potřeby jej zvětší
 * 
 * @param stack Ukazatel na zásobník
 * @param data Data pro uložení
 * @return true Operace proběhla v pořádku
 * @return false Došlo k chybě
 */
bool stackPush(DynamicStackPtr stack,Symbol data);

/**
 * @brief Vrátí data z vrcholu zásobníku (nemaže zásobník)
 * 
 * @param stack Ukazatel na zásobník
 * @return StackElementPtr Data z vrcholu
 */
Symbol stackRead(DynamicStackPtr stack);

/**
 * @brief Odstraní vrchní prvek zásobníku, zajistí případné zmenšení zásobníku
 * 
 * @param stack Ukazatel na zásobník
 * @return true Operace proběhla v pořádku
 * @return false Došlo k chybě (např prázdný zásobník)
 */
bool stackPop(DynamicStackPtr stack);

/**
 * @brief Určuje zda je zásobník prázdný
 * 
 * @param stack Ukazatel na zásobník
 * @return true Je prázdný
 * @return false Není prázdný, nebo nedostal zásobník
 */
bool stackIsEmpty(DynamicStackPtr stack);

/**
 * @brief Zajišťuje dealokaci dat i metadat zásobníku
 * 
 * @param stack Ukazatel na zásobník
 */
void stackDestruct(DynamicStackPtr stack);

#endif

 /*** konec dynamic_stack.h ***/