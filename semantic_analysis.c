// TODO hlavička

#include "semantic_analysis.h"

#include <limits.h>
#include <string.h>

#include "dynamic_stack.h"
#include "support_functions.h"
#include "symtable.h"

// TODO
// - zpřísnit po důkladném promyšlení, co je návratem operace
// - zkusit testy od Kuby, až mu budou procházet
// - vlastní AI testy (pozor na injections)
//

// TODO aktualizace dokumentačních komentářů
// TODO kontrola error kódů!!!

/**
 * @brief Ukazatel na zásobník pro ukládání jednotlivých rámců
 *
 */
static DynamicHashTablePtrStackPtr scopesStack = NULL;

static HashTable* undeclaredReferenceTable = NULL;

/**
 * @brief Vytvoří zásobník
 *
 */
void createScopeStack();

/**
 * @brief Přidá na zásobník nový rámec
 *
 */
void createNewScope();

/**
 * @brief Odebere ze zásobníku rámec
 *
 */
void deleteScope();

/**
 * @brief Prohledá globální rámec
 *
 * @param key Klíč hledaného symbolu
 * @return SemanticSymbol* Ukazatel na nalezený symbol
 */
SemanticSymbol* findGlobalSymbol(char* key);

/**
 * @brief Prohledá všechny rámce, vrací ukazatel na první nalezený symbol a upraví AST pokud se
 * jedná o lokální proměnnou
 *
 * Začíná nejbližším rámcem (vrchol zásobníku)
 * @attention V případě nalezení symbolu upraví AST uzel
 * @param sourceNode Zdrojový AST uzel
 * @param destSymbol Ukazatel na symbol pro upravení, pokud je klíč nalezený v rámcích
 * @return SemanticSymbol* Ukazatel na nalezený symbol
 */
SemanticSymbol* findSymbolAndModifyAST(AstNode* sourceNode, SemanticSymbol* destSymbol);

/**
 * @brief Prohledá všechny rámce, vrací ukazatel na první nalezený odpovídající symbol
 *
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
 */
void insertSymbolGlobalScope(SemanticSymbol symbol);

/**
 * @brief Přidá symbol do aktuálního rámce
 *
 * @param symbol Symbol pro přidání
 * @return true Úspěch
 * @return false Došlo k chybě
 */
void insertSymbolCurrentScope(SemanticSymbol symbol);

/**
 * @brief Vytvoří tabulku symbolů pro uložení volání na nedeklarované funkce
 *
 */
void createUndeclaredFunctionCallsTable();

/**
 * @brief Odstraní z tabulky nedeklarovaných referencí všechny příslušné reference
 *
 * @param referenceSymbolName Název reference nově deklarované reference ( @note zpracovaný pomocí
 * @see initNewSymbol() )
 */
void removeUndeclaredReference(char* referenceSymbolName);

/**
 * @brief Přidá nedeklarovanou referenci do tabulky
 *
 * @param referenceSymbolName Název nedeklarované reference ( @note zpracovaný pomocí
 * @see initNewSymbol() )
 * @param symbol Symbol pro přidání ( @note zpracovaný pomocí
 * @see initNewSymbol() )
 */
void insertUndeclaredReference(char* referenceSymbolName, SemanticSymbol symbol);

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
 */
void initNewSymbol(AstNode* sourceNode, SemanticSymbol* destSymbol, SemanticSymbolTypes symbolType,
                   SemanticSymbolDataType symbolDataType,
                   SemanticSymbolDataType functionReturnValue, unsigned scope);

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
                                                          AstNodeType parentNodeType,
                                                          size_t      childNum);

bool startSematicValidation(AstNode* root) {
    if (root == NULL) {
        printError(INTERNAL_ERROR, "(%s) Nebyl predan AST!\n", __func__);
        memorySafeExit(INTERNAL_ERROR);
        return false;
    }

    createScopeStack();
    createUndeclaredFunctionCallsTable();

    for (size_t i = 0; i < root->arity; i++) {
        if (recursiveNodeWalker(root->child[i], AST_PROGRAM, i) != OK) {
            freeSemanticsMemory();
            return false;
        }
    }

    if (undeclaredReferenceTable != NULL) {
        if (hashTableIsEmpty(undeclaredReferenceTable) == false) {
            printError(SEMANTICS_ERROR_UNDEF,
                       "Reference na nedeklarovany getter, setter nebo funkci!\n");
            memorySafeExit(SEMANTICS_ERROR_UNDEF);
            return false;
        }
    }

    if (scopesStack != NULL) {
        if (stackHashTablePtrIsEmpty(scopesStack) == false) {
            printError(SEMANTICS_ERROR_UNDEF, "Nebyly uzavreny vsechny bloky!\n");
            memorySafeExit(SEMANTICS_ERROR_UNDEF);
            return false;
        }
    }

    freeSemanticsMemory();

    return true;
}

void freeSemanticsMemory() {
    if (scopesStack != NULL) {
        while (stackHashTablePtrIsEmpty(scopesStack) != true) {
            hashTableDealloc(stackHashTablePtrRead(scopesStack));
            stackHashTablePtrPop(scopesStack);
        }
        stackHashTablePtrDestruct(scopesStack);
        scopesStack = NULL;
    }

    if (undeclaredReferenceTable != NULL) {
        hashTableDealloc(undeclaredReferenceTable);
        undeclaredReferenceTable = NULL;
    }
}

RecursiveSemanticAnalysisReturnValues recursiveNodeWalker(AstNode*    currentNode,
                                                          AstNodeType parentNodeType,
                                                          size_t      childNum) {
    if (currentNode == NULL) {
        printWarning("Predan ukazatel na NULL\n");
        return GOT_NODE_NULL;
    }

    switch (currentNode->kind) {
        case (AST_PROGRAM): {
            printLog("AST_PROGRAM\n");
            // Toto bych neměl dostat znovu (už jej zpracovalo startSematicValidation() )
            printError(INTERNAL_ERROR, "Vice AST_PROGRAM neni validni!\n");
            memorySafeExit(INTERNAL_ERROR);
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
            }

            if (recursiveNodeWalker(currentNode->child[1], AST_MAIN_CLASS, 1) != OK) {
                printError(SEMANTICS_ERROR_OTHERS, "Neplatny prikaz v Program()'!\n",
                           currentNode->child[0]->value.str);
                memorySafeExit(SEMANTICS_ERROR_OTHERS);
            }

            return OK;
            break;
        }
        case (AST_MAIN_CLASS_BLOCK): {
            printLog("AST_MAIN_CLASS_BLOCK\n");
            createNewScope();
            for (size_t i = 0; i < currentNode->arity; i++) {
                if (recursiveNodeWalker(currentNode->child[i], AST_MAIN_CLASS_BLOCK, i) != OK) {
                    printError(SEMANTICS_ERROR_OTHERS, "Neplatne prikazy v Program()'!\n",
                               currentNode->child[0]->value.str);
                    memorySafeExit(SEMANTICS_ERROR_OTHERS);
                }
            }

            SemanticSymbol* mainFunctionSymbol = findGlobalSymbol("main$0");

            if (mainFunctionSymbol == NULL) {
                printError(SEMANTICS_ERROR_UNDEF, "Chybi funkce 'main()'!\n");
                memorySafeExit(SEMANTICS_ERROR_UNDEF);
            }

            deleteScope();

            return OK;
            break;
        }
        case (AST_FUNC_DEC): {
            printLog("AST_FUNC_DEC\n");

            SemanticSymbol newSymbol;

            SemanticSymbolDataType functionReturnValue = NULL_VAL;

            recursiveNodeWalker(currentNode->child[1], AST_FUNC_DEC, 1);

            RecursiveSemanticAnalysisReturnValues childValueBlock =
                recursiveNodeWalker(currentNode->child[2], AST_FUNC_DEC, 2);

            if (childValueBlock != OK) {
                functionReturnValue = convertSemanticReturnValueToSemanticDataType(childValueBlock);
            }

            initNewSymbol(currentNode, &newSymbol, FUNCTION, NULL_VAL, functionReturnValue,
                          scopesStack->nextTopElement - 1);

            SemanticSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
            if (foundSymbol != NULL) {
                printError(SEMANTICS_ERROR_REDEF, "Funkce '%s' jiz existuje!\n", newSymbol.name);
                memorySafeExit(SEMANTICS_ERROR_REDEF);
            }

            insertSymbolGlobalScope(newSymbol);

            removeUndeclaredReference(newSymbol.name);

            return OK;
            break;
        }
        case (AST_PARAM): {
            printLog("AST_PARAM\n");

            AstNodeType fakeParentType = AST_PARAM;

            if (parentNodeType == AST_IFJ_CALL) {
                fakeParentType = AST_IFJ_CALL;
            } else {
                createNewScope();
            }
            for (size_t i = 0; i < currentNode->arity; i++) {
                recursiveNodeWalker(currentNode->child[i], fakeParentType, i);
            }

            return OK;
            break;
        }
        case (AST_BLOCK): {
            printLog("AST_BLOCK\n");
            if (parentNodeType != AST_FUNC_DEC && parentNodeType != AST_SETTER &&
                parentNodeType != AST_FOR) {
                createNewScope();
            }

            AstNodeType fakeParentType = AST_BLOCK;

            if (parentNodeType == AST_FUNC_DEC || parentNodeType == AST_SETTER ||
                parentNodeType == AST_GETTER) {
                fakeParentType = AST_FUNC_DEC;
            }

            RecursiveSemanticAnalysisReturnValues childValue =
                recursiveNodeWalker(currentNode->child[0], fakeParentType, 0);

            deleteScope();
            
            // TODO pokud je více return tak nevím, který z nich bude zrovna platný
            // Nějak to vymyslet, aby to pokud bude ve funkci více než 1 return vracelo UNKNOWN_DATA
            childValue =UNKNOWN_DATA;

            return childValue;
            break;
        }
        case (AST_STATEMENT_LIST): {
            printLog("AST_STATEMENT_LIST\n");

            RecursiveSemanticAnalysisReturnValues returnValue = OK;
            bool                                  foundReturn = false;
            unsigned returnCounter = 0;

            RecursiveSemanticAnalysisReturnValues childValue;
            for (size_t i = 0; i < currentNode->arity; i++) {
                childValue = recursiveNodeWalker(currentNode->child[i], AST_STATEMENT_LIST, i);
                if (currentNode->child[i]->kind == AST_RETURN) {
                    foundReturn = true;
                    returnValue = childValue;
                }
                if (currentNode->child[i]->kind == AST_RETURN ||
                    currentNode->child[i]->kind == AST_BREAK ||
                    currentNode->child[i]->kind == AST_CONTINUE) {
                    // Kód po tomto dítěti lze smazat
                    i++;
                    size_t preDeletionArity = currentNode->arity;
                    for (size_t deletionIterator = i; deletionIterator < preDeletionArity;
                         deletionIterator++) {
                        ast_free_whole(currentNode->child[deletionIterator]);
                        currentNode->arity--;
                    }
                    break;
                }
            }
            if (parentNodeType == AST_FUNC_DEC && currentNode->arity == 1 && foundReturn == false) {
                returnValue = childValue;
            }

            return returnValue;
            break;
        }
        case (AST_FUNC_CALL): {
            printLog("AST_FUNC_CALL\n");

            SemanticSymbol         newSymbol;
            SemanticSymbolDataType functionReturnValue = UNKNOWN_VAL;

            initNewSymbol(currentNode, &newSymbol, FUNCTION, NULL_VAL, functionReturnValue,
                          scopesStack->nextTopElement - 1);

            SemanticSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
            if (foundSymbol == NULL) {
                insertUndeclaredReference(newSymbol.name, newSymbol);
            } else {
                // Na stejný řetězec ukazuje AST strom
                functionReturnValue = foundSymbol->returnValue;
            }

            return convertSemanticDataTypeToSemanticReturnValue(functionReturnValue);

            break;
        }
        case (AST_SETTER): {
            printLog("AST_SETTER\n");

            SemanticSymbol newSymbol;

            SemanticSymbolDataType setterStartValue = NULL_VAL;

            recursiveNodeWalker(currentNode->child[1], AST_SETTER, 1);

            RecursiveSemanticAnalysisReturnValues childValueBlock =
                recursiveNodeWalker(currentNode->child[2], AST_SETTER, 2);

            if (childValueBlock != OK) {
                setterStartValue = convertSemanticReturnValueToSemanticDataType(childValueBlock);
            }

            initNewSymbol(currentNode->child[0], &newSymbol, SETTER, setterStartValue, NULL_VAL,
                          scopesStack->nextTopElement - 1);

            SemanticSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
            if (foundSymbol != NULL) {
                printError(SEMANTICS_ERROR_REDEF, "Nastavovac '%s' jiz existuje!\n",
                           newSymbol.name);
                memorySafeExit(SEMANTICS_ERROR_REDEF);
            }

            insertSymbolGlobalScope(newSymbol);

            removeUndeclaredReference(newSymbol.name);

            return OK;

            break;
        }
        case (AST_GETTER): {
            printLog("AST_GETTER\n");

            SemanticSymbol newSymbol;

            SemanticSymbolDataType functionReturnValue = NULL_VAL;

            RecursiveSemanticAnalysisReturnValues childValueBlock =
                recursiveNodeWalker(currentNode->child[2], AST_GETTER, 2);

            if (childValueBlock != OK) {
                functionReturnValue = convertSemanticReturnValueToSemanticDataType(childValueBlock);
            }

            initNewSymbol(currentNode->child[0], &newSymbol, GETTER, NULL_VAL, functionReturnValue,
                          scopesStack->nextTopElement - 1);

            SemanticSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
            if (foundSymbol != NULL) {
                printError(SEMANTICS_ERROR_REDEF, "Ziskavac '%s' jiz existuje!\n", newSymbol.name);
                memorySafeExit(SEMANTICS_ERROR_REDEF);
            }

            insertSymbolGlobalScope(newSymbol);

            removeUndeclaredReference(newSymbol.name);

            return OK;

            break;
        }
        case (AST_VAR): {
            printLog("AST_VAR\n");

            SemanticSymbol newSymbol;

            SemanticSymbolDataType varDataType = NULL_VAL;

            if (currentNode->child[1]->kind != AST_EMPTY) {
                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[1]->child[0], AST_VAR, 1);

                varDataType = convertSemanticReturnValueToSemanticDataType(childValue);
            }

            initNewSymbol(currentNode, &newSymbol, VARIABLE_DEFINITION, varDataType, NULL_VAL,
                          scopesStack->nextTopElement - 1);

            SemanticSymbol* foundSymbol = findSymbolCurrentScope(newSymbol.name);
            if (foundSymbol != NULL) {
                printError(SEMANTICS_ERROR_REDEF, "Promena '%s' v danem ramci jiz existuje!\n",
                           newSymbol.name);
                memorySafeExit(SEMANTICS_ERROR_REDEF);
            }

            insertSymbolCurrentScope(newSymbol);
            return OK;
            break;
        }
        case (AST_ASSIGN): {
            printLog("AST_ASSIGN\n");

            if (parentNodeType == AST_VAR || parentNodeType == AST_GLOBAL_ID) {
                return recursiveNodeWalker(currentNode->child[0], AST_ASSIGN, 0);
            }

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_ASSIGN, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_ASSIGN, 1);

            if (leftChild != IDENTIFIER) {
                printError(SEMANTICS_ERROR_OTHERS,
                           "(=) Levy operand prirazeni musi byt promenna!\n");
                memorySafeExit(SEMANTICS_ERROR_OTHERS);
            }

            SemanticSymbol* leftSymbol = findSymbol(currentNode->child[0]->value.str);

            if (rightChild == IDENTIFIER) {
                SemanticSymbol* rightSymbol = findSymbol(currentNode->child[1]->value.str);

                if (leftSymbol == NULL) {
                    return OK;
                }

                if (rightSymbol == NULL) {
                    leftSymbol->dataType = UNKNOWN_VAL;
                    return OK;
                }

                leftSymbol->dataType = rightSymbol->dataType;
            }

            if (leftSymbol != NULL) {
                leftSymbol->dataType = convertSemanticReturnValueToSemanticDataType(rightChild);
            }
            return OK;

            break;
        }
        case (AST_IF_CHAIN): {
            printLog("AST_IF_CHAIN\n");

            for (size_t i = 0; i < currentNode->arity; i++) {
                recursiveNodeWalker(currentNode->child[i], AST_IF_CHAIN, i);
            }
            return OK;

            break;
        }
        case (AST_IF): {
            printLog("AST_IF\n");

            RecursiveSemanticAnalysisReturnValues conditionChildValue =
                recursiveNodeWalker(currentNode->child[0], AST_IF, 0);

            if (conditionChildValue != OK) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(if) Neplatna podminka!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            }

            // Sémantická kontrola bloku
            recursiveNodeWalker(currentNode->child[1], AST_IF, 1);
            /*RecursiveSemanticAnalysisReturnValues blockValue =
                recursiveNodeWalker(currentNode->child[1], AST_IF, 1);

             if (blockValue != OK) {
                printError(SEMANTICS_ERROR_OTHERS, "(if) Neplatny blok!\n");
                memorySafeExit(SEMANTICS_ERROR_OTHERS);
            } */

            return OK;
            break;
        }
        case (AST_CONDITION): {
            printLog("AST_CONDITION\n");

            if (parentNodeType == AST_FOR) {
                RecursiveSemanticAnalysisReturnValues leftChild =
                    recursiveNodeWalker(currentNode->child[0], AST_CONDITION, 0);

                if (leftChild != IDENTIFIER) {
                    printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                               "(in) Leva strana musi byt identifikator!\n");
                    memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                }

                RecursiveSemanticAnalysisReturnValues rightChild =
                    recursiveNodeWalker(currentNode->child[1], AST_CONDITION, 1);

                if (rightChild != OK) {
                    printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(in) Prava strana neni v poradku!\n");
                    memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                }
                return OK;

            } else {
                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[0], AST_CONDITION, 0);
                if (childValue == OK) {
                    printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(condition) Neplatna podminka!\n");
                    memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
                }
                return OK;
            }
            break;
        }
        case (AST_ELSE): {
            printLog("AST_ELSE\n");

            // Sémantická kontrola bloku
            recursiveNodeWalker(currentNode->child[0], AST_ELSE, 0);

            return OK;
            break;
        }
        case (AST_WHILE): {
            printLog("AST_WHILE\n");
            RecursiveSemanticAnalysisReturnValues conditionChildValue =
                recursiveNodeWalker(currentNode->child[0], AST_WHILE, 0);

            if (conditionChildValue != OK) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(while) Neplatna podminka!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            }

            // Sémantická kontrola bloku
            recursiveNodeWalker(currentNode->child[1], AST_WHILE, 1);

            return OK;
            break;
        }
        case (AST_FOR): {
            printLog("AST_FOR\n");

            // Zpřístupnění iterátoru pro podmínku a blok
            createNewScope();
            recursiveNodeWalker(currentNode->child[0]->child[0], AST_FOR, 0);

            RecursiveSemanticAnalysisReturnValues conditionChildValue =
                recursiveNodeWalker(currentNode->child[0], AST_FOR, 0);

            if (conditionChildValue != OK) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(for) Neplatna podminka!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            }

            // Sémantická kontrola bloku
            recursiveNodeWalker(currentNode->child[1], AST_FOR, 1);

            return OK;
            break;
        }
        case (AST_IDENTIFIER): {

            printLog("AST_IDENTIFIER\n");
            if (parentNodeType == AST_PARAM || parentNodeType == AST_FOR) {
                SemanticSymbol newSymbol;

                SemanticSymbolDataType symbolDataType = UNKNOWN_VAL;

                if (parentNodeType == AST_FOR) {
                    symbolDataType = INT_VAL;
                }

                initNewSymbol(currentNode, &newSymbol, FUNCTION_PARAMETER, symbolDataType, NULL_VAL,
                              scopesStack->nextTopElement - 1);

                SemanticSymbol* foundSymbol = findSymbolCurrentScope(newSymbol.name);

                if (foundSymbol != NULL) {
                    printError(SEMANTICS_ERROR_REDEF, "Promena '%s' v danem ramci jiz existuje!\n",
                               newSymbol.name);
                    memorySafeExit(SEMANTICS_ERROR_REDEF);
                }
                insertSymbolCurrentScope(newSymbol);
                if (parentNodeType == AST_FOR) {
                    return IDENTIFIER;
                }
                return OK;

            } else {
                SemanticSymbol tempSymbol;

                SemanticSymbol* foundSymbol = findSymbolCurrentScope(currentNode->value.str);

                if (foundSymbol == NULL) {
                    foundSymbol = findSymbolAndModifyAST(currentNode, &tempSymbol);
                }

                if (foundSymbol == NULL) {
                    // SETTER
                    if (parentNodeType == AST_ASSIGN && childNum == 0) {
                        SemanticSymbol newSymbol;

                        initNewSymbol(currentNode, &newSymbol, SETTER, UNKNOWN_VAL, NULL_VAL,
                                      scopesStack->nextTopElement - 1);
                        currentNode->kind = AST_SETTER_CALL;

                        insertUndeclaredReference(newSymbol.name, newSymbol);
                        return IDENTIFIER;
                    }

                    // GETTER
                    SemanticSymbol newSymbol;
                    initNewSymbol(currentNode, &newSymbol, GETTER, NULL_VAL, UNKNOWN_VAL,
                                  scopesStack->nextTopElement - 1);

                    insertUndeclaredReference(newSymbol.name, newSymbol);
                    currentNode->kind = AST_GETTER_CALL;

                    if (parentNodeType == AST_CONDITION) {
                        return IDENTIFIER;
                    }
                    return convertSemanticDataTypeToSemanticReturnValue(UNKNOWN_VAL);
                }

                if (parentNodeType == AST_ASSIGN && childNum == 0) {
                    return IDENTIFIER;
                }

                if (parentNodeType == AST_CONDITION) {
                    return IDENTIFIER;
                }

                if (foundSymbol->symbolType == GETTER) {
                    return convertSemanticDataTypeToSemanticReturnValue(foundSymbol->returnValue);
                } else {
                    return convertSemanticDataTypeToSemanticReturnValue(foundSymbol->dataType);
                }
            }
            break;
        }
        case (AST_GLOBAL_ID): {
            printLog("AST_GLOBAL_ID\n");

            if (parentNodeType == AST_STATEMENT_LIST) {
                SemanticSymbol newSymbol;

                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[1], AST_GLOBAL_ID, 1);

                SemanticSymbolDataType varDataType =
                    convertSemanticReturnValueToSemanticDataType(childValue);

                initNewSymbol(currentNode, &newSymbol, GLOBAL, varDataType, NULL_VAL,
                              scopesStack->nextTopElement - 1);

                SemanticSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
                if (foundSymbol != NULL) {
                    // Jedná se o přiřazení nového hodnoty již existující globální proměnné
                    foundSymbol->dataType = newSymbol.dataType;
                    return OK;
                }

                insertSymbolGlobalScope(newSymbol);
                return OK;
            } else {
                SemanticSymbol* foundSymbol = findGlobalSymbol(currentNode->value.str);

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
        case (AST_STRING_BLOCK): {
            printLog("AST_STRING_BLOCK\n");
            return STRING_DATA;
            break;
        }
        case (AST_SMALL_NULL): {
            printLog("AST_SMALL_NULL\n");
            return NULL_DATA;
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
                recursiveNodeWalker(currentNode->child[0], AST_PLUS, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_PLUS, 1);

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
                (leftChild == FLOAT_DATA && rightChild == INT_DATA) ||
                (leftChild == NUMBER_DATA && rightChild == FLOAT_DATA) ||
                (leftChild == NUMBER_DATA && rightChild == INT_DATA) ||
                (leftChild == INT_DATA && rightChild == NUMBER_DATA) ||
                (leftChild == FLOAT_DATA && rightChild == NUMBER_DATA)) {
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

            break;
        }
        case (AST_MINUS): {
            printLog("AST_MINUS\n");

            if (currentNode->arity == 1) {
                RecursiveSemanticAnalysisReturnValues onlyChild =
                    recursiveNodeWalker(currentNode->child[0], AST_MINUS, 0);

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
                    recursiveNodeWalker(currentNode->child[0], AST_MINUS, 0);
                RecursiveSemanticAnalysisReturnValues rightChild =
                    recursiveNodeWalker(currentNode->child[1], AST_MINUS, 1);

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
                    (leftChild == FLOAT_DATA && rightChild == INT_DATA) ||
                    (leftChild == NUMBER_DATA && rightChild == FLOAT_DATA) ||
                    (leftChild == NUMBER_DATA && rightChild == INT_DATA) ||
                    (leftChild == INT_DATA && rightChild == NUMBER_DATA) ||
                    (leftChild == FLOAT_DATA && rightChild == NUMBER_DATA)) {
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

            break;
        }
        case (AST_MUL): {
            printLog("AST_MUL\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_MUL, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_MUL, 1);

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
                (leftChild == FLOAT_DATA && rightChild == INT_DATA) ||
                (leftChild == NUMBER_DATA && rightChild == FLOAT_DATA) ||
                (leftChild == NUMBER_DATA && rightChild == INT_DATA) ||
                (leftChild == INT_DATA && rightChild == NUMBER_DATA) ||
                (leftChild == FLOAT_DATA && rightChild == NUMBER_DATA)) {
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

            break;
        }
        case (AST_DIV): {
            printLog("AST_DIV\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_DIV, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_DIV, 1);

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
                (leftChild == FLOAT_DATA && rightChild == INT_DATA) ||
                (leftChild == NUMBER_DATA && rightChild == FLOAT_DATA) ||
                (leftChild == NUMBER_DATA && rightChild == INT_DATA) ||
                (leftChild == INT_DATA && rightChild == NUMBER_DATA) ||
                (leftChild == FLOAT_DATA && rightChild == NUMBER_DATA)) {
                return NUMBER_DATA;
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

            break;
        }
        case (AST_TERNARY): {
            printLog("AST_TERNARY\n");

            RecursiveSemanticAnalysisReturnValues conditionChild =
                recursiveNodeWalker(currentNode->child[0], AST_TERNARY, 0);
            if (conditionChild == OK) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(?:) Neplatna podminka!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            }

            RecursiveSemanticAnalysisReturnValues trueChild =
                recursiveNodeWalker(currentNode->child[1], AST_TERNARY, 1);
            if (trueChild == OK) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                           "(?:) Neplatny vyraz pri platne podmince!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            }

            RecursiveSemanticAnalysisReturnValues falseChild =
                recursiveNodeWalker(currentNode->child[2], AST_TERNARY, 2);
            if (falseChild == OK) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                           "(?:) Neplatny vyraz pri neplatne podmince!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            }

            return UNKNOWN_DATA;

            break;
        }
        case (AST_EQ): {
            printLog("AST_EQ\n");

            recursiveNodeWalker(currentNode->child[0], AST_EQ, 0);
            recursiveNodeWalker(currentNode->child[1], AST_EQ, 1);

            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;

            break;
        }
        case (AST_NEQ): {
            printLog("AST_NEQ\n");

            recursiveNodeWalker(currentNode->child[0], AST_EQ, 0);
            recursiveNodeWalker(currentNode->child[1], AST_EQ, 1);

            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;

            break;
        }
        case (AST_LT): {
            printLog("AST_LT\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_LT, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_LT, 1);

            if (leftChild == NUMBER_DATA && rightChild == NUMBER_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == INT_DATA && rightChild == INT_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == FLOAT_DATA && rightChild == FLOAT_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == NUMBER_DATA && (rightChild == INT_DATA || rightChild == FLOAT_DATA ||
                                             rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == INT_DATA && (rightChild == NUMBER_DATA || rightChild == FLOAT_DATA ||
                                          rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == FLOAT_DATA && (rightChild == INT_DATA || rightChild == NUMBER_DATA ||
                                            rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == UNKNOWN_DATA &&
                (rightChild == INT_DATA || rightChild == FLOAT_DATA || rightChild == NUMBER_DATA)) {
                return BOOLEAN_DATA;
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(<) Nekompatibilni datove typy!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);

            break;
        }
        case (AST_GT): {
            printLog("AST_GT\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_GT, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_GT, 1);

            if (leftChild == NUMBER_DATA && rightChild == NUMBER_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == INT_DATA && rightChild == INT_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == FLOAT_DATA && rightChild == FLOAT_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == NUMBER_DATA && (rightChild == INT_DATA || rightChild == FLOAT_DATA ||
                                             rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == INT_DATA && (rightChild == NUMBER_DATA || rightChild == FLOAT_DATA ||
                                          rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == FLOAT_DATA && (rightChild == INT_DATA || rightChild == NUMBER_DATA ||
                                            rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == UNKNOWN_DATA &&
                (rightChild == INT_DATA || rightChild == FLOAT_DATA || rightChild == NUMBER_DATA)) {
                return BOOLEAN_DATA;
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(>) Nekompatibilni datove typy!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);

            break;
        }
        case (AST_GTE): {
            printLog("AST_GTE\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_GTE, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_GTE, 1);

            if (leftChild == NUMBER_DATA && rightChild == NUMBER_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == INT_DATA && rightChild == INT_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == FLOAT_DATA && rightChild == FLOAT_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == NUMBER_DATA && (rightChild == INT_DATA || rightChild == FLOAT_DATA ||
                                             rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == INT_DATA && (rightChild == NUMBER_DATA || rightChild == FLOAT_DATA ||
                                          rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == FLOAT_DATA && (rightChild == INT_DATA || rightChild == NUMBER_DATA ||
                                            rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == UNKNOWN_DATA &&
                (rightChild == INT_DATA || rightChild == FLOAT_DATA || rightChild == NUMBER_DATA)) {
                return BOOLEAN_DATA;
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(>=) Nekompatibilni datove typy!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);

            break;
        }
        case (AST_LTE): {
            printLog("AST_LTE\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_LTE, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_LTE, 1);

            if (leftChild == NUMBER_DATA && rightChild == NUMBER_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == INT_DATA && rightChild == INT_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == FLOAT_DATA && rightChild == FLOAT_DATA) {
                return BOOLEAN_DATA;
            }

            if (leftChild == NUMBER_DATA && (rightChild == INT_DATA || rightChild == FLOAT_DATA ||
                                             rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == INT_DATA && (rightChild == NUMBER_DATA || rightChild == FLOAT_DATA ||
                                          rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == FLOAT_DATA && (rightChild == INT_DATA || rightChild == NUMBER_DATA ||
                                            rightChild == UNKNOWN_DATA)) {
                return BOOLEAN_DATA;
            }

            if (leftChild == UNKNOWN_DATA &&
                (rightChild == INT_DATA || rightChild == FLOAT_DATA || rightChild == NUMBER_DATA)) {
                return BOOLEAN_DATA;
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(<=) Nekompatibilni datove typy!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);

            break;
        }
        case (AST_NOT): {
            printLog("AST_NOT\n");

            RecursiveSemanticAnalysisReturnValues onlyChild = recursiveNodeWalker(
                currentNode->child[0], AST_NOT, 0);  // Negace má pouze jedno dítě

            if (onlyChild == BOOLEAN_DATA) {
                return BOOLEAN_DATA;
            }

            if (onlyChild == UNKNOWN_DATA) {
                return BOOLEAN_DATA;  // Výsledek logické negace bude vždy bool hodnota
            }

            printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(!) Datovy typy musi byt kompatibilni!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);

            break;
        }
        case (AST_AND): {
            printLog("AST_AND\n");

            recursiveNodeWalker(currentNode->child[0], AST_AND, 0);
            recursiveNodeWalker(currentNode->child[1], AST_AND, 1);

            // Nelze říct, co bude návratová hodnota
            return UNKNOWN_DATA;

            break;
        }
        case (AST_OR): {
            printLog("AST_OR\n");

            recursiveNodeWalker(currentNode->child[0], AST_OR, 0);
            recursiveNodeWalker(currentNode->child[1], AST_OR, 1);

            // Nelze říct, co bude návratová hodnota
            return UNKNOWN_DATA;

            break;
        }
        case (AST_IS): {
            printLog("AST_IS\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_IS, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_IS, 1);

            if (leftChild == OK || leftChild == IDENTIFIER) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                           "(is) Leva strana operatoru musi mit jako vysledek datovy typ!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            }

            if (rightChild != DATATYPE_KEYWORD) {
                printError(SEMANTICS_ERROR_UNCOMPATIBLE,
                           "(is) Prava strana operatoru musi byt nazev datoveho typu!\n");
                memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);
            }

            return BOOLEAN_DATA;

            break;
        }
        case (AST_DOUBLE_DOT): {
            printLog("AST_DOUBLE_DOT\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_DOUBLE_DOT, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_DOUBLE_DOT, 1);

            if ((leftChild == NUMBER_DATA || leftChild == INT_DATA || leftChild == UNKNOWN_DATA) &&
                (rightChild == NUMBER_DATA || rightChild == INT_DATA ||
                 rightChild == UNKNOWN_DATA)) {
                return OK;
            }
            printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(..) Operandy musi byt cela cisla!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);

            break;
        }
        case (AST_TRIPLE_DOT): {
            printLog("AST_TRIPLE_DOT\n");

            RecursiveSemanticAnalysisReturnValues leftChild =
                recursiveNodeWalker(currentNode->child[0], AST_TRIPLE_DOT, 0);
            RecursiveSemanticAnalysisReturnValues rightChild =
                recursiveNodeWalker(currentNode->child[1], AST_TRIPLE_DOT, 1);

            if ((leftChild == NUMBER_DATA || leftChild == INT_DATA || leftChild == UNKNOWN_DATA) &&
                (rightChild == NUMBER_DATA || rightChild == INT_DATA ||
                 rightChild == UNKNOWN_DATA)) {
                return OK;
            }
            printError(SEMANTICS_ERROR_UNCOMPATIBLE, "(...) Operandy musi byt cela cisla!\n");
            memorySafeExit(SEMANTICS_ERROR_UNCOMPATIBLE);

            break;
        }
        case (AST_RETURN): {
            printLog("AST_RETURN\n");

            RecursiveSemanticAnalysisReturnValues returnDataType = NULL_DATA;

            if (currentNode->child[0]->kind != AST_EMPTY) {
                RecursiveSemanticAnalysisReturnValues childValue =
                    recursiveNodeWalker(currentNode->child[0], AST_RETURN, 0);

                if (returnDataType != OK)
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
            if (parentNodeType == AST_IS) {
                // Značí datový typ Null
                return DATATYPE_KEYWORD;
            }
            // Značí prázdný jednořádkový blok (jeho návratovou hodnotu)
            return NULL_DATA;
            break;
        }
        case (AST_BREAK): {
            printLog("AST_BREAK\n");

            return OK;
            break;
        }
        case (AST_CONTINUE): {
            printLog("AST_CONTINUE\n");

            return OK;
            break;
        }
        case (AST_LPAREN): {
            printLog("AST_LPAREN\n");
            // Tento AST uzel bych neměl dostat
            printError(INTERNAL_ERROR, "Uzel AST_LPAREN neni validni uzel stromu!\n");
            memorySafeExit(INTERNAL_ERROR);
            break;
        }
        case (AST_RPAREN): {
            printLog("AST_RPAREN\n");
            // Tento AST uzel bych neměl dostat
            printError(INTERNAL_ERROR, "Uzel AST_RPAREN neni validni uzel stromu!\n");
            memorySafeExit(INTERNAL_ERROR);
            break;
        }
        case (AST_SETTER_CALL): {
            printLog("AST_SETTER_CALL\n");

            // Tento AST uzel bych neměl dostat
            printError(INTERNAL_ERROR, "Uzel AST_SETTER_CALL neni validni uzel stromu!\n");
            memorySafeExit(INTERNAL_ERROR);

            break;
        }
        case (AST_GETTER_CALL): {
            printLog("AST_GETTER_CALL\n");

            // Tento AST uzel bych neměl dostat
            printError(INTERNAL_ERROR, "Uzel AST_GETTER_CALL neni validni uzel stromu!\n");
            memorySafeExit(INTERNAL_ERROR);

            break;
        }
        case (AST_FLOAT_LIT_EXP): {
            printLog("AST_FLOAT_LIT_EXP\n");

            // Tento AST uzel bych neměl dostat
            printError(INTERNAL_ERROR, "Uzel AST_FLOAT_LIT_EXP neni validni uzel stromu!\n");
            memorySafeExit(INTERNAL_ERROR);

            break;
        }
                case (AST_INT_LIT_EXP): {
            printLog("AST_INT_LIT_EXP\n");

            // Tento AST uzel bych neměl dostat
            printError(INTERNAL_ERROR, "Uzel AST_INT_LIT_EXP neni validni uzel stromu!\n");
            memorySafeExit(INTERNAL_ERROR);

            break;
        }
    }

    printWarning("(%s) Neznamy stav: %i!\n", __func__, currentNode->kind);
    return GOT_NODE_NULL;
}

void createScopeStack() {
    DynamicHashTablePtrStackPtr stack = stackHashTablePtrConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    if (stack == NULL) {
        printError(INTERNAL_ERROR, "Selhalo vytvareni zasobniku!\n");
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    scopesStack = stack;
}

void createNewScope() {
    HashTable* newScope = hashTableInit();
    if (newScope == NULL) {
        printError(INTERNAL_ERROR, "Selhalo vytvareni ramce!\n");
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    hashTableInitItems(newScope);

    stackHashTablePtrPush(scopesStack, newScope);
}

void deleteScope() {
    if (scopesStack == NULL) {
        printError(INTERNAL_ERROR, "(%s) Zasobnik neexistuje!\n", __func__);
        memorySafeExit(INTERNAL_ERROR);
        return;
    }
    if (stackHashTablePtrIsEmpty(scopesStack)) {
        printError(INTERNAL_ERROR, "(%s) Snaha odstranit prvek z prazdneho zasobniku!\n", __func__);
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    HashTable* newScope = stackHashTablePtrRead(scopesStack);
    if (newScope == NULL) {
        printError(INTERNAL_ERROR, "(%s) Na zasobniku byl prazdny ukazatel!\n", __func__);
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    hashTableDealloc(newScope);

    stackHashTablePtrPop(scopesStack);
}

SemanticSymbol* findSymbolAndModifyAST(AstNode* sourceNode, SemanticSymbol* destSymbol) {
    unsigned toSearchScopeIndex = scopesStack->nextTopElement;
    int      index              = -1;
    char*    key                = sourceNode->value.str;

    unsigned symbolNameLength =
        strlen(key) + 2 + digitsCount(scopesStack->nextTopElement);  // prostor pro separátor '$$'

    char symbolName[symbolNameLength];

    while (toSearchScopeIndex != 0) {
        toSearchScopeIndex--;

        // variableName$$scopeNumber ($$ pro rozlišení mezi funkcí a lokální proměnnou)
        snprintf(symbolName, symbolNameLength, "%s$$%u", key, toSearchScopeIndex);

        index = hashTableSearchItem(scopesStack->array[toSearchScopeIndex], symbolName);
        if (index != -1) {
            break;
        }
    }

    if (index == -1) {
        // Může to být i získávač nebo nastavovač
        SemanticSymbol* foundSymbol = findGlobalSymbol(key);
        if (foundSymbol != NULL) {
            if (foundSymbol->symbolType == GETTER) {
                sourceNode->kind = AST_GETTER_CALL;
            } else if (foundSymbol->symbolType == SETTER) {
                sourceNode->kind = AST_SETTER_CALL;
            }
        }
        return foundSymbol;
    }

    SemanticSymbol* foundSymbol = &(scopesStack->array[toSearchScopeIndex]->items[index].symbol);

    if (destSymbol != NULL) {
        // Potřeba přejmenovat symbol v AST stromě
        initNewSymbol(sourceNode, destSymbol, VARIABLE, foundSymbol->dataType,
                      foundSymbol->returnValue, toSearchScopeIndex);
    }
    return foundSymbol;
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

    return &(scopesStack->array[toSearchScopeIndex]->items[index].symbol);
}

SemanticSymbol* findGlobalSymbol(char* key) {
    int index = hashTableSearchItem(scopesStack->array[0], key);

    if (index == -1) {
        return NULL;
    }

    return &(scopesStack->array[0]->items[index].symbol);
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

void insertSymbolGlobalScope(SemanticSymbol symbol) {
    if (scopesStack == NULL) {
        printError(INTERNAL_ERROR, "Neexistuje zasobnik!\n");
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    if (stackHashTablePtrIsEmpty(scopesStack) == true) {
        printError(INTERNAL_ERROR, "Neni inicializovan zadny ramec!\n");
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    int index = hashTableInsertItem(scopesStack->array[0], symbol.name, symbol);

    if (index == -1) {
        printError(INTERNAL_ERROR, "Selhalo vlozeni prvku do globalniho ramce!\n");
        memorySafeExit(INTERNAL_ERROR);
        return;
    }
}

void insertSymbolCurrentScope(SemanticSymbol symbol) {
    if (scopesStack == NULL) {
        printError(INTERNAL_ERROR, "Neexistuje zasobnik!\n");
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    if (stackHashTablePtrIsEmpty(scopesStack) == true) {
        printError(INTERNAL_ERROR, "Neni inicializovan zadny ramec!\n");
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    HashTable* currentScope = stackHashTablePtrRead(scopesStack);

    int index = hashTableInsertItem(currentScope, symbol.name, symbol);

    if (index == -1) {
        printError(INTERNAL_ERROR, "Selhalo vlozeni prvku do ramce!\n");
        memorySafeExit(INTERNAL_ERROR);
    };
}

void createUndeclaredFunctionCallsTable() {
    undeclaredReferenceTable = hashTableInit();
    if (undeclaredReferenceTable == NULL) {
        printError(INTERNAL_ERROR, "Selhalo vytvoreni tabulky nedeklarovanych referenci!\n");
        memorySafeExit(INTERNAL_ERROR);
    }
}

void removeUndeclaredReference(char* referenceSymbolName) {
    if (undeclaredReferenceTable == NULL) {
        printError(INTERNAL_ERROR, "(%s) Tabulka pro nedeklarovane reference neexistuje!\n",
                   __func__);
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    if (hashTableSearchItem(undeclaredReferenceTable, referenceSymbolName) == -1) {
        return;
    }

    hashTableDeleteItem(undeclaredReferenceTable, referenceSymbolName);
}

void insertUndeclaredReference(char* referenceSymbolName, SemanticSymbol symbol) {
    if (undeclaredReferenceTable == NULL) {
        // Uvolnění paměti zajišťuje AST strom
        printError(INTERNAL_ERROR, "(%s) Tabulka pro nedeklarovane reference neexistuje!\n",
                   __func__);
        memorySafeExit(INTERNAL_ERROR);
        return;
    }

    if (hashTableInsertItem(undeclaredReferenceTable, referenceSymbolName, symbol) == -1) {
        // Uvolnění paměti zajišťuje AST strom
        printError(INTERNAL_ERROR, "(%s) Nepodarilo se vlozit prvek do tabulky!\n", __func__);
        memorySafeExit(INTERNAL_ERROR);
    }
}

void initNewSymbol(AstNode* sourceNode, SemanticSymbol* destSymbol, SemanticSymbolTypes symbolType,
                   SemanticSymbolDataType symbolDataType,
                   SemanticSymbolDataType functionReturnValue, unsigned scope) {
    if (sourceNode == NULL || destSymbol == NULL) {
        printError(INTERNAL_ERROR, "Neplatne hodnoty!\n");
        memorySafeExit(INTERNAL_ERROR);
        return;
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
                return;
            }

            destSymbol->name        = newName;
            destSymbol->returnValue = functionReturnValue;

            // functionName$parametrCount
            snprintf(destSymbol->name, newNameLength, "%s$%lu", sourceNode->child[0]->value.str,
                     numberOfParams);

            destSymbol->paramCount = sourceNode->child[1]->arity;

            free(sourceNode->child[0]->value.str);
            sourceNode->child[0]->value.str = newName;
            break;
        }
        case (VARIABLE_DEFINITION): {
            unsigned newNameLength = strlen(sourceNode->child[0]->value.str) + 2 +
                                     digitsCount(scope);  // prostor pro separátor '$$'

            char* newName = malloc(sizeof(char) * newNameLength);

            if (newName == NULL) {
                printError(INTERNAL_ERROR, "(%s) Selhala alokace pameti!", __func__);
                memorySafeExit(INTERNAL_ERROR);
                return;
            }

            destSymbol->name = newName;

            snprintf(destSymbol->name, newNameLength, "%s$$%u", sourceNode->child[0]->value.str,
                     scope);

            free(sourceNode->child[0]->value.str);
            sourceNode->child[0]->value.str = newName;
            destSymbol->dataType            = symbolDataType;
            break;
        }
        case (VARIABLE):
        case (FUNCTION_PARAMETER): {
            unsigned newNameLength = strlen(sourceNode->value.str) + 2 +
                                     digitsCount(scope);  // prostor pro separátor '$$'

            char* newName = malloc(sizeof(char) * newNameLength);

            if (newName == NULL) {
                printError(INTERNAL_ERROR, "(%s) Selhala alokace pameti!", __func__);
                memorySafeExit(INTERNAL_ERROR);
                return;
            }

            destSymbol->name = newName;

            // variableName$$scopeNumber ($$ pro rozlišení mezi funkcí a lokální proměnnou)
            snprintf(destSymbol->name, newNameLength, "%s$$%u", sourceNode->value.str, scope);

            free(sourceNode->value.str);
            sourceNode->value.str = newName;
            destSymbol->dataType  = symbolDataType;
            break;
        }
        case (GLOBAL): {
            destSymbol->name     = sourceNode->child[0]->value.str;
            destSymbol->dataType = symbolDataType;
            break;
        }
        case (SETTER): {
            destSymbol->name     = sourceNode->value.str;
            destSymbol->dataType = symbolDataType;
            break;
        }
        case (GETTER): {
            destSymbol->name        = sourceNode->value.str;
            destSymbol->returnValue = functionReturnValue;
            break;
        }
        default: {
            printError(INTERNAL_ERROR, "(%s) Neplatny typ symbolu '%u`", __func__, symbolType);
            memorySafeExit(INTERNAL_ERROR);
            return;
        }
    }

    return;
}

RecursiveSemanticAnalysisReturnValues handleBuildinFunctions(AstNode* functionCallNode) {
    char* functionName = functionCallNode->child[0]->value.str;

    if (strncmp(functionName, "read_str", 9) == 0) {
        printLog("read_str\n");

        if (functionCallNode->child[1]->arity != 1) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.read_str) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        return STRING_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "read_num", 9) == 0) {
        printLog("read_num\n");

        if (functionCallNode->child[1]->arity != 1) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.read_num) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        return NUMBER_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "read_bool", 10) == 0) {
        printLog("read_bool\n");

        if (functionCallNode->child[1]->arity != 1) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.read_bool) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        return BOOLEAN_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "write", 6) == 0) {
        printLog("write\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.write) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        recursiveNodeWalker(functionCallNode->child[1], AST_IFJ_CALL, 1);

        return NULL_DATA;
    }

    if (strncmp(functionName, "floor", 6) == 0) {
        printLog("floor\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.floor) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        RecursiveSemanticAnalysisReturnValues onlyChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (onlyChild == NUMBER_DATA || onlyChild == FLOAT_DATA || onlyChild == INT_DATA ||
            onlyChild == UNKNOWN_DATA) {
            return INT_DATA;
        }

        printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.floor) Nevhodny parametr!\n");
        memorySafeExit(SEMANTICS_ERROR_INCORRECT);
    }

    if (strncmp(functionName, "str", 4) == 0) {
        printLog("str\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.str) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        recursiveNodeWalker(functionCallNode->child[1], AST_IFJ_CALL, 1);

        return STRING_DATA;
    }

    if (strncmp(functionName, "length", 7) == 0) {
        printLog("length\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.length) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        RecursiveSemanticAnalysisReturnValues onlyChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (onlyChild == STRING_DATA || onlyChild == UNKNOWN_DATA) {
            return INT_DATA;
        }

        printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.length) Nevhodny parametr!\n");
        memorySafeExit(SEMANTICS_ERROR_INCORRECT);
    }

    if (strncmp(functionName, "substring", 10) == 0) {
        printLog("substring\n");

        if (functionCallNode->child[1]->arity != 4) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.substring) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        RecursiveSemanticAnalysisReturnValues firstChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny prvni parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        }

        RecursiveSemanticAnalysisReturnValues secondChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[1], AST_IFJ_CALL, 1);
        if (secondChild != NUMBER_DATA && secondChild != INT_DATA && secondChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny druhy parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        }

        RecursiveSemanticAnalysisReturnValues thirdChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[2], AST_IFJ_CALL, 2);
        if (thirdChild != NUMBER_DATA && thirdChild != INT_DATA && thirdChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny treti parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        }

        return STRING_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "strcmp", 7) == 0) {
        printLog("strcmp\n");

        if (functionCallNode->child[1]->arity != 3) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.strcmp) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        RecursiveSemanticAnalysisReturnValues firstChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.strcmp) Nevhodny prvni parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        }

        RecursiveSemanticAnalysisReturnValues secondChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[1], AST_IFJ_CALL, 1);
        if (secondChild != STRING_DATA && secondChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.strcmp) Nevhodny druhy parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        }

        return INT_DATA;
    }

    if (strncmp(functionName, "ord", 4) == 0) {
        printLog("ord\n");

        if (functionCallNode->child[1]->arity != 3) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.ord) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        RecursiveSemanticAnalysisReturnValues firstChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.ord) Nevhodny prvni parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        }

        RecursiveSemanticAnalysisReturnValues secondChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[1], AST_IFJ_CALL, 1);
        if (secondChild != NUMBER_DATA && secondChild != INT_DATA && secondChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.ord) Nevhodny druhy parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        }

        return INT_DATA;
    }

    if (strncmp(functionName, "chr", 4) == 0) {
        printLog("chr\n");

        if (functionCallNode->child[1]->arity != 2) {
            printError(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.chr) Nevhodny pocet parametru!\n");
            memorySafeExit(SEMANTICS_ERROR_UNEXPECTED);
        }

        RecursiveSemanticAnalysisReturnValues firstChild =
            recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (firstChild != NUMBER_DATA && firstChild != INT_DATA && firstChild != UNKNOWN_DATA) {
            printError(SEMANTICS_ERROR_INCORRECT, "(Ifj.char) Nevhodny parametr!\n");
            memorySafeExit(SEMANTICS_ERROR_INCORRECT);
        }

        return STRING_DATA;
    }

    printError(SEMANTICS_ERROR_UNDEF, "Neznama vestavena funkce: '%s'!\n", functionName);
    memorySafeExit(SEMANTICS_ERROR_UNDEF);
    return GOT_NODE_NULL;
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