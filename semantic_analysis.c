// TODO hlavička

#include "semantic_analysis.h"

#include <limits.h>

/* TODO
- Nějaká struktura pro ukládání zatím nedefinovaných funkcí (potřeba hlídat, že definice je v nějaké
NADŘAZENÉ tabulce (a ne v tabulce na stejné/nižší úrovni))
        - Asi to bude hashovací tabulka (která bude mimo zásobník)

- Funkce pro úklid mnou vytvořené paměti v případě chyby (pro přidání do memoryClean())
*/

/**
 * @brief Ukazatel na zásobník pro ukládání jednotlivých rámců
 *
 */
static DynamicHashTablePtrStackPtr scopesStack = NULL;

/**
 * @brief Pomocná funkce pro ověření existence funkce main
 * 
 */
bool foundMainFunction = false;


/**
 * @brief Vytvoří zásobník
 *
 * @return true Úspěch
 * @return false Došlo k chybě
 */
bool initiateScopeStack();

/**
 * @brief Přidá na zásobník nový rámec
 *
 * @return true Úspěch
 * @return false Došlo k chybě
 */
bool createNewScope();

/**
 * @brief Odebere ze zásobníku rámec
 *
 * @return true Úspěch
 * @return false Došlo k chybě
 */
bool deleteScope();

/**
 * @brief Prohledá všechny rámce a vrací ukazatel na první nalezený symbol
 *
 * Začíná nejbližší rámcem (vrchol zásobníku)
 * @param key Klíč hledaného symbolu
 * @return SemanticSymbol* Ukazatel na nalezený symbol
 */
SemanticSymbol* findSymbol(char* key);

/**
 * @brief Prohledá aktuální rámec (vrchol zásobníku) a vrací ukazatel na nalezený symbol
 *
 * @param key Klíč hledaného symbolu
 * @return SemanticSymbol* Ukazatel na nalezený symbol
 */
SemanticSymbol* findSymbolCurrentScope(char* key);

/**
 * @brief Přidá symbol do globálního rámce (první vytvořený rámec)
 *
 * @param symbol Symbol pro přidání
 * @return true Úspěch
 * @return false Došlo k chybě
 */
bool insertSymbolGlobalScope(SemanticSymbol symbol);

/**
 * @brief Přidá symbol do aktuálního rámce
 *
 * @param symbol Symbol pro přidání
 * @return true Úspěch
 * @return false Došlo k chybě
 */
bool insertSymbolCurrentScope(SemanticSymbol symbol);

/**
 * @brief Převede informace z AST uzlu do symbolu pro uložení do tabulky
 *
 * @warning Pokud je cílový symbol funkce, tak ji přejmenuje v sourceNode i destSymbol na "nazevFunkce$pocetParametru"
 * @param sourceNode Zdrojový AST uzel
 * @param destSymbol Symbol pro upravení
 * @param symbolType Typ cílového symbolu
 * @return true Úspěch
 * @return false Došlo k chybě
 */
bool initNewSymbol(AstNode* sourceNode, SemanticSymbol* destSymbol, SemanticSymbolTypes symbolType);


/**
 * @brief Počet znaků potřebných pro uložení čísla jako řetězce
 * 
 * K počtu ještě přičte 1 pro uložení '\0' 
 * @param n Číslo
 * @return size_t Počet znaků
 */
size_t digitsCount(size_t n);

// TODO všude přidat kontrolu chyb u bool funkcí (existuje v GCC nějaký flag, co tě nenechá ignorovat výstup funkce?)

// idk_co findDeclarationInUndeclaredCalls(/*idk co*/);

/**
 * @brief Funkce pro rekurzivní procházení stromu
 *
 * Chování a návratový kód závisí na typu vstupního uzlu.  
 * @param currentNode Vstupní uzel
 * @param parentNodeType Uzel nadřazený vstupnímu uzlu
 * @return RecursiveSemanticAnalysisReturnValues Návratový kód
 */
RecursiveSemanticAnalysisReturnValues recursiveNodeWalker(AstNode*    currentNode,
                                                          AstNodeType parentNodeType);

bool startSematicValidation(AstNode* root) {
    if (root == NULL) {
        printError(INTERNAL_ERROR, "(%s) Nebyl predan AST!\n", __func__);
        freeSematicsMemory();
        return false;
    }

    initiateScopeStack();
    for (size_t i = 0; i < root->arity; i++) {
        if (recursiveNodeWalker(root->child[i], AST_PROGRAM) != OK) {
            freeSematicsMemory();
            return false;
        }
    }

    if(foundMainFunction == false) {
        printError(SEMANTICS_ERROR_OTHERS, "Chybi funkce 'main'!\n");
        return false;
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
            printLog("AST_PROGRAM\n");
            // Toto bych neměl dostat znovu (už jej zpracovalo @see startSematicValidation() )
            return ERROR;
            break;
        }
        case (AST_PROLOG): {
            printLog("AST_PROLOG\n");
            // TODO Huh co tady
            return OK;
            break;
        }
        case (AST_MAIN_CLASS): {
            printLog("AST_MAIN_CLASS\n");
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
            printLog("AST_MAIN_CLASS_BLOCK\n");
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
            printLog("AST_BLOCK\n");
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
            printLog("AST_STATEMENT\n");
            break;
        }
        case (AST_STATEMENT_LIST): {
            printLog("AST_STATEMENT_LIST\n");
            break;
        }
        case (AST_FUNC_DEC): {
            printLog("AST_FUNC_DEC\n");

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
            printLog("AST_PARAM\n");
            // TODO tady asi něco bude
            return OK;
            break;
        }
        case (AST_FUNC_CALL): {
            printLog("AST_FUNC_CALL\n");
            break;
        }
        case (AST_VAR): {
            printLog("AST_VAR\n");

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
            printLog("AST_ASSIGN\n");
            break;
        }
        case (AST_IF): {
            printLog("AST_IF\n");
            break;
        }
        case (AST_WHILE): {
            printLog("AST_WHILE\n");
            break;
        }
        case (AST_FOR): {
            printLog("AST_FOR\n");
            break;
        }
        case (AST_BREAK): {
            printLog("AST_BREAK\n");
            break;
        }
        case (AST_CONTINUE): {
            printLog("AST_CONTINUE\n");
            break;
        }
        case (AST_RETURN): {
            printLog("AST_RETURN\n");
            // TODO odstranění věcí za?
            return OK;
            break;
        }
        case (AST_IDENTIFIER): {
            printLog("AST_IDENTIFIER\n");
            break;
        }
        case (AST_GLOBAL_ID): {
            printLog("AST_GLOBAL_ID\n");
            break;
        }
        case (AST_INT_LIT): {
            printLog("AST_INT_LIT\n");
            break;
        }
        case (AST_FLOAT_LIT): {
            printLog("AST_FLOAT_LIT\n");
            break;
        }
        case (AST_STRING_LIT): {
            printLog("AST_STRING_LIT\n");
            break;
        }
        case (AST_EMPTY): {
            printLog("AST_EMPTY\n");
            // TODO
            return OK;
            break;
        }
        case (AST_NULL): {
            printLog("AST_NULL\n");
            break;
        }
        case (AST_PLUS): {
            printLog("AST_PLUS\n");
            break;
        }
        case (AST_MINUS): {
            printLog("AST_MINUS\n");
            break;
        }
        case (AST_MUL): {
            printLog("AST_MUL\n");
            break;
        }
        case (AST_DIV): {
            printLog("AST_DIV\n");
            break;
        }
        case (AST_EQ): {
            printLog("AST_EQ\n");
            break;
        }
        case (AST_LT): {
            printLog("AST_LT\n");
            break;
        }
        case (AST_GT): {
            printLog("AST_GT\n");
            break;
        }
        case (AST_NOT): {
            printLog("AST_NOT\n");
            break;
        }
        case (AST_AND): {
            printLog("AST_AND\n");
            break;
        }
        case (AST_OR): {
            printLog("AST_OR\n");
            break;
        }
        case (AST_IS): {
            printLog("AST_IS\n");
            break;
        }
        case (AST_GTE): {
            printLog("AST_GTE\n");
            break;
        }
        case (AST_LTE): {
            printLog("AST_LTE\n");
            break;
        }
    }
    return OK;
}

void freeSematicsMemory() {
    if(scopesStack == NULL) {
        printLog("(%s) volano na prazdny scopeStack.\n", __func__);
        return;
    }

    while (stackHashTablePtrIsEmpty(scopesStack) != true) {
        hashTableDealloc(stackHashTablePtrRead(scopesStack));
        stackHashTablePtrPop(scopesStack);
    }

    stackHashTablePtrDestruct(scopesStack);
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
                strlen(sourceNode->child[0]->value.str) + 1 + digitsCount(sourceNode->child[1]->arity); // prostor pro separátor '$'

            char* newName = malloc(sizeof(char) * newNameLength);

            if (newName == NULL) {
                printError(INTERNAL_ERROR, "(%s) Selhala alokace pameti!", __func__);
                return false;
            }

            destSymbol->name = newName;

            snprintf(destSymbol->name, newNameLength ,"%s$%lu", sourceNode->child[0]->value.str,
                    sourceNode->child[1]->arity);
            destSymbol->paramCount = sourceNode->child[1]->arity;

            free(sourceNode->child[0]->value.str);
            sourceNode->child[0]->value.str = newName;
            break;
        }
        case (VARIABLE): {
            destSymbol->name = sourceNode->child[0]->value.str;
            destSymbol->dataType = NULL_VAL; // TODO tady to bude jinak pro 'var x' a 'var x = 5'
            break;
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

size_t digitsCount(size_t n) {
// TODO nějak chytřeji udělat...

    if(n < 10) {
        return 2; // číslice a \0
    }

    if(n < 100) {
        return 3; // číslice a \0
    }

        if(n < 1000) {
        return 4; // číslice a \0
    }

        if(n < 10000) {
        return 5; // číslice a \0
    }

    // TODO
    printWarning("NENI IMPLEMEnTOVANO %s", __func__);
    return 50;
}

// TODO patička