/**
 * Fakulta informačních technologií VUT Brno
 *
 * Hlavička pro dynamický zásobník (prostor se zvětšuje i zmenšuje dle naplnění) s podporou určení
 * datového typu před překladem
 * @file dynamic_stack.h
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#ifndef DYNAMIC_STACK
#define DYNAMIC_STACK 1

#include <stdbool.h>
#include <stdlib.h>

#include "support_functions.h"
#include "symbol.h"

/**
 * @brief Základní velikost zásobníku (možné použít při inicializaci)
 *
 */
#define DEFAULT_INIT_ELEMENT_COUNT 20

/**
 * @brief Makro pro deklaraci zásobníku pro určitý datový typ
 *
 * Vytvoří deklarace struktury a funkcí pro práci se zásobníkem s formátem stackNAMEOperace
 * (stackIntPush)
 * @param TYPE Datový typ data v poli zásobníku
 * @param NAME Řetězec pro odlišení funkcí prodaný typ zásobníku
 */
#define DECLARE_STACK(TYPE, NAME)                                                                         \
    /**                                                                                                   \
     * @brief Zásobník s automatickým zvětšením/zmenšením                                         \
     *                                                                                                    \
     */                                                                                                   \
    typedef struct {                                                                                      \
        /**                                                                                               \
         * @brief Uložení dat                                                                           \
         *                                                                                                \
         */                                                                                               \
        TYPE* array;                                                                                      \
        /**                                                                                               \
         * @brief Nejnižší prázdné místo zásobníku (hned nad vrcholem zásobníku)                \
         *                                                                                                \
         */                                                                                               \
        unsigned nextTopElement;                                                                          \
        /**                                                                                               \
         * @brief Alokovaná velikost zásobníku                                                         \
         *                                                                                                \
         */                                                                                               \
        unsigned maxSize;                                                                                 \
        /**                                                                                               \
         * @brief Při překročení hodnoty dojde k zdvojnásobení velikosti zásobníku                \
         *                                                                                                \
         * maxSize/2                                                                                      \
         */                                                                                               \
        unsigned nextUpsizeAt;                                                                            \
        /**                                                                                               \
         * @brief Při podkročení hodnoty dojde k zmenšení zásobníku o polovinu                     \
         *                                                                                                \
         * maxSize/4                                                                                      \
         */                                                                                               \
        unsigned nextDownsizeAt;                                                                          \
        /**                                                                                               \
         * @brief Značí zda během poslední operace došlo ke změně velikosti                        \
         *                                                                                                \
         */                                                                                               \
        bool resized;                                                                                     \
    } Dynamic##NAME##Stack;                                                                               \
                                                                                                          \
    /**                                                                                                   \
     * @brief Syntaktické pozlátko pro zjednodušení práce se zásobníkem \                          \
     *                                                                                                    \
     */                                                                                                   \
    typedef Dynamic##NAME##Stack* Dynamic##NAME##StackPtr;                                                \
                                                                                                          \
    /**                                                                                                   \
     * @brief Konstruktor zásobníku                                                                     \
     *                                                                                                    \
     * Alokuje strukturu zásobníku a pole pro data, nastaví počáteční hodnoty                      \
     * @param initStackSize Počáteční velikost zásobníku (>1)                                       \
     * @return Ukazatel na nový zásobník                                                               \
     */                                                                                                   \
    Dynamic##NAME##StackPtr stack##NAME##Construct(unsigned initStackSize);                               \
                                                                                                          \
    /**                                                                                                   \
     * @brief Přidá hodnotu na zásobník, v případě potřeby jej zvětší                          \
     *                                                                                                    \
     * @param stack Ukazatel na zásobník                                                                \
     * @param data Data pro uložení                                                                     \
     * @return true Operace proběhla v pořádku                                                         \
     * @return false Došlo k chybě (selhání při změně velikosti není považováno za chybu)       \
     */                                                                                                   \
    bool stack##NAME##Push(Dynamic##NAME##StackPtr stack, TYPE data);                                     \
                                                                                                          \
    /**                                                                                                   \
     * @brief Vrátí data z vrcholu zásobníku (nemaže zásobník)                                     \
     *                                                                                                    \
     * @warning Očekává, že zásobník existuje a není prázdný. Jinak nedefinované chování!     \
     * @param stack Ukazatel na zásobník                                                                \
     * @return Data z vrcholu                                                                             \
     */                                                                                                   \
    TYPE stack##NAME##Read(Dynamic##NAME##StackPtr stack);                                                \
                                                                                                          \
    /**                                                                                                   \
     * @brief Odstraní vrchní prvek zásobníku, zajistí případné zmenšení zásobníku            \
     *                                                                                                    \
     * @param stack Ukazatel na zásobník                                                                \
     * @return true Operace proběhla v pořádku                                                         \
     * @return false Došlo k chybě (např prázdný zásobník) (selhání při změně velikosti není \
     * považováno za chybu)                                                                             \
     */                                                                                                   \
    bool stack##NAME##Pop(Dynamic##NAME##StackPtr stack);                                                 \
                                                                                                          \
    /**                                                                                                   \
     * @brief Určuje zda je zásobník prázdný                                                         \
     *                                                                                                    \
     * @param stack Ukazatel na zásobník                                                                \
     * @return true Je prázdný                                                                          \
     * @return false Není prázdný, nebo nedostal zásobník                                            \
     */                                                                                                   \
    bool stack##NAME##IsEmpty(Dynamic##NAME##StackPtr stack);                                             \
                                                                                                          \
    /**                                                                                                   \
     * @brief Zajišťuje dealokaci dat i metadat zásobníku                                             \
     *                                                                                                    \
     * @warning Pokud jsou položky zásobníku ukazatele, je potřeba dealokaci cílů ukazatelů        \
     * zajistit zvlášť. Zásobník tuto variantu nekontroluje!                                         \
     * @param stack Ukazatel na zásobník                                                                \
     */                                                                                                   \
    void stack##NAME##Destruct(Dynamic##NAME##StackPtr stack);

//  DECLARE_STACK(char*, String) // Slouží pro testování
DECLARE_STACK(Symbol, Symbol)
DECLARE_STACK(AstNode*, AstNode)
DECLARE_STACK(Token, Precedence)
DECLARE_STACK(char *, labelStack)
DECLARE_STACK(AstNode*, EmitOp);
#endif
/*** konec dynamic_stack.h ***/
