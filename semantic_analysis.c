// TODO hlavička

#include "semantic_analysis.h"

#include <limits.h>

/* TODO
- Nějaká struktura pro ukládání úrovní tabulek
- Nějaká struktura pro ukládání zatím nedefinovaných funkcí (potřeba hlídat, že definice je v nějaké
NADŘAZENÉ tabulce (a ne v tabulce na stejné/nižší úrovni))
        - Asi to bude hashovací tabulka (která bude mimo zásobník)

- Funkce pro posun stromem
- Funkce pro validaci různých častých struktur (vracející bool zda je vše ok)
- Funkce pro úklid mnou vytvořené paměti v případě chyby (pro přidání do memoryClean())
*/

/*
Rovnou upraví jméno funkce aby odpovídalo počtu parametrů (v tom AST)
*/

/**
 * @brief
 *
 */
static DynamicHashTablePtrStackPtr scopesStack = NULL;

/**
 * @brief 
 * 
 */
bool foundMainFunction = false;


/**
 * @brief
 *
 * @return true
 * @return false
 */
bool initiateScopeStack();

/**
 * @brief Create a New Scope object
 *
 * @return true
 * @return false
 */
bool createNewScope();

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool deleteScope();

/**
 * @brief
 *
 * @param key
 * @return SemanticSymbol*
 */
SemanticSymbol* findSymbol(char* key);

/**
 * @brief
 *
 * @param key
 * @return SemanticSymbol*
 */
SemanticSymbol* findSymbolCurrentScope(char* key);

/**
 * @brief
 *
 * @param symbol
 * @return true
 * @return false
 */
bool insertSymbolGlobalScope(SemanticSymbol symbol);

/**
 * @brief
 *
 * @param symbol
 * @return true
 * @return false
 */
bool insertSymbolCurrentScope(SemanticSymbol symbol);

/**
 * @brief Create a New Symbol object
 *
 * @param sourceNode
 * @param destSymbol
 * @return true
 * @return false
 */
bool initNewSymbol(AstNode* sourceNode, SemanticSymbol* destSymbol, SemanticSymbolTypes symbolType);

// TODO upravit aby nebyl plagiat a špatné typy
int numPlaces(int n) {
    int r = 1;
    if (n < 0)
        n = (n == INT_MIN) ? INT_MAX : -n;
    while (n > 9) {
        n /= 10;
        r++;
    }
    return r;
}

// bude vracet něco jiného, asi ukazatel na hodnotu v tabulce checkPossibleScopes(/*TODO hodnota pro
// hash a hledání*/)
//  Začne na stackTop a pojede až k 0

// Možná hashtable s těmi mými hodnotami?

// idk_co findDeclarationInUndeclaredCalls(/*idk co*/);

// Nějaká ta funkce na zpracování jednotlivých uzlů (v případě potřeby bude volat sama sebe na
// určité svoje pod-uzly) Měla by vracet nějakou užitečnou hodnotu... ale jakou bude hodně záležet
// na typu uzlu... (jak nadřazeného tak podřazeného)

/*
Návratové hodnoty funkce na post order průchod

*/

RecursiveSemanticAnalysisReturnValues recursiveNodeWalker(AstNode*    currentNode,
                                                          AstNodeType parentNodeType);

bool startSematicValidation(AstNode* root) {
    if (root == NULL) {
        printError(INTERNAL_ERROR, "(%s) Nebyl predan AST!\n", __func__);
        freeSematicsMemory();
        return false;
    }

    for (size_t i = 0; i < root->arity; i++) {
        if (recursiveNodeWalker(root->child[i], AST_PROGRAM) != OK) {
            freeSematicsMemory();
            return false;
        }
    }
    return true;

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
                - To udělám pomocí zásobníku ukazatelů na tabulky, čím vyšší tabulky tím víc lokální
    je Při prohledávání jdu od shora dolů (což size ruší pravidla zásobníku, ale to nevadí) Při
    opuštění bloku tabulku zahodím, hodnot z ní mě nezajímají Pokud narazím na volání nedeklarované
    funkce tak si uložím název parametry a tak bokem a taky scope (např 3 na zásobníku) Pokud
    narazím na deklaraci funkce s tímto názvem a parametry tak ověřím, že je scope vyšší než scope
    daného volání (0,1,2)
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

RecursiveSemanticAnalysisReturnValues recursiveNodeWalker(AstNode*    currentNode,
                                                          AstNodeType parentNodeType) {
    if (currentNode == NULL) {
        return GOT_NODE_NULL;
    }

    switch (currentNode->kind) {
        case (AST_PROGRAM): {
            // Toto bych neměl dostat znovu (už jej zpracovalo @see startSematicValidation() )
            return ERROR;
            break;
        }
        case (AST_PROLOG): {
            // TODO Huh co tady
            printWarning("Neni implementovano1!\n");
            return ERROR;
            break;
        }
        case (AST_MAIN_CLASS): {
            if (currentNode->child[0]->value.has_str == false ||
                strncmp(currentNode->child[0]->value.str, "Program", 8) != 0) {
                printError(SEMANTICS_ERROR_OTHERS, "Neplatny nazev '%s', ocekavan 'Program'!\n",
                           currentNode->child[0]->value.str);
                return ERROR;
            }

            if (recursiveNodeWalker(currentNode->child[1], AST_MAIN_CLASS) != OK) {
                return ERROR;
            }
            break;
        }
        case (AST_MAIN_CLASS_BLOCK): {
            createNewScope();
            for (size_t i = 0; i < currentNode->arity; i++) {
                if (recursiveNodeWalker(currentNode->child[i], AST_MAIN_CLASS_BLOCK) != OK) {
                    return ERROR;
                }
            }
            deleteScope();
            break;
        }
        case (AST_BLOCK): {
            createNewScope();
            for (size_t i = 0; i < currentNode->arity; i++) {
                if (recursiveNodeWalker(currentNode->child[i], AST_BLOCK) != OK) {
                    return ERROR;
                }
            }
            deleteScope();
            break;
        }
        case (AST_STATEMENT): {
            break;
        }
        case (AST_STATEMENT_LIST): {
            break;
        }
        case (AST_FUNC_DEC): {
            // Vezmu jméno a počet parametrů a spojím na jmeno$pocetParam
            // Vyhledám klíč v databázi (řvu pokud existuje)
            // Přidám nový do scope

            if(strncmp(currentNode->child[0]->value.str, "main", 5) == 0) {
                // TODO asi chyba pro víc main
                foundMainFunction = true;
            }

            SemanticSymbol newSymbol;

            initNewSymbol(currentNode, &newSymbol, FUNCTION);

            SemanticSymbol* foundSymbol = findSymbol(currentNode->child[0]->value.str);
            if (foundSymbol != NULL) {
                // Možná i u funkce prohledat jen current scope?
                printError(SEMANTICS_ERROR_REDEF, "Funkce '%s' v danem bloku jiz existuje!", currentNode->child[0]->value.str);
                return ERROR;
            }

            insertSymbolCurrentScope(newSymbol);


            if(recursiveNodeWalker(currentNode->child[1], AST_FUNC_DEC ) != OK) {
                return ERROR;
            }
            if(recursiveNodeWalker(currentNode->child[2], AST_FUNC_DEC) != OK) {
                return ERROR;
            }
            break;
        }
        case (AST_PARAM): {
            // TODO tady asi něco bude
            return OK;
            break;
        }
        case (AST_FUNC_CALL): {
            break;
        }
        case (AST_VAR): {

            SemanticSymbol newSymbol;

            initNewSymbol(currentNode, &newSymbol, VARIABLE);

            SemanticSymbol* foundSymbol = findSymbolCurrentScope(currentNode->child[0]->value.str);
            if (foundSymbol != NULL) {
                printError(SEMANTICS_ERROR_REDEF, "Promena '%s' v danem bloku jiz existuje!", currentNode->child[0]->value.str);
                return ERROR;
            }

            insertSymbolCurrentScope(newSymbol);

            break;
        }
        case (AST_ASSIGN): {
            break;
        }
        case (AST_IF): {
            break;
        }
        case (AST_WHILE): {
            break;
        }
        case (AST_FOR): {
            break;
        }
        case (AST_BREAK): {
            break;
        }
        case (AST_CONTINUE): {
            break;
        }
        case (AST_RETURN): {
            // TODO odstranění věcí za?
            return OK;
            break;
        }
        case (AST_IDENTIFIER): {
            break;
        }
        case (AST_GLOBAL_ID): {
            break;
        }
        case (AST_INT_LIT): {
            break;
        }
        case (AST_FLOAT_LIT): {
            break;
        }
        case (AST_STRING_LIT): {
            break;
        }
        case (AST_EMPTY): {
            // TODO
            return OK;
            break;
        }
        case (AST_PLUS): {
            break;
        }
        case (AST_MINUS): {
            break;
        }
        case (AST_MUL): {
            break;
        }
        case (AST_DIV): {
            break;
        }
        case (AST_EQ): {
            break;
        }
        case (AST_LT): {
            break;
        }
        case (AST_GT): {
            break;
        }
        case (AST_NOT): {
            break;
        }
        case (AST_AND): {
            break;
        }
        case (AST_OR): {
            break;
        }
        case (AST_IS): {
            break;
        }
        case (AST_GTE): {
            break;
        }
        case (AST_LTE): {
            break;
        }
    }

    printLog("End of (%s)\n", __func__);
    return OK;
}

void freeSematicsMemory() {
    // TODO
}

bool initiateScopeStack() {
    DynamicHashTablePtrStackPtr stack = stackHashTablePtrConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    if (stack == NULL) {
        return false;
    }

    scopesStack = stack;
    return true;
}

bool createNewScope() {
    HashTable* newScope = hashTableInit();
    if (newScope == NULL) {
        return false;
    }

    hashTableInitItems(newScope);

    stackHashTablePtrPush(scopesStack, newScope);
    return true;
}

bool deleteScope() {
    HashTable* newScope = stackHashTablePtrRead(scopesStack);
    if (newScope == NULL) {
        return false;
    }

    stackHashTablePtrPop(scopesStack);

    return true;
}

SemanticSymbol* findSymbol(char* key) {
    unsigned toSearchScopeIndex = scopesStack->nextTopElement;
    int      index              = -1;
    while (toSearchScopeIndex != 0) {
        toSearchScopeIndex--;

        index = hashTableSearchItem(scopesStack->array[toSearchScopeIndex], key);
        if (index != -1) {
            break;
        }
    }

    if (index == -1) {
        return NULL;
    }

    return &(scopesStack->array[toSearchScopeIndex]->items[index].symbol);
}

SemanticSymbol* findSymbolCurrentScope(char* key) {
    if (scopesStack == NULL) {
        printWarning("(%s) Neexistuje zasobnik!\n", __func__);
        return NULL;
    }

    if (stackHashTablePtrIsEmpty(scopesStack) == true) {
        return NULL;
    }

    int index = hashTableSearchItem(stackHashTablePtrRead(scopesStack), key);

    if (index == -1) {
        return NULL;
    }

    return &(stackHashTablePtrRead(scopesStack)->items[index].symbol);
}

bool insertSymbolGlobalScope(SemanticSymbol symbol) {
    if (scopesStack->nextTopElement == 0) {
        printError(INTERNAL_ERROR, "Neni inicializovan scope!\n");  // TODO česky
        return false;
    }

    int index = hashTableInsertItem(scopesStack->array[0], symbol.name, symbol);

    if (index == -1) {
        return false;
    }

    return true;
}

bool insertSymbolCurrentScope(SemanticSymbol symbol) {
    if (scopesStack == NULL) {
        printWarning("(%s) Neexistuje zasobnik!\n", __func__);
        return NULL;
    }

    if (stackHashTablePtrIsEmpty(scopesStack) == true) {
        printError(INTERNAL_ERROR, "No scope initiated!");  // TODO česky
        return NULL;
    }

    HashTable* currentScope = stackHashTablePtrRead(scopesStack);

    int index = hashTableInsertItem(currentScope, symbol.name, symbol);

    if (index == -1) {
        return false;
    }

    return true;
}

bool initNewSymbol(AstNode* sourceNode, SemanticSymbol* destSymbol,
                   SemanticSymbolTypes symbolType) {
    if (sourceNode == NULL || destSymbol == NULL) {
        printWarning("(%s) got NULL instead of values!\n", __func__);
        return false;
    }

    destSymbol->symbolType = symbolType;
    destSymbol->scope = scopesStack->nextTopElement - 1; // v global přepíšu
    switch (symbolType) {
        case (FUNCTION): {
            unsigned newNameLength =
                strlen(sourceNode->value.str) + numPlaces(sourceNode->child[1]->arity)+1;

            char* newName = malloc(sizeof(char) * newNameLength);

            if (newName == NULL) {
                printError(INTERNAL_ERROR, "(%s) Selhala alokace pameti!", __func__);
                return false;
            }

            destSymbol->name = newName;

            sprintf(destSymbol->name, "%s$%lu", sourceNode->child[0]->value.str,
                    sourceNode->child[1]->arity);
            destSymbol->paramCount = sourceNode->child[1]->arity;

            free(sourceNode->child[0]->value.str);
            sourceNode->child[0]->value.str = newName;
        }
        case (VARIABLE): {
            destSymbol->name = sourceNode->child[0]->value.str;
            destSymbol->dataType = NULL_VAL; // TODO tady to bude jinak pro 'var x' a 'var x = 5'
        }
        case (GLOBAL): {
        }
        case (SETTER): {
        }
        case (GETTER): {
            printWarning("NOT IMPLEMENTED!!!!");
            return false;
        }
        default: {
            printError(INTERNAL_ERROR, "(%s) Neplatny typ symbolu '%u`", __func__, symbolType);
            return false;
        }
    }

    return true;
}

// TODO patička