// TODO hlavička

#include "semantic_analysis.h"

#include <limits.h>
#include <string.h>

#include "dynamic_stack.h"
#include "support_functions.h"
#include "symtable.h"

// TODO promyslet zda má význam aby většina funkcí něco vracela (bool), spousta z nich může rovnou
// ukončit vykonávání programu, pokud funkce co je používají nevytváří paměť
// přidat to jako @attention (viz funkce pro práci s tabulkou volání)
// Pokud tam bool dává smysl, tak povinně kontrolovat!

// TODO ověřit, že nedochází k úniku paměti při ukončení neplánovaně
// TODO všude kde vrací error popravit program (pokud nehrozí ztráta paměti!) (Ohlídat aby to nikdy
// nemohlo nastat) Práce s dynamickou pamětí probíhá jen při initSymbol a je nebezpečná až po
// insertToScope Minimalizovat počet řádků mezitím
// Poté odstranit zbytečné kontrolování ERROR když se tam stejně program nedostane

/**
 * @brief Ukazatel na zásobník pro ukládání jednotlivých rámců
 *
 */
static DynamicHashTablePtrStackPtr scopesStack = NULL;

static HashTable* undeclaredFunctionCalls = NULL;

/**
 * @brief Pomocná funkce pro ověření existence funkce main
 *
 */
static bool foundMainFunction = false;

/**
 * @brief Vytvoří zásobník
 *
 * @return true Úspěch
 * @return false Došlo k chybě
 */
bool createScopeStack();

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
 * @brief Vytvoří tabulku symbolů pro uložení volání na nedeklarované funkce
 *
 * @return true
 * @return false
 */
bool createUndeclaredFunctionCallsTable();

/**
 * @brief Odstraní z tabulky volání na nedeklarované funkce všechna volání funkce odpovídající
 * functionSymbolName
 *
 * @param functionSymbolName Název funkce jejíž volání má být odstraněno ( @note zpracovaný pomocí
 * @see initNewSymbol() )
 */
void removeUndeclaredFunctionCall(char* functionSymbolName);

/**
 * @brief Přidá
 *
 * @param functionSymbolName Název funkce jejíž volání má být odstraněno ( @note zpracovaný pomocí
 * @see initNewSymbol() )
 */
void insertUndeclaredFunctionCall(char* functionSymbolName, SemanticSymbol symbol);

/**
 * @brief Převede informace z AST uzlu do symbolu pro uložení do tabulky
 *
 * @note Pokud je cílový symbol funkce, tak ji přejmenuje v sourceNode i destSymbol na
 * "nazevFunkce$pocetParametru"
 * @param sourceNode Zdrojový AST uzel
 * @param destSymbol Symbol pro upravení
 * @param symbolType Typ cílového symbolu
 * @param symbolDataType Datový typ hodnoty v symbolu
 * @param functionReturnValue Datový typ návratové hodnoty funkce
 * @return true Úspěch
 * @return false Došlo k chybě
 */
bool initNewSymbol(AstNode* sourceNode, SemanticSymbol* destSymbol, SemanticSymbolTypes symbolType,
                   SemanticSymbolDataType symbolDataType,
                   SemanticSymbolDataType functionReturnValue);

/**
 * @brief Zpracovává sémantickou kontrolu volání vestavěných funkcí
 *
 * @param functionCallNode Uzel typu IFJ_CALL
 * @return RecursiveSemanticAnalysisReturnValues Návratová hodnota volání funkce
 */
RecursiveSemanticAnalysisReturnValues handleBuildinFunctions(AstNode* functionCallNode);

/**
 * @brief Počet znaků potřebných pro uložení čísla jako řetězce
 *
 * K počtu ještě přičte 1 pro uložení '\0'
 * @param n Číslo
 * @return size_t Počet znaků
 */
size_t digitsCount(size_t n);

/**
 * @brief Převádí návratovou hodnotu rekurzivního procházení stromu na hodnotu datového typu
 *
 * @param returnedValue Návratová hodnota rekurzivního průchodu
 * @return SemanticSymbolDataType odpovídající hodnota datového typu
 */
SemanticSymbolDataType convertSemanticReturnValueToSemanticDataType(
    RecursiveSemanticAnalysisReturnValues returnedValue);

/**
 * @brief Převádí hodnotu datového typu na návratovou hodnotu rekurzivního procházení stromu
 *
 * @param returnedValue Hodnota datového typu
 * @return RecursiveSemanticAnalysisReturnValues Odpovídající návratová hodnota
 */
RecursiveSemanticAnalysisReturnValues convertSemanticDataTypeToSemanticReturnValue(
    SemanticSymbolDataType dataType);

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
        memorySafeExit(INTERNAL_ERROR);
        return false;
    }

    createScopeStack();
    createUndeclaredFunctionCallsTable();

    for (size_t i = 0; i < root->arity; i++) {
        if (recursiveNodeWalker(root->child[i], AST_PROGRAM) != OK) {
            freeSematicsMemory();
            return false;
        }
    }

    if (foundMainFunction == false) {
        printError(SEMANTICS_ERROR_OTHERS, "Chybi funkce 'main()'!\n");
        memorySafeExit(SEMANTICS_ERROR_OTHERS);
        return false;
    }

    if (undeclaredFunctionCalls != NULL) {
        if (hashTableIsEmpty(undeclaredFunctionCalls) == false) {
            printError(SEMANTICS_ERROR_UNDEF, "Volani nedeklarovane funkce!\n");
            memorySafeExit(SEMANTICS_ERROR_OTHERS);
            return false;
        }
    }

    freeSematicsMemory();

    return true;

    /*
    S využitím pomocných funkcí prochází strom shora dolů a:
        Při každém začátku bloku vytvoří novou tabulku symbolů
            - Při hledání proměnných nebo funkcí hledá v tabulce odspoda nahoru (nejvíc lokální
    -> globální)
                - Pokud nenalezne volanou funkci nebo deklaraci dané proměnné uloží ji do
    struktury
    X
                - Pokud nalezne tak Ok
                - Při deklarování proměnné/funkce prohledává jen nejbližší úroveň tabulky
    (umožňuje překrytí globálních věcí lokálními)
                - pozor: var x = x + 6 je potřeba vyhodnocovat tak aby nejdřív hledalo x v "x+6"
    a pak až vzniklo var x (musí to být jiná proměnná než ta právě deklarovaná)
                    - To lze zajistit tak, že pokud najdu var x = tak to neuložím do tabulky
    dokud nezpracuji část za = (abych znal datový typ)
                    - U pouze var x \n do tabulky přidám s typem Null a hodnotou null
        - Při skončení bloku stoupám o úroveň výše (potřeba zajistit aby další klesnutí kleslo
    do jiné tabulky => každé klesnutí = vytvořím novou)
            - Je potřeba nějak ukládat jakou tabulku prohledávat a jaké jsou jí nadřazené...
                - To udělám pomocí zásobníku ukazatelů na tabulky, čím vyšší tabulky tím víc
    lokální je Při prohledávání jdu od shora dolů (což size ruší pravidla zásobníku, ale to
    nevadí) Při opuštění bloku tabulku zahodím, hodnot z ní mě nezajímají Pokud narazím na
    volání nedeklarované funkce tak si uložím název parametry a tak bokem a taky scope (např 3
    na zásobníku) Pokud narazím na deklaraci funkce s tímto názvem a parametry tak ověřím, že je
    scope vyšší než scope daného volání (0,1,2)
                        - Pokud ano tak mohu volání prohlásit za OK\
                        - Pokud něco nesedí (scope/název/počet argumentů) tak jedu dál


    Kontroluji (by AI, radši ještě ověřit):



    Deklarace proměnných: Kontrola, zda lokální proměnná definovaná příkazem var idl není v
    aktuálním bloku definována vícekrát (jinak chyba 4). Po deklaraci je její hodnota nastavena
    na null.

    Přiřazení hodnoty: Levý operand přiřazovacího příkazu musí být lokální, globální proměnná
    nebo statický setter. Kontrola kompatibility přiřazované hodnoty (výrazu) s očekávaným typem
    proměnné alespoň pro literály, ostatní typové kontroly probíhají až za běhu. Pokud levý
    operand je statický setter, přiřazení se interpretuje jako volání funkce.

    Podmínky v příkazech if a while: Výrazy mohou být libovolného typu, pravdivost se určuje
    pravidly z textu (null je nepravdivé, jinak hodnota evaluačně pravdivá). Sémantika kontroly
    správnosti výrazů v podmínkách.

    Volání funkcí: Kontrola, zda počet parametrů funkce odpovídá definici (chyba 5). Parametry
    musí být správných typů minimálně u literálů. Volání statických getterů a setterů správně
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
    if (scopesStack != NULL) {
        while (stackHashTablePtrIsEmpty(scopesStack) != true) {
            hashTableDealloc(stackHashTablePtrRead(scopesStack));
            stackHashTablePtrPop(scopesStack);
        }
        stackHashTablePtrDestruct(scopesStack);
        scopesStack = NULL;
    }

    if (undeclaredFunctionCalls != NULL) {
        hashTableDealloc(undeclaredFunctionCalls);
        undeclaredFunctionCalls = NULL;
    }
}

RecursiveSemanticAnalysisReturnValues recursiveNodeWalker(AstNode*    currentNode,
                                                          AstNodeType parentNodeType) {
    if (currentNode == NULL) {
        return GOT_NODE_NULL;
    }

    switch (currentNode->kind) {
        case (AST_PROGRAM): {
            printLog("AST_PROGRAM\n");
            // Toto bych neměl dostat znovu (už jej zpracovalo startSematicValidation() )
            printError(INTERNAL_ERROR, "Vice AST_PROGRAM neni validni!\n");
            memorySafeExit(INTERNAL_ERROR);
            return ERROR;
            break;
        }
        case (AST_PROLOG): {
            printLog("AST_PROLOG\n");
            return OK;
            break;
        }
        case (AST_MAIN_CLASS): {
            printLog("AST_MAIN_CLASS\n");
            if (currentNode->child[0]->value.has_str == false ||
                strncmp(currentNode->child[0]->value.str, "Program", 8) != 0) {
                printError(SEMANTICS_ERROR_OTHERS, "Neplatny nazev '%s', ocekavan 'Program'!\n",
                           currentNode->child[0]->value.str);
                memorySafeExit(SEMANTICS_ERROR_OTHERS);
                return ERROR;
            }

            if (recursiveNodeWalker(currentNode->child[1], AST_MAIN_CLASS) != OK) {
                return ERROR;
            }

            return OK;
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

            return OK;
            break;
        }
        case (AST_FUNC_DEC): {
            printLog("AST_FUNC_DEC\n");

            SemanticSymbol newSymbol;

            SemanticSymbolDataType functionReturnValue = NULL_VAL;

            if (currentNode->child[2]->child[0]->arity != 0) {
                RecursiveSemanticAnalysisReturnValues childValueParam =
                    recursiveNodeWalker(currentNode->child[1], AST_FUNC_DEC);
                RecursiveSemanticAnalysisReturnValues childValueBlock =
                    recursiveNodeWalker(currentNode->child[2], AST_FUNC_DEC);

                if (childValueParam == ERROR || childValueBlock == ERROR) {
                    return ERROR;
                }

                if (childValueBlock != OK && childValueBlock != GOT_NODE_NULL) {
                    functionReturnValue =
                        convertSemanticReturnValueToSemanticDataType(childValueBlock);
                }
            }

            initNewSymbol(currentNode, &newSymbol, FUNCTION, NULL_VAL, functionReturnValue);

            SemanticSymbol* foundSymbol = findSymbol(newSymbol.name);
            if (foundSymbol != NULL) {
                free(newSymbol.name);
                printError(SEMANTICS_ERROR_REDEF, "Funkce '%s' jiz existuje!", newSymbol.name);
                memorySafeExit(SEMANTICS_ERROR_REDEF);
                return ERROR;
            }

            if (strncmp(newSymbol.name, "main$0", 7) == 0) {
                if (foundMainFunction == true) {
                    printError(SEMANTICS_ERROR_REDEF,
                               "V kodu muze byt pouze jedna funkce pojmenovana 'main()`!\n");
                    memorySafeExit(SEMANTICS_ERROR_REDEF);
                }

                foundMainFunction = true;
            }

            insertSymbolGlobalScope(newSymbol);

            removeUndeclaredFunctionCall(newSymbol.name);

            return OK;
            break;
        }
        case (AST_PARAM): {
            printLog("AST_PARAM\n");

            createNewScope();
            for (size_t i = 0; i < currentNode->arity; i++) {
                if (recursiveNodeWalker(currentNode->child[i], AST_PARAM) == ERROR) {
                    return ERROR;
                }
            }

            return OK;
            break;
        }
        case (AST_BLOCK): {
            printLog("AST_BLOCK\n");
            if (parentNodeType != AST_FUNC_DEC) {
                createNewScope();
            }
            for (size_t i = 0; i < currentNode->arity; i++) {
                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[i], AST_BLOCK);
                if (childValue == ERROR) {
                    return ERROR;
                } else if (childValue != OK) {
                    return childValue;
                }
            }

            deleteScope();

            return OK;
            break;
        }
        case (AST_STATEMENT): {
            printLog("AST_STATEMENT\n");
            break;
        }
        case (AST_STATEMENT_LIST): {
            printLog("AST_STATEMENT_LIST\n");
            for (size_t i = 0; i < currentNode->arity; i++) {
                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[i], AST_STATEMENT_LIST);
                if (childValue == ERROR) {
                    return ERROR;
                }
            }

            return OK;
            break;
        }
        case (AST_FUNC_CALL): {
            printLog("AST_FUNC_CALL\n");

            SemanticSymbol         newSymbol;
            SemanticSymbolDataType functionReturnValue = UNKNOWN_VAL;

            initNewSymbol(currentNode, &newSymbol, FUNCTION, NULL_VAL, functionReturnValue);

            SemanticSymbol* foundSymbol = findSymbol(newSymbol.name);
            if (foundSymbol == NULL) {
                insertUndeclaredFunctionCall(newSymbol.name, newSymbol);
            } else {
                functionReturnValue = foundSymbol->returnValue;
            }

            if (parentNodeType == AST_ASSIGN) {
                return convertSemanticDataTypeToSemanticReturnValue(functionReturnValue);
            }
            return OK;

            break;
        }
        case (AST_VAR): {
            printLog("AST_VAR\n");

            SemanticSymbol newSymbol;

            SemanticSymbolDataType varDataType = NULL_VAL;

            if (currentNode->child[1]->kind != AST_EMPTY) {
                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[1], AST_VAR);

                if (childValue == ERROR) {
                    return ERROR;
                }

                varDataType = convertSemanticReturnValueToSemanticDataType(childValue);
            }

            initNewSymbol(currentNode, &newSymbol, VARIABLE, varDataType, NULL_VAL);

            SemanticSymbol* foundSymbol = findSymbolCurrentScope(newSymbol.name);
            if (foundSymbol != NULL) {
                printError(SEMANTICS_ERROR_REDEF, "Promena '%s' v danem ramci jiz existuje!\n",
                           newSymbol.name);
                memorySafeExit(SEMANTICS_ERROR_REDEF);
                return ERROR;
            }

            insertSymbolCurrentScope(newSymbol);
            return OK;
            break;
        }
        case (AST_ASSIGN): {
            printLog("AST_ASSIGN\n");

            if (parentNodeType == AST_VAR || parentNodeType == AST_GLOBAL_ID) {
                return recursiveNodeWalker(currentNode->child[0], AST_ASSIGN);
            }

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_ASSIGN);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_ASSIGN);

            if (leftChild != IDENTIFIER) {
                printError(SEMANTICS_ERROR_OTHERS,
                           "(=) Levy operand prirazeni musi byt promenna!\n");
                memorySafeExit(SEMANTICS_ERROR_OTHERS);
            }

            SemanticSymbol* leftSymbol = findSymbol(currentNode->child[0]->value.str);
            if (leftSymbol == NULL) {
                printError(SEMANTICS_ERROR_UNDEF, "(=) Prirazeni do nedefinovane promenne!\n");
                memorySafeExit(SEMANTICS_ERROR_UNDEF);
            }

            if (rightChild == IDENTIFIER) {
                SemanticSymbol* rightSymbol = findSymbol(currentNode->child[1]->value.str);
                if (rightSymbol == NULL) {
                    printError(SEMANTICS_ERROR_UNDEF, "(=) Cteni nedefinovane promenne!\n");
                    memorySafeExit(SEMANTICS_ERROR_UNDEF);
                }

                leftSymbol->dataType = rightSymbol->dataType;
                return OK;
            }

            leftSymbol->dataType = convertSemanticReturnValueToSemanticDataType(rightChild);
            return OK;

            break;
        }
        case (AST_IF_CHAIN): {
            printLog("AST_IF_CHAIN\n");
            // TODO ast if chain
            break;
        }
        case (AST_IF): {
            printLog("AST_IF\n");
            // TODO AST_IF
            break;
        }
        case (AST_ELSE): {
            printLog("AST_ELSE\n");
            // TODO ast_else
            break;
        }
        case (AST_WHILE): {
            printLog("AST_WHILE\n");
            // TODO AST_WHILE
            break;
        }
        case (AST_FOR): {
            printLog("AST_FOR\n");
            // TODO FOR
            break;
        }
        case (AST_IDENTIFIER): {
            printLog("AST_IDENTIFIER\n");

            if (parentNodeType == AST_PARAM) {
                SemanticSymbol newSymbol;

                initNewSymbol(currentNode, &newSymbol, FUNCTION_PARAMETER, UNKNOWN_VAL, NULL_VAL);

                SemanticSymbol* foundSymbol = findSymbolCurrentScope(newSymbol.name);

                if (foundSymbol != NULL) {
                    printError(SEMANTICS_ERROR_REDEF, "Promena '%s' v danem ramci jiz existuje!\n",
                               newSymbol.name);
                    memorySafeExit(SEMANTICS_ERROR_REDEF);
                    return ERROR;
                }

                insertSymbolCurrentScope(newSymbol);
                return OK;
            } else {
                SemanticSymbol* foundSymbol = findSymbol(currentNode->value.str);

                if (foundSymbol == NULL) {
                    printError(SEMANTICS_ERROR_UNDEF, "Promena '%s' neexistuje!\n",
                               currentNode->value.str);
                    memorySafeExit(SEMANTICS_ERROR_UNDEF);
                }

                if (parentNodeType == AST_ASSIGN) {
                    return IDENTIFIER;
                }

                return convertSemanticDataTypeToSemanticReturnValue(foundSymbol->dataType);
            }
            break;
        }
        case (AST_GLOBAL_ID): {
            printLog("AST_GLOBAL_ID\n");

            if (parentNodeType == AST_STATEMENT_LIST) {
                SemanticSymbol newSymbol;

                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[1], AST_GLOBAL_ID);

                if (childValue == ERROR) {
                    return ERROR;
                }

                SemanticSymbolDataType varDataType =
                    convertSemanticReturnValueToSemanticDataType(childValue);

                initNewSymbol(currentNode, &newSymbol, GLOBAL, varDataType, NULL_VAL);

                SemanticSymbol* foundSymbol = findSymbol(newSymbol.name);
                if (foundSymbol != NULL) {
                    // Jedná se o přiřazení nového hodnoty již existující globální proměnné
                    foundSymbol->dataType = newSymbol.dataType;
                    return OK;
                }

                insertSymbolGlobalScope(newSymbol);
                return OK;
            } else {
                SemanticSymbol* foundSymbol = findSymbol(currentNode->value.str);

                if (foundSymbol == NULL) {
                    // Globální proměnné lze číst i bez definování, vrací hodnotu null
                    return NULL_DATA;
                }

                if (parentNodeType == AST_ASSIGN) {
                    return IDENTIFIER;
                }
                return convertSemanticDataTypeToSemanticReturnValue(foundSymbol->dataType);
            }

            break;
        }
        case (AST_INT_LIT): {
            printLog("AST_INT_LIT\n");
            return INT_DATA;
            break;
        }
        case (AST_FLOAT_LIT): {
            printLog("AST_FLOAT_LIT\n");
            return FLOAT_DATA;
            break;
        }
        case (AST_BOOL_LIT): {
            printLog("AST_BOOL_LIT\n");
            return BOOLEAN_DATA;
            break;
        }
        case (AST_STRING_LIT): {
            printLog("AST_STRING_LIT\n");
            return STRING_DATA;
            break;
        }
        case (AST_KEYWORD_NUM): {
            printLog("AST_KEYWORD_NUM\n");
            return DATATYPE_KEYWORD;
            break;
        }
        case (AST_KEYWORD_STRING): {
            printLog("AST_KEYWORD_STRING\n");
            return DATATYPE_KEYWORD;
            break;
        }
        case (AST_PLUS): {
            printLog("AST_PLUS\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_PLUS);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_PLUS);

            if (leftChild == NUMBER_DATA && rightChild == NUMBER_DATA) {
                return NUMBER_DATA;
            }

            if (leftChild == INT_DATA && rightChild == INT_DATA) {
                return INT_DATA;
            }

            if (leftChild == FLOAT_DATA && rightChild == FLOAT_DATA) {
                return NUMBER_DATA;
            }

            if ((leftChild == INT_DATA && rightChild == FLOAT_DATA) ||
                (leftChild == FLOAT_DATA && rightChild == INT_DATA)) {
                return NUMBER_DATA;
            }

            if (leftChild == STRING_DATA && rightChild == STRING_DATA) {
                return STRING_DATA;
            }

            if (leftChild == UNKNOWN_DATA || rightChild == UNKNOWN_DATA) {
                if (leftChild == NUMBER_DATA || leftChild == INT_DATA || leftChild == FLOAT_DATA) {
                    return NUMBER_DATA;
                }

                if (leftChild == STRING_DATA) {
                    return STRING_DATA;
                }

                if (rightChild == NUMBER_DATA || rightChild == INT_DATA ||
                    rightChild == FLOAT_DATA) {
                    return NUMBER_DATA;
                }

                if (rightChild == STRING_DATA) {
                    return STRING_DATA;
                }

                if (leftChild == UNKNOWN_DATA && rightChild == UNKNOWN_DATA) {
                    return UNKNOWN_DATA;
                }
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                       "(+) Datove datove typy musi byt kompatibilni!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            return ERROR;

            break;
        }
        case (AST_MINUS): {
            printLog("AST_MINUS\n");

            if (currentNode->arity == 1) {
                RecursiveSemanticAnalysisReturnValues onlyChild =
                    recursiveNodeWalker(currentNode->child[0], AST_MINUS);

                if (onlyChild == NUMBER_DATA) {
                    return NUMBER_DATA;
                }

                if (onlyChild == INT_DATA) {
                    return INT_DATA;
                }

                if (onlyChild == FLOAT_DATA) {
                    return FLOAT_DATA;
                }

                if (onlyChild == UNKNOWN_DATA) {
                    return UNKNOWN_DATA;
                }
            } else {
                RecursiveSemanticAnalysisReturnValues leftChild =
                    recursiveNodeWalker(currentNode->child[0], AST_MINUS);
                RecursiveSemanticAnalysisReturnValues rightChild =
                    recursiveNodeWalker(currentNode->child[1], AST_MINUS);

                if (leftChild == NUMBER_DATA && rightChild == NUMBER_DATA) {
                    return NUMBER_DATA;
                }

                if (leftChild == INT_DATA && rightChild == INT_DATA) {
                    return INT_DATA;
                }

                if (leftChild == FLOAT_DATA && rightChild == FLOAT_DATA) {
                    return NUMBER_DATA;
                }

                if ((leftChild == INT_DATA && rightChild == FLOAT_DATA) ||
                    (leftChild == FLOAT_DATA && rightChild == INT_DATA)) {
                    return NUMBER_DATA;
                }

                if (leftChild == UNKNOWN_DATA || rightChild == UNKNOWN_DATA) {
                    if (leftChild == NUMBER_DATA || leftChild == INT_DATA ||
                        leftChild == FLOAT_DATA) {
                        return NUMBER_DATA;
                    }

                    if (rightChild == NUMBER_DATA || rightChild == INT_DATA ||
                        rightChild == FLOAT_DATA) {
                        return NUMBER_DATA;
                    }

                    if (leftChild == UNKNOWN_DATA && rightChild == UNKNOWN_DATA) {
                        return NUMBER_DATA;  // výsledek odčítání je vždy číslo
                    }
                }
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                       "(-) Datove datove typy musi byt kompatibilni!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            return ERROR;

            break;
        }
        case (AST_MUL): {
            printLog("AST_MUL\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_MUL);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_MUL);

            if (leftChild == NUMBER_DATA && rightChild == NUMBER_DATA) {
                return NUMBER_DATA;
            }

            if (leftChild == INT_DATA && rightChild == INT_DATA) {
                return INT_DATA;
            }

            if (leftChild == FLOAT_DATA && rightChild == FLOAT_DATA) {
                return NUMBER_DATA;
            }

            if ((leftChild == INT_DATA && rightChild == FLOAT_DATA) ||
                (leftChild == FLOAT_DATA && rightChild == INT_DATA)) {
                return NUMBER_DATA;
            }

            if (leftChild == STRING_DATA && rightChild == INT_DATA) {
                return STRING_DATA;
            }

            if (leftChild == UNKNOWN_DATA || rightChild == UNKNOWN_DATA) {
                if (leftChild == NUMBER_DATA || leftChild == INT_DATA || leftChild == FLOAT_DATA) {
                    return NUMBER_DATA;
                }

                if (leftChild == STRING_DATA) {
                    return STRING_DATA;
                }

                if (rightChild == NUMBER_DATA || rightChild == INT_DATA) {
                    return UNKNOWN_DATA;  // V tomto případě nelze rozhodnout zda se jedná o
                                          // operaci s řetězcem nebo celým číslem
                }

                if (rightChild == FLOAT_DATA) {
                    return NUMBER_DATA;  // Desetinné číslo v pravém operandu znamená, že
                                         // výsledek bude určitě desetinné číslo
                }

                if (leftChild == UNKNOWN_DATA && rightChild == UNKNOWN_DATA) {
                    return UNKNOWN_DATA;
                }
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                       "(*) Datove datove typy musi byt kompatibilni!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            return ERROR;

            break;
        }
        case (AST_DIV): {
            printLog("AST_DIV\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_DIV);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_DIV);

            if (leftChild == NUMBER_DATA && rightChild == NUMBER_DATA) {
                return NUMBER_DATA;
            }

            if (leftChild == INT_DATA && rightChild == INT_DATA) {
                return NUMBER_DATA;  // Nelze určit zda výsledek dělení dvou celých čísel bude
                                     // desetinný nebo celý
            }

            if (leftChild == FLOAT_DATA && rightChild == FLOAT_DATA) {
                return NUMBER_DATA;  // Nelze určit zda výsledek dělení dvou desetinných čísel
                                     // bude desetinný nebo celý
            }

            if ((leftChild == INT_DATA && rightChild == FLOAT_DATA) ||
                (leftChild == FLOAT_DATA && rightChild == INT_DATA)) {
                return NUMBER_DATA;  // Nelze určit zda výsledek dělení dvou celých čísel bude
                                     // desetinný nebo celý
            }

            if (leftChild == UNKNOWN_DATA || rightChild == UNKNOWN_DATA) {
                if (leftChild == NUMBER_DATA || leftChild == INT_DATA || leftChild == FLOAT_DATA) {
                    return NUMBER_DATA;
                }

                if (rightChild == NUMBER_DATA || rightChild == INT_DATA ||
                    rightChild == FLOAT_DATA) {
                    return NUMBER_DATA;
                }

                if (leftChild == UNKNOWN_DATA && rightChild == UNKNOWN_DATA) {
                    return NUMBER_DATA;  // Výsledek dělení je vždy číslo
                }
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                       "(/) Datove datove typy musi byt kompatibilni!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            return ERROR;

            break;
        }
        case (AST_EQ): {
            printLog("AST_EQ\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_EQ);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_EQ);

            if (leftChild == ERROR || rightChild == ERROR) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(==) Doslo k chybe!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                return ERROR;
            }
            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;

            break;
        }
        case (AST_NEQ): {
            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_EQ);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_EQ);

            if (leftChild == ERROR || rightChild == ERROR) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(!=) Doslo k chybe!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                return ERROR;
            }
            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;

            break;
        }
        case (AST_LT): {
            printLog("AST_LT\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_LT);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_LT);

            if (leftChild == ERROR || rightChild == ERROR) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(<) Doslo k chybe!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                return ERROR;
            }
            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;

            break;
        }
        case (AST_GT): {
            printLog("AST_GT\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_GT);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_GT);

            if (leftChild == ERROR || rightChild == ERROR) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(>) Doslo k chybe!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                return ERROR;
            }
            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;

            break;
        }
        case (AST_GTE): {
            printLog("AST_GTE\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_GTE);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_GTE);

            if (leftChild == ERROR || rightChild == ERROR) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(>=) Doslo k chybe!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                return ERROR;
            }
            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;

            break;
        }
        case (AST_LTE): {
            printLog("AST_LTE\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_LTE);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_LTE);

            if (leftChild == ERROR || rightChild == ERROR) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(<=) Doslo k chybe!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                return ERROR;
            }
            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;

            break;
        }
        case (AST_NOT): {
            printLog("AST_NOT\n");

            RecursiveSemanticAnalysisReturnValues onlyChild =
                recursiveNodeWalker(currentNode->child[0], AST_NOT);  // Negace má pouze jedno dítě

            if (onlyChild == BOOLEAN_DATA) {
                return BOOLEAN_DATA;
            }

            if (onlyChild == UNKNOWN_DATA) {
                return BOOLEAN_DATA;  // Výsledek logické negace bude vždy bool hodnota
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(!) Datovy typy musi byt kompatibilni!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            return ERROR;

            break;
        }
        case (AST_AND): {
            printLog("AST_AND\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_AND);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_AND);

            if (leftChild == BOOLEAN_DATA && rightChild == BOOLEAN_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == UNKNOWN_DATA || rightChild == UNKNOWN_DATA) {
                if (leftChild == BOOLEAN_DATA) {
                    return leftChild;
                }

                if (rightChild == BOOLEAN_DATA) {
                    return rightChild;
                }

                if (leftChild == UNKNOWN_DATA && rightChild == UNKNOWN_DATA) {
                    return BOOLEAN_DATA;  // Výsledek logického součtu bude vždy bool hodnota
                }
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                       "(&&) Datove datove typy musi byt kompatibilni!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            return ERROR;

            break;
        }
        case (AST_OR): {
            printLog("AST_OR\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_OR);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_OR);

            if (leftChild == BOOLEAN_DATA && rightChild == BOOLEAN_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == UNKNOWN_DATA || rightChild == UNKNOWN_DATA) {
                if (leftChild == BOOLEAN_DATA) {
                    return leftChild;
                }

                if (rightChild == BOOLEAN_DATA) {
                    return rightChild;
                }

                if (leftChild == UNKNOWN_DATA && rightChild == UNKNOWN_DATA) {
                    return BOOLEAN_DATA;  // Výsledek logického součinu bude vždy bool hodnota
                }
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                       "(||) Datove datove typy musi byt kompatibilni!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            return ERROR;

            break;
        }
        case (AST_IS): {
            printLog("AST_IS\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_IS);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_IS);

            if (leftChild == OK || leftChild == ERROR || leftChild == IDENTIFIER ||
                leftChild == GOT_NODE_NULL) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                           "(is) Leva strana operatoru musi mit jako vysledek datovy typ!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                return ERROR;
            }

            if (rightChild != DATATYPE_KEYWORD) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                           "(is) Prava strana operatoru musi byt nazev datoveho typu!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                return ERROR;
            }

            return BOOLEAN_DATA;

            break;
        }
        case (AST_DOUBLE_DOT): {
            printLog("AST_DOUBLE_DOT\n");
            // TODO
            break;
        }
        case (AST_TRIPLE_DOT): {
            printLog("AST_TRIPLE_DOT\n");
            // TODO
            break;
        }
        case (AST_RETURN): {
            printLog("AST_RETURN\n");

            RecursiveSemanticAnalysisReturnValues returnDataType = NULL_DATA;

            if (currentNode->child[0]->kind != AST_EMPTY) {
                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[0], AST_RETURN);
                if (childValue == ERROR) {
                    return ERROR;
                }

                if (returnDataType != OK || returnDataType != GOT_NODE_NULL)
                    returnDataType = childValue;
            }

            return returnDataType;
            break;
        }
        case (AST_IFJ_CALL): {
            printLog("AST_IFJ_CALL\n");
            return handleBuildinFunctions(currentNode);
            break;
        }
        case (AST_EMPTY): {
            printLog("AST_EMPTY\n");
            // Pomocný uzel u parametrů funkce
            return OK;
            break;
        }
        case (AST_NULL): {
            printLog("AST_NULL\n");
            // Značí prázdný jednořádkový blok (jeho návratovou hodnotu)
            return NULL_DATA;
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
        case (AST_LPAREN): {
            printLog("AST_LPAREN\n");
            // Tento AST uzel bych neměl dostat
            printError(INTERNAL_ERROR, "Vice AST_LPAREN neni validni uzel stromu!\n");
            memorySafeExit(INTERNAL_ERROR);
            return ERROR;
            break;
        }
        case (AST_RPAREN): {
            printLog("AST_RPAREN\n");
            // Tento AST uzel bych neměl dostat
            printError(INTERNAL_ERROR, "Vice AST_RPAREN neni validni uzel stromu!\n");
            memorySafeExit(INTERNAL_ERROR);
            return ERROR;
            break;
        }
    }

    printWarning("(%s) Neznamy stav: %i!\n", __func__, currentNode->kind);
    return ERROR;
}

bool createScopeStack() {
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
    if (scopesStack == NULL) {
        printError(INTERNAL_ERROR, "(%s) Zasobnik neexistuje!\n", __func__);
        return false;
    }
    if (stackHashTablePtrIsEmpty(scopesStack)) {
        printError(INTERNAL_ERROR, "(%s) Snaha odstranit prvek z prazdneho zasobniku!\n", __func__);
        return false;
    }

    HashTable* newScope = stackHashTablePtrRead(scopesStack);
    if (newScope == NULL) {
        printError(INTERNAL_ERROR, "(%s) Na zasobniku byl prazdny ukazatel!\n", __func__);
        return false;
    }

    hashTableDealloc(newScope);

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
        printLog("Prazdny zasobnik!\n");
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
        printError(INTERNAL_ERROR, "Neni inicializovan ramec!\n");
        memorySafeExit(INTERNAL_ERROR);
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
        printError(INTERNAL_ERROR, "Neni inicializovan ramec!\n");
        memorySafeExit(INTERNAL_ERROR);
        return NULL;
    }

    HashTable* currentScope = stackHashTablePtrRead(scopesStack);

    int index = hashTableInsertItem(currentScope, symbol.name, symbol);

    if (index == -1) {
        return false;
    }

    return true;
}

bool createUndeclaredFunctionCallsTable() {
    undeclaredFunctionCalls = hashTableInit();
    if (undeclaredFunctionCalls == NULL) {
        printError(INTERNAL_ERROR,
                   "Selhalo vytvoření tabulky pro volání nedeklarovaných funkcí!\n");
        return false;
    }
    return true;
}

void removeUndeclaredFunctionCall(char* functionSymbolName) {
    if (undeclaredFunctionCalls == NULL) {
        printError(INTERNAL_ERROR, "(%s) Tabulka pro volání nedeklarovaných funkcí neexistuje!\n",
                   __func__);
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    if (hashTableSearchItem(undeclaredFunctionCalls, functionSymbolName) == -1) {
        return;
    }

    hashTableDeleteItem(undeclaredFunctionCalls, functionSymbolName);
}

void insertUndeclaredFunctionCall(char* functionSymbolName, SemanticSymbol symbol) {
    if (undeclaredFunctionCalls == NULL) {
        printError(INTERNAL_ERROR, "(%s) Tabulka pro volání nedeklarovaných funkcí neexistuje!\n",
                   __func__);
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    if (hashTableInsertItem(undeclaredFunctionCalls, functionSymbolName, symbol) == -1) {
        printError(INTERNAL_ERROR, "(%s) Nepodařilo se vložit prvek do tabulky!\n", __func__);
        memorySafeExit(INTERNAL_ERROR);
    }
}

bool initNewSymbol(AstNode* sourceNode, SemanticSymbol* destSymbol, SemanticSymbolTypes symbolType,
                   SemanticSymbolDataType symbolDataType,
                   SemanticSymbolDataType functionReturnValue) {
    if (sourceNode == NULL || destSymbol == NULL) {
        printWarning("(%s) got NULL instead of values!\n", __func__);
        return false;
    }

    destSymbol->symbolType = symbolType;

    switch (symbolType) {
        case (FUNCTION): {
            unsigned newNameLength =
                strlen(sourceNode->child[0]->value.str) + 1 +
                digitsCount(sourceNode->child[1]->arity);  // prostor pro separátor '$'

            char* newName = malloc(sizeof(char) * newNameLength);

            size_t numberOfParams = sourceNode->child[1]->arity;
            numberOfParams--;  // Každá deklarace i volání funkce obsahuje v parametrech 1
                               // AST_EMTPY

            if (newName == NULL) {
                printError(INTERNAL_ERROR, "(%s) Selhala alokace pameti!", __func__);
                memorySafeExit(INTERNAL_ERROR);
                return false;
            }

            destSymbol->name        = newName;
            destSymbol->returnValue = functionReturnValue;

            snprintf(destSymbol->name, newNameLength, "%s$%lu", sourceNode->child[0]->value.str,
                     numberOfParams);
            destSymbol->paramCount = sourceNode->child[1]->arity;

            free(sourceNode->child[0]->value.str);
            sourceNode->child[0]->value.str = newName;
            break;
        }
        case (VARIABLE): {
            destSymbol->name = sourceNode->child[0]->value.str;

            destSymbol->dataType = symbolDataType;
            break;
        }
        case (FUNCTION_PARAMETER): {
            destSymbol->name = sourceNode->value.str;
            break;
        }
        case (GLOBAL): {
            destSymbol->name     = sourceNode->child[0]->value.str;
            destSymbol->dataType = symbolDataType;
            break;
        }
        case (SETTER): {
        }
        case (GETTER): {
            printWarning("NOT IMPLEMENTED!!!!");
            return false;
        }
        default: {
            printError(INTERNAL_ERROR, "(%s) Neplatny typ symbolu '%u`", __func__, symbolType);
            memorySafeExit(INTERNAL_ERROR);
            return false;
        }
    }

    return true;
}

RecursiveSemanticAnalysisReturnValues handleBuildinFunctions(AstNode* functionCallNode) {
    char* functionName = functionCallNode->child[0]->value.str;

    if (strncmp(functionName, "read_str", 9) == 0) {
        printLog("read_str\n");

        if (functionCallNode->child[1]->arity != 1) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.read_str) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        return STRING_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "read_num", 9) == 0) {
        printLog("read_num\n");

        if (functionCallNode->child[1]->arity != 1) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.read_num) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        return NUMBER_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }
    if (strncmp(functionName, "write", 6) == 0) {
        printLog("write\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.write) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        if (recursiveNodeWalker(functionCallNode->child[1], AST_IFJ_CALL) == ERROR) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.write) Nevhodny parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        return NULL_DATA;
    }
    if (strncmp(functionName, "floor", 6) == 0) {
        printLog("floor\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.floor) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues onlyChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL);
        if (onlyChild == NUMBER_DATA || onlyChild == FLOAT_DATA || onlyChild == INT_DATA ||
            onlyChild == UNKNOWN_DATA) {
            return INT_DATA;
        }

        printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.floor) Nevhodny parametr!\n");
        memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        return ERROR;
    }
    if (strncmp(functionName, "str", 4) == 0) {
        printLog("str\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.str) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        if (recursiveNodeWalker(functionCallNode->child[1], AST_IFJ_CALL) == ERROR) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.str) Nevhodny parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }
        return STRING_DATA;
    }
    if (strncmp(functionName, "length", 7) == 0) {
        printLog("length\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.length) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues onlyChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL);
        if (onlyChild == STRING_DATA || onlyChild == UNKNOWN_DATA) {
            return INT_DATA;
        }

        printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.length) Nevhodny parametr!\n");
        memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        return ERROR;
    }
    if (strncmp(functionName, "substring", 10) == 0) {
        printLog("substring\n");

        if (functionCallNode->child[1]->arity != 4) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.substring) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues firstChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny prvni parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues secondChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[1], AST_IFJ_CALL);
        if (secondChild != NUMBER_DATA && secondChild != INT_DATA && secondChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny druhy parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues thirdChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[2], AST_IFJ_CALL);
        if (thirdChild != NUMBER_DATA && thirdChild != INT_DATA && thirdChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny treti parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        return STRING_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }
    if (strncmp(functionName, "strcmp", 7) == 0) {
        printLog("strcmp\n");

        if (functionCallNode->child[1]->arity != 3) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.strcmp) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues firstChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.strcmp) Nevhodny prvni parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues secondChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL);
        if (secondChild != STRING_DATA && secondChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.strcmp) Nevhodny druhy parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        return INT_DATA;
    }

    if (strncmp(functionName, "ord", 4) == 0) {
        printLog("ord\n");

        if (functionCallNode->child[1]->arity != 3) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.ord) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues firstChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.ord) Nevhodny prvni parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues secondChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[1], AST_IFJ_CALL);
        if (secondChild != NUMBER_DATA && secondChild != INT_DATA && secondChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.ord) Nevhodny druhy parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        return INT_DATA;
    }

    if (strncmp(functionName, "chr", 4) == 0) {
        printLog("chr\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.chr) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
            return ERROR;
        }

        RecursiveSemanticAnalysisReturnValues firstChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL);
        if (firstChild != NUMBER_DATA && firstChild != INT_DATA && firstChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.char) Nevhodny parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
            return ERROR;
        }

        return STRING_DATA;
    }

    printError(SEMANTICS_ERROR_UNDEF, "Neznama vestavena funkce: '%s'!\n", functionName);
    memorySafeExit(SEMANTICS_ERROR_UNDEF);
    return ERROR;
}

size_t digitsCount(size_t n) {
    if (n < 10) {
        return 2;  // číslice a \0
    }

    if (n < 100) {
        return 3;  // číslice a \0
    }

    if (n < 1000) {
        return 4;  // číslice a \0
    }

    if (n < 10000) {
        return 5;  // číslice a \0
    }

    printWarning("Funkce by nemela mit vice nez 9999 parametru!\n");
    return 50;
}

SemanticSymbolDataType convertSemanticReturnValueToSemanticDataType(
    RecursiveSemanticAnalysisReturnValues returnedValue) {
    switch (returnedValue) {
        case (NUMBER_DATA): {
            return NUMBER_VAL;
        }
        case (INT_DATA): {
            return INT_VAL;
        }
        case (FLOAT_DATA): {
            return FLOAT_VAL;
        }
        case (STRING_DATA): {
            return STRING_VAL;
        }
        case (BOOLEAN_DATA): {
            return BOOLEAN_VAL;
        }
        case (UNKNOWN_DATA): {
            return UNKNOWN_VAL;
        }
        case (NULL_DATA): {
            return NULL_VAL;
        }
        default: {
            printWarning("Nevhodna navratova hodnota %i\n", returnedValue);
            return NULL_VAL;
        }
    }
}

RecursiveSemanticAnalysisReturnValues convertSemanticDataTypeToSemanticReturnValue(
    SemanticSymbolDataType dataType) {
    switch (dataType) {
        case (NUMBER_VAL): {
            return NUMBER_DATA;
        }
        case (INT_VAL): {
            return INT_DATA;
        }
        case (FLOAT_VAL): {
            return FLOAT_DATA;
        }
        case (STRING_VAL): {
            return STRING_DATA;
        }
        case (BOOLEAN_VAL): {
            return BOOLEAN_DATA;
        }
        case (UNKNOWN_VAL): {
            return UNKNOWN_DATA;
        }
        case (NULL_VAL): {
            return NULL_DATA;
        }
        default: {
            printWarning("Nevhodny datovy typ %i\n", dataType);
            return NULL_DATA;
        }
    }
}

// TODO patička