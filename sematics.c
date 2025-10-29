// TODO hlavička

#include "semantics.h"

/* TODO
- Nějaká struktura pro ukládání úrovní tabulek
- Nějaká struktura pro ukládání zatím nedefinovaných funkcí (potřeba hlídat, že definice je v nějaké NADŘAZENÉ tabulce (a ne v tabulce na stejné/nižší úrovni))

- Funkce pro posun stromem
- Funkce pro validaci různých častých struktur (vracející bool zda je vše ok)
- Funkce pro úklid mnou vytvořené paměti v případě chyby (pro přidání do memoryClean())
*/

static DynamicHashTablePtrStackPtr scopesStack;

bool initiateScopeStack();

bool createNewScope(/*TODO*/);
    // Přidá scope na stack

bool leaveScope(/*TODO*/);
    // Dropne jeden scope ze stack

//bude vracet něco jiného, asi ukazatel na hodnotu v tabulce checkPossibleScopes(/*TODO hodnota pro hash a hledání*/)
    // Začne na stackTop a pojede až k 0

//Možná hashtable s těmi mými hodnotami?

//idk_co findDeclarationInUndeclaredCalls(/*idk co*/);

// Nějaká ta funkce na zpracování jednotlivých uzlů (v případě potřeby bude volat sama sebe na určité svoje pod-uzly)
    // Měla by vracet nějakou užitečnou hodnotu... ale jakou bude hodně záležet na typu uzlu... (jak nadřazeného tak podřazeného)


/*
Návratové hodnoty funkce na post order průchod

*/

void startSematicValidation(AstNode *root) {
    /*
    S využitím pomocných funkcí prochází strom shora dolů a:
        Při každém začátku bloku vytvoří novou tabulku symbolů
            - Při hledání proměnných nebo funkcí hledá v tabulce odspoda nahoru (nejvíc lokální ->
    globální)
                - Pokud nenalezne volanou funkci nebo deklaraci dané proměnné uloží ji do struktury
    X
                - Pokud nalezne tak Ok
                - Při deklarování proměnné/funkce prohledává jen nejbližší úroveň tabulky (umožňuje
    překrytí globálních věcí lokálními)
                - pozor: var x = x + 6 je potřeba vyhodnocovat tak aby nejdřív hledalo x v "x+6" a
    pak až vzniklo var x (musí to být jiná proměnná než ta právě deklarovaná)
                    - To lze zajistit tak, že pokud najdu var x = tak to neuložím do tabulky dokud
    nezpracuji část za = (abych znal datový typ)
                    - U pouze var x \n do tabulky přidám s typem Null a hodnotou null
        - Při skončení bloku stoupám o úroveň výše (potřeba zajistit aby další klesnutí kleslo do
    jiné tabulky => každé klesnutí = vytvořím novou)
            - Je potřeba nějak ukládat jakou tabulku prohledávat a jaké jsou jí nadřazené... 
                - To udělám pomocí zásobníku ukazatelů na tabulky, čím vyšší tabulky tím víc lokální je
                    Při prohledávání jdu od shora dolů (což size ruší pravidla zásobníku, ale to nevadí)
                    Při opuštění bloku tabulku zahodím, hodnot z ní mě nezajímají
                    Pokud narazím na volání nedeklarované funkce tak si uložím název parametry a tak bokem a taky scope (např 3 na zásobníku) 
                    Pokud narazím na deklaraci funkce s tímto názvem a parametry tak ověřím, že je scope vyšší než scope daného volání (0,1,2)
                        - Pokud ano tak mohu volání prohlásit za OK\
                        - Pokud něco nesedí (scope/název/počet argumentů) tak jedu dál


    Kontroluji (by AI, radši ještě ověřit):



    Deklarace proměnných: Kontrola, zda lokální proměnná definovaná příkazem var idl není v
    aktuálním bloku definována vícekrát (jinak chyba 4). Po deklaraci je její hodnota nastavena na
    null.

    Přiřazení hodnoty: Levý operand přiřazovacího příkazu musí být lokální, globální proměnná nebo
    statický setter. Kontrola kompatibility přiřazované hodnoty (výrazu) s očekávaným typem proměnné
    alespoň pro literály, ostatní typové kontroly probíhají až za běhu. Pokud levý operand je
    statický setter, přiřazení se interpretuje jako volání funkce.

    Podmínky v příkazech if a while: Výrazy mohou být libovolného typu, pravdivost se určuje
    pravidly z textu (null je nepravdivé, jinak hodnota evaluačně pravdivá). Sémantika kontroly
    správnosti výrazů v podmínkách.

    Volání funkcí: Kontrola, zda počet parametrů funkce odpovídá definici (chyba 5). Parametry musí
    být správných typů minimálně u literálů. Volání statických getterů a setterů správně
    interpretovat.

    Typová kontrola ve výrazech:

        Minimálně kontrola typů u literálů (např. řetězec + číslo je chyba 6).

        Operátory +, -, *, /, relační operátory vyžadují operandy správných typů (typ Num nebo
    String dle operátoru).

        Není povoleno implicitní převádění mezi řetězcem a číslem.

        Iterační operátor * musí mít vpravo celé číslo.

    Vestavěné funkce: Kontrola správných typů parametrů u volání vestavěných funkcí, minimálně u
    literálů, jinak chyba 5, s dynamickou kontrolou i za běhu.

    Return příkaz: Vyhodnocení výrazu a přiřazení návratové hodnoty funkce, správná implementace
    toku návratu.


    */
}

void freeSematicsMemory() {
    // TODO
}

bool initiateScopeStack() {
    DynamicHashTablePtrStackPtr stack = stackHashTablePtrConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    if(stack == NULL) {
        return false;
    }

    scopesStack = stack;
    return true;
}

bool createNewScope() {
    HashTable* newScope = hashTableInit();
    if(newScope == NULL) {
        return false;
    }

    stackHashTablePtrPush(scopesStack, newScope);
    return true;
}

bool leaveScope() {
    HashTable* newScope = stackHashTablePtrRead(scopesStack);

    return false;

}





// TODO patička