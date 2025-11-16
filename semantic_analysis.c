// TODO hlavička

#include "semantic_analysis.h"

#include <limits.h>
#include <string.h>

#include "dynamic_stack.h"
#include "support_functions.h"
#include "symtable.h"

#define EXEC_END(CODE, TEXT)    \
    do {                        \
        printError(CODE, TEXT); \
        memorySafeExit(CODE);   \
    } while (0)

#define NEWEST_SCOPE scopesStack->nextTopElement - 1

// TODO aktualizace dokumentačních komentářů

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
 * @return SemSymbol* Ukazatel na nalezený symbol
 */
SemSymbol* findGlobalSymbol(char* key);

/**
 * @brief Prohledá všechny rámce, vrací ukazatel na první nalezený symbol a upraví AST pokud se
 * jedná o lokální proměnnou
 *
 * Začíná nejbližším rámcem (vrchol zásobníku)
 * @attention V případě nalezení symbolu upraví AST uzel
 * @param sourceNode Zdrojový AST uzel
 * @param destSymbol Ukazatel na symbol pro upravení, pokud je klíč nalezený v rámcích
 * @return SemSymbol* Ukazatel na nalezený symbol
 */
SemSymbol* findSymbolAndModifyAST(AstNode* sourceNode, SemSymbol* destSymbol);

/**
 * @brief Prohledá všechny rámce, vrací ukazatel na první nalezený odpovídající symbol
 *
 * @param key Klíč hledaného symbolu
 * @return SemSymbol* Ukazatel na nalezený symbol
 */
SemSymbol* findSymbol(char* key);

/**
 * @brief Prohledá aktuální rámec (vrchol zásobníku) a vrací ukazatel na nalezený symbol
 *
 * @param key Klíč hledaného symbolu
 * @return SemSymbol* Ukazatel na nalezený symbol
 */
SemSymbol* findSymbolCurrentScope(char* key);

/**
 * @brief Přidá symbol do globálního rámce (první vytvořený rámec)
 *
 * @param symbol Symbol pro přidání
 */
void insertSymbolGlobalScope(SemSymbol symbol);

/**
 * @brief Přidá symbol do aktuálního rámce
 *
 * @param symbol Symbol pro přidání
 * @return true Úspěch
 * @return false Došlo k chybě
 */
void insertSymbolCurrentScope(SemSymbol symbol);

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
void insertUndeclaredReference(char* referenceSymbolName, SemSymbol symbol);

/**
 * @brief Převede informace z AST uzlu do symbolu pro uložení do tabulky
 *
 * @note Pokud je cílový symbol funkce, tak ji přejmenuje v sourceNode i destSymbol na
 * "nazevFunkce$pocetParametru"
 * @param sourceNode Zdrojový AST uzel
 * @param destSymbol Symbol pro upravení
 */
void initNewSymbol(AstNode* sourceNode, SemSymbol* destSymbol);

/**
 * @brief Zpracovává sémantickou kontrolu volání vestavěných funkcí
 *
 * @param functionCallNode Uzel typu IFJ_CALL
 * @return SemValidationStates Návratová hodnota volání funkce
 */
SemValidationStates handleBuildinFunctions(AstNode* functionCallNode);

/**
 * @brief Zpracuje sémantickou analýzu porovnávacích operátorů
 *
 * @note (s výjimkou == a !=)
 * @param currentNode AST uzel operátoru
 * @param operatorCode Typ uzlu (kód operátoru)
 * @param operatorName Řetězec o maximálně 2 znacích sloužící pro identifikaci operátoru při výpisu chyby (např.: >=)
 * @return SemValidationStates Výsledek sémantické analýzy podstromu
 */
SemValidationStates handleComparisonOperators(AstNode* currentNode, AstNodeType operatorCode, char* operatorName);

/**
 * @brief Zpracuje aritmetické (a řetězcové)
 *
 * @param currentNode AST uzel operátoru
 * @param operatorCode Typ uzlu (kód operátoru)
 * @param operatorName Řetězec o maximálně 1 znaku sloužící pro identifikaci operátoru při výpisu chyby (např.: +)
 * @return SemValidationStates Výsledek sémantické analýzy podstromu
 */
SemValidationStates handleMathOperators(AstNode* currentNode, AstNodeType operatorCode, char* operatorName);

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
 * @return SemSymbDataType odpovídající hodnota datového typu
 */
SemSymbDataType convSemValStatesToSemSymbDType(SemValidationStates returnedValue);

/**
 * @brief Převádí hodnotu datového typu na návratovou hodnotu rekurzivního procházení stromu
 *
 * @param returnedValue Hodnota datového typu
 * @return SemValidationStates Odpovídající návratová hodnota
 */
SemValidationStates convSemSymbDTypeToSemValStates(SemSymbDataType dataType);

/**
 * @brief Funkce pro rekurzivní procházení stromu
 *
 * Chování a návratový kód závisí na typu vstupního uzlu.
 * @param currentNode Vstupní uzel
 * @param parentNodeType Uzel nadřazený vstupnímu uzlu
 * @return SemValidationStates Návratový kód
 */
SemValidationStates recursiveNodeWalker(AstNode* currentNode, AstNodeType parentNodeType, size_t childNum);

bool startSematicValidation(AstNode* root) {
    if (root == NULL) {
        EXEC_END(INTERNAL_ERROR, "Nebyl predan AST!\n");
        return false;
    }

    createScopeStack();
    createUndeclaredFunctionCallsTable();

    for (size_t i = 0; i < root->arity; i++) {
        if (recursiveNodeWalker(root->child[i], AST_PROGRAM, i) != OK) {
            EXEC_END(INTERNAL_ERROR, "Spatna struktura stromu!\n");
            return false;
        }
    }

    if (undeclaredReferenceTable != NULL) {
        if (hashTableIsEmpty(undeclaredReferenceTable) == false) {
            printLog("Tabulka nedeklarovaných referencí není prázdná! Výpis obsahu níže:\n");
            for (int i = 0; i < undeclaredReferenceTable->tableCapacity; i++) {
                if (undeclaredReferenceTable->items[i].key != NULL) {
                    printLog("Klíč:      %s\n", undeclaredReferenceTable->items[i].key);
                    printLog("isDeleted: %i\n", undeclaredReferenceTable->items[i].isDeleted);
                    printLog("isEmpty:   %i\n", undeclaredReferenceTable->items[i].isEmpty);
                    printLog("-\n");
                }
            }
            EXEC_END(SEMANTICS_ERROR_UNDEF, "Reference na nedeklarovany getter, setter nebo funkci!\n");
            return false;
        }
    }

    if (scopesStack != NULL) {
        if (stackHashTablePtrIsEmpty(scopesStack) == false) {
            EXEC_END(SEMANTICS_ERROR_OTHERS, "Nebyly uzavreny vsechny bloky!\n");
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

SemValidationStates recursiveNodeWalker(AstNode* currentNode, AstNodeType parentNodeType, size_t childNum) {
    static int returnCounter = 0;
    if (currentNode == NULL) {
        printWarning("Predan ukazatel na NULL\n");
        return GOT_NODE_NULL;
    }

    switch (currentNode->kind) {
        case (AST_PROGRAM): {
            printLog("AST_PROGRAM\n");
            // Toto bych neměl dostat znovu (už jej zpracovalo startSematicValidation() )
            EXEC_END(INTERNAL_ERROR, "Vice AST_PROGRAM neni validni!\n");
            break;
        }
        case (AST_PROLOG): {
            printLog("AST_PROLOG\n");
            return OK;
        }
        case (AST_MAIN_CLASS): {
            printLog("AST_MAIN_CLASS\n");
            if (currentNode->child[0]->value.has_str == false ||
                strncmp(currentNode->child[0]->value.str, "Program", 8) != 0) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "Ocekavan 'Program'!\n");
            }

            if (recursiveNodeWalker(currentNode->child[1], AST_MAIN_CLASS, 1) != OK) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "Neplatny prikaz v Program()'!\n");
            }

            return OK;
        }
        case (AST_MAIN_CLASS_BLOCK): {
            printLog("AST_MAIN_CLASS_BLOCK\n");
            createNewScope();
            for (size_t i = 0; i < currentNode->arity; i++) {
                if (recursiveNodeWalker(currentNode->child[i], AST_MAIN_CLASS_BLOCK, i) != OK) {
                    EXEC_END(SEMANTICS_ERROR_OTHERS, "Neplatne prikazy v Program()'!\n");
                }
            }

            SemSymbol* mainFunctionSymbol = findGlobalSymbol("main$0");

            if (mainFunctionSymbol == NULL) {
                EXEC_END(SEMANTICS_ERROR_UNDEF, "Chybi funkce 'main()'!\n");
            }

            deleteScope();

            return OK;
        }
        case (AST_FUNC_DEC): {
            printLog("AST_FUNC_DEC\n");

            SemSymbol newSymbol = {.symbolType = FUNCTION, .dataType = NULL_VAL, .scope = NEWEST_SCOPE};

            SemSymbDataType functionReturnValue = NULL_VAL;

            recursiveNodeWalker(currentNode->child[1], AST_FUNC_DEC, 1);

            SemValidationStates childValueBlock = recursiveNodeWalker(currentNode->child[2], AST_FUNC_DEC, 2);

            if (childValueBlock != OK) {
                functionReturnValue = convSemValStatesToSemSymbDType(childValueBlock);
            }

            newSymbol.returnValue = functionReturnValue;
            initNewSymbol(currentNode, &newSymbol);

            SemSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
            if (foundSymbol != NULL) {
                EXEC_END(SEMANTICS_ERROR_REDEF, "Funkce jiz existuje!\n");
            }

            insertSymbolGlobalScope(newSymbol);

            removeUndeclaredReference(newSymbol.name);

            return OK;
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

            if(parentNodeType == AST_FUNC_CALL) {
                deleteScope();
            }

            return OK;
        }
        case (AST_BLOCK): {
            printLog("AST_BLOCK\n");
            if (parentNodeType != AST_FUNC_DEC && parentNodeType != AST_SETTER && parentNodeType != AST_FOR) {
                createNewScope();
            }
            SemValidationStates childValue;

            if (parentNodeType == AST_FUNC_DEC || parentNodeType == AST_SETTER || parentNodeType == AST_GETTER) {
                // Potřeba ověřit počet návratových příkazů z funkce
                unsigned oldReturnCount = returnCounter;
                // Pro onelineblock u funkcí
                childValue              = recursiveNodeWalker(currentNode->child[0], AST_FUNC_DEC, 0);
                unsigned newReturnCount = returnCounter;

                if (newReturnCount - 1 > oldReturnCount) {
                    // Během zpracovávání funkce bylo nalezeno 2 a více návratových příkazů
                    // Nelze proto určit návratovou hodnotu funkce
                    childValue = UNKNOWN_DATA;
                }
            }

            else {
                childValue = recursiveNodeWalker(currentNode->child[0], AST_BLOCK, 0);
            }

            deleteScope();

            return childValue;
        }
        case (AST_STATEMENT_LIST): {
            printLog("AST_STATEMENT_LIST\n");

            SemValidationStates returnValue = OK;
            bool                foundReturn = false;

            SemValidationStates childValue;
            for (size_t i = 0; i < currentNode->arity; i++) {
                childValue = recursiveNodeWalker(currentNode->child[i], AST_STATEMENT_LIST, i);
                if (currentNode->child[i]->kind == AST_RETURN) {
                    returnCounter++;
                    foundReturn = true;
                    returnValue = childValue;
                }
                if (currentNode->child[i]->kind == AST_RETURN || currentNode->child[i]->kind == AST_BREAK ||
                    currentNode->child[i]->kind == AST_CONTINUE) {
                    // Kód po tomto dítěti lze smazat
                    i++;
                    size_t preDeletionArity = currentNode->arity;
                    for (size_t deletionIterator = i; deletionIterator < preDeletionArity; deletionIterator++) {
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
        }
        case (AST_FUNC_CALL): {
            printLog("AST_FUNC_CALL\n");

            SemSymbol newSymbol = {
                .symbolType = FUNCTION, .dataType = NULL_VAL, .returnValue = UNKNOWN_VAL, .scope = NEWEST_SCOPE};
            SemSymbDataType functionReturnValue = UNKNOWN_VAL;

            initNewSymbol(currentNode, &newSymbol);

            SemSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
            if (foundSymbol == NULL) {
                insertUndeclaredReference(newSymbol.name, newSymbol);
            } else {
                // Na stejný řetězec ukazuje AST strom
                functionReturnValue = foundSymbol->returnValue;
            }

            recursiveNodeWalker(currentNode->child[1], AST_FUNC_CALL, 0);

            return convSemSymbDTypeToSemValStates(functionReturnValue);
        }
        case (AST_SETTER): {
            printLog("AST_SETTER\n");

            SemSymbol newSymbol = {.symbolType = SETTER, .returnValue = NULL_VAL, .scope = NEWEST_SCOPE};

            SemSymbDataType setterStartValue = NULL_VAL;

            recursiveNodeWalker(currentNode->child[1], AST_SETTER, 1);

            SemValidationStates childValueBlock = recursiveNodeWalker(currentNode->child[2], AST_SETTER, 2);

            if (childValueBlock != OK) {
                setterStartValue = convSemValStatesToSemSymbDType(childValueBlock);
            }

            newSymbol.dataType = setterStartValue;
            initNewSymbol(currentNode->child[0], &newSymbol);

            SemSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
            if (foundSymbol != NULL) {
                EXEC_END(SEMANTICS_ERROR_REDEF, "Nastavovac jiz existuje!\n");
            }

            insertSymbolGlobalScope(newSymbol);

            removeUndeclaredReference(newSymbol.name);

            return OK;
        }
        case (AST_GETTER): {
            printLog("AST_GETTER\n");

            SemSymbol newSymbol = {.symbolType = GETTER, .dataType = NULL_VAL, .scope = NEWEST_SCOPE};

            SemSymbDataType getterReturnValue = NULL_VAL;

            SemValidationStates childValueBlock = recursiveNodeWalker(currentNode->child[2], AST_GETTER, 2);

            if (childValueBlock != OK) {
                getterReturnValue = convSemValStatesToSemSymbDType(childValueBlock);
            }

            newSymbol.returnValue = getterReturnValue;
            initNewSymbol(currentNode->child[0], &newSymbol);

            SemSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
            if (foundSymbol != NULL) {
                EXEC_END(SEMANTICS_ERROR_REDEF, "Ziskavac jiz existuje!\n");
            }

            insertSymbolGlobalScope(newSymbol);

            removeUndeclaredReference(newSymbol.name);

            return OK;
        }
        case (AST_VAR): {
            printLog("AST_VAR\n");

            SemSymbol newSymbol = {.symbolType = VARIABLE_DEFINITION, .returnValue = NULL_VAL, .scope = NEWEST_SCOPE};

            SemSymbDataType varDataType = NULL_VAL;

            if (currentNode->child[1]->kind != AST_EMPTY) {
                SemValidationStates childValue = recursiveNodeWalker(currentNode->child[1]->child[0], AST_VAR, 1);

                varDataType = convSemValStatesToSemSymbDType(childValue);
            }

            newSymbol.dataType = varDataType;
            initNewSymbol(currentNode, &newSymbol);

            SemSymbol* foundSymbol = findSymbolCurrentScope(newSymbol.name);
            if (foundSymbol != NULL) {
                EXEC_END(SEMANTICS_ERROR_REDEF, "Promena v danem ramci jiz existuje!\n");
            }

            insertSymbolCurrentScope(newSymbol);
            return OK;
        }
        case (AST_ASSIGN): {
            printLog("AST_ASSIGN\n");

            if (parentNodeType == AST_VAR || parentNodeType == AST_GLOBAL_ID) {
                return recursiveNodeWalker(currentNode->child[0], AST_ASSIGN, 0);
            }

            SemValidationStates leftChild  = recursiveNodeWalker(currentNode->child[0], AST_ASSIGN, 0);
            SemValidationStates rightChild = recursiveNodeWalker(currentNode->child[1], AST_ASSIGN, 1);

            if (leftChild != IDENTIFIER) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "(=) Levy operand prirazeni musi byt promenna!\n");
            }

            SemSymbol* leftSymbol = findSymbol(currentNode->child[0]->value.str);

            if (rightChild == IDENTIFIER) {
                SemSymbol* rightSymbol = findSymbol(currentNode->child[1]->value.str);

                if (leftSymbol == NULL) {
                    return OK;
                }

                if (rightSymbol == NULL) {
                    leftSymbol->dataType = UNKNOWN_VAL;
                    return OK;
                }

                // IMPORTANT Dokud nebude pořádná statická analýza, tak nelze přiřadit přesnou hodnotu (např kvůli
                // if/else blokům) leftSymbol->dataType = rightSymbol->dataType;
                leftSymbol->dataType = UNKNOWN_VAL;
            }

            if (leftSymbol != NULL) {
                // IMPORTANT Dokud nebude pořádná statická analýza, tak nelze přiřadit přesnou hodnotu (např kvůli
                // if/else blokům) leftSymbol->dataType = convSemValStatesToSemSymbDType(rightChild);
                leftSymbol->dataType = UNKNOWN_VAL;
            }
            return OK;
        }
        case (AST_IF_CHAIN): {
            printLog("AST_IF_CHAIN\n");

            for (size_t i = 0; i < currentNode->arity; i++) {
                recursiveNodeWalker(currentNode->child[i], AST_IF_CHAIN, i);
            }
            return OK;
        }
        case (AST_IF): {
            printLog("AST_IF\n");

            SemValidationStates conditionChildValue = recursiveNodeWalker(currentNode->child[0], AST_IF, 0);

            if (conditionChildValue != OK) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "(if) Neplatna podminka!\n");
            }

            // Sémantická kontrola bloku
            recursiveNodeWalker(currentNode->child[1], AST_IF, 1);

            return OK;
        }
        case (AST_CONDITION): {
            printLog("AST_CONDITION\n");

            if (parentNodeType == AST_FOR) {
                SemValidationStates leftChild = recursiveNodeWalker(currentNode->child[0], AST_CONDITION, 0);

                if (leftChild != IDENTIFIER) {
                    EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE, "(in) Leva strana musi byt identifikator!\n");
                }

                SemValidationStates rightChild = recursiveNodeWalker(currentNode->child[1], AST_CONDITION, 1);

                if (rightChild != OK) {
                    EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE, "(in) Prava strana neni v poradku!\n");
                }
                return OK;

            } else {
                SemValidationStates childValue = recursiveNodeWalker(currentNode->child[0], AST_CONDITION, 0);
                if (childValue == OK) {
                    EXEC_END(SEMANTICS_ERROR_OTHERS, "(condition) Neplatna podminka!\n");
                }
                return OK;
            }
        }
        case (AST_ELSE): {
            printLog("AST_ELSE\n");

            // Sémantická kontrola bloku
            recursiveNodeWalker(currentNode->child[0], AST_ELSE, 0);

            return OK;
        }
        case (AST_WHILE): {
            printLog("AST_WHILE\n");
            SemValidationStates conditionChildValue = recursiveNodeWalker(currentNode->child[0], AST_WHILE, 0);

            if (conditionChildValue != OK) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "(while) Neplatna podminka!\n");
            }

            // Sémantická kontrola bloku
            recursiveNodeWalker(currentNode->child[1], AST_WHILE, 1);

            return OK;
        }
        case (AST_FOR): {
            printLog("AST_FOR\n");

            // Zpřístupnění iterátoru pro podmínku a blok
            createNewScope();
            recursiveNodeWalker(currentNode->child[0]->child[0], AST_FOR, 0);

            SemValidationStates conditionChildValue = recursiveNodeWalker(currentNode->child[0], AST_FOR, 0);

            if (conditionChildValue != OK) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "(for) Neplatna podminka!\n");
            }

            // Sémantická kontrola bloku
            recursiveNodeWalker(currentNode->child[1], AST_FOR, 1);

            return OK;
        }
        case (AST_IDENTIFIER): {
            printLog("AST_IDENTIFIER\n");
            if (parentNodeType == AST_PARAM || parentNodeType == AST_FOR) {
                SemSymbol newSymbol = {
                    .symbolType = FUNCTION_PARAMETER, .returnValue = NULL_VAL, .scope = NEWEST_SCOPE};

                SemSymbDataType identifierDataType = UNKNOWN_VAL;

                if (parentNodeType == AST_FOR) {
                    identifierDataType = INT_VAL;
                }

                newSymbol.dataType = identifierDataType;
                initNewSymbol(currentNode, &newSymbol);

                SemSymbol* foundSymbol = findSymbolCurrentScope(newSymbol.name);

                if (foundSymbol != NULL) {
                    EXEC_END(SEMANTICS_ERROR_REDEF, "Promena v danem ramci jiz existuje!\n");
                }
                insertSymbolCurrentScope(newSymbol);
                if (parentNodeType == AST_FOR) {
                    return IDENTIFIER;
                }
                return OK;

            } else {
                SemSymbol tempSymbol;

                SemSymbol* foundSymbol = findSymbolCurrentScope(currentNode->value.str);

                if (foundSymbol == NULL) {
                    foundSymbol = findSymbolAndModifyAST(currentNode, &tempSymbol);
                }

                if (foundSymbol == NULL) {
                    // SETTER
                    if (parentNodeType == AST_ASSIGN && childNum == 0) {
                        SemSymbol newSymbol = {.symbolType  = SETTER,
                                               .dataType    = UNKNOWN_VAL,
                                               .returnValue = NULL_VAL,
                                               .scope       = NEWEST_SCOPE};

                        initNewSymbol(currentNode, &newSymbol);
                        currentNode->kind = AST_SETTER_CALL;

                        insertUndeclaredReference(newSymbol.name, newSymbol);
                        return IDENTIFIER;
                    }

                    // GETTER
                    SemSymbol newSymbol = {
                        .symbolType = GETTER, .dataType = NULL_VAL, .returnValue = UNKNOWN_VAL, .scope = NEWEST_SCOPE};
                    initNewSymbol(currentNode, &newSymbol);

                    insertUndeclaredReference(newSymbol.name, newSymbol);
                    currentNode->kind = AST_GETTER_CALL;

                    if (parentNodeType == AST_CONDITION) {
                        return IDENTIFIER;
                    }
                    return convSemSymbDTypeToSemValStates(UNKNOWN_VAL);
                }

                if (parentNodeType == AST_ASSIGN && childNum == 0) {
                    return IDENTIFIER;
                }

                if (parentNodeType == AST_CONDITION) {
                    return IDENTIFIER;
                }

                if (foundSymbol->symbolType == GETTER) {
                    return convSemSymbDTypeToSemValStates(foundSymbol->returnValue);
                } else {
                    return convSemSymbDTypeToSemValStates(foundSymbol->dataType);
                }
            }
        }
        case (AST_GLOBAL_ID): {
            printLog("AST_GLOBAL_ID\n");

            if (parentNodeType == AST_STATEMENT_LIST) {
                SemSymbol newSymbol = {.symbolType = GLOBAL, .returnValue = NULL_VAL, .scope = NEWEST_SCOPE};

                SemValidationStates childValue = recursiveNodeWalker(currentNode->child[1], AST_GLOBAL_ID, 1);

                SemSymbDataType varDataType = convSemValStatesToSemSymbDType(childValue);

                newSymbol.dataType = varDataType;
                initNewSymbol(currentNode, &newSymbol);

                SemSymbol* foundSymbol = findGlobalSymbol(newSymbol.name);
                if (foundSymbol != NULL) {
                    // Jedná se o přiřazení nového hodnoty již existující globální proměnné
                    foundSymbol->dataType = newSymbol.dataType;
                    return OK;
                }

                insertSymbolGlobalScope(newSymbol);
                return OK;
            } else {
                SemSymbol* foundSymbol = findGlobalSymbol(currentNode->value.str);

                if (foundSymbol == NULL) {
                    // Globální proměnné lze číst i bez definování, vrací hodnotu null
                    // return NULL_DATA;
                    // IMPORTANT, protože není statická sémantická kontrola je potřeba přetypovat.
                    return UNKNOWN_DATA;
                }

                if (parentNodeType == AST_ASSIGN) {
                    return IDENTIFIER;
                }
                return convSemSymbDTypeToSemValStates(foundSymbol->dataType);
            }
        }
        case (AST_INT_LIT): {
            printLog("AST_INT_LIT\n");
            return INT_DATA;
        }
        case (AST_FLOAT_LIT): {
            printLog("AST_FLOAT_LIT\n");
            return NUMBER_DATA;
        }
        case (AST_BOOL_LIT): {
            printLog("AST_BOOL_LIT\n");
            return BOOLEAN_DATA;
        }
        case (AST_STRING_LIT): {
            printLog("AST_STRING_LIT\n");
            return STRING_DATA;
        }
        case (AST_SMALL_NULL): {
            printLog("AST_SMALL_NULL\n");
            // return NULL_DATA;
            // IMPORTANT, protože není statická sémantická kontrola je potřeba přetypovat.
            return UNKNOWN_DATA;
        }
        case (AST_KEYWORD_NUM): {
            printLog("AST_KEYWORD_NUM\n");
            return DATATYPE_KEYWORD;
        }
        case (AST_KEYWORD_STRING): {
            printLog("AST_KEYWORD_STRING\n");
            return DATATYPE_KEYWORD;
        }
        case (AST_PLUS): {
            printLog("AST_PLUS\n");
            return handleMathOperators(currentNode, AST_PLUS, "+");
        }
        case (AST_MINUS): {
            printLog("AST_MINUS\n");
            // Unární mínus
            if (currentNode->arity == 1) {
                SemValidationStates onlyChild = recursiveNodeWalker(currentNode->child[0], AST_MINUS, 0);
                if (onlyChild == NUMBER_DATA)
                    return NUMBER_DATA;
                if (onlyChild == INT_DATA)
                    return INT_DATA;
                if (onlyChild == UNKNOWN_DATA)
                    return UNKNOWN_DATA;
                EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE, "(unarni -)  Nekompatibilni datovy typ!\n");
                break;
            } else // Binární mínus
                return handleMathOperators(currentNode, AST_MINUS, "-");
        }
        case (AST_MUL): {
            printLog("AST_MUL\n");
            return handleMathOperators(currentNode, AST_MUL, "*");
        }
        case (AST_DIV): {
            printLog("AST_DIV\n");
            return handleMathOperators(currentNode, AST_DIV, "/");
        }
        case (AST_TERNARY): {
            printLog("AST_TERNARY\n");

            SemValidationStates conditionChild = recursiveNodeWalker(currentNode->child[0], AST_TERNARY, 0);
            if (conditionChild == OK) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "(?:) Neplatna podminka!\n");
            }

            SemValidationStates trueChild = recursiveNodeWalker(currentNode->child[1], AST_TERNARY, 1);
            if (trueChild == OK) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "(?:) Neplatny vyraz pri platne podmince!\n");
            }

            SemValidationStates falseChild = recursiveNodeWalker(currentNode->child[2], AST_TERNARY, 2);
            if (falseChild == OK) {
                EXEC_END(SEMANTICS_ERROR_OTHERS, "(?:) Neplatny vyraz pri neplatne podmince!\n");
            }

            return UNKNOWN_DATA;
        }
        case (AST_EQ): {
            printLog("AST_EQ\n");

            recursiveNodeWalker(currentNode->child[0], AST_EQ, 0);
            recursiveNodeWalker(currentNode->child[1], AST_EQ, 1);

            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;
        }
        case (AST_NEQ): {
            printLog("AST_NEQ\n");

            recursiveNodeWalker(currentNode->child[0], AST_NEQ, 0);
            recursiveNodeWalker(currentNode->child[1], AST_NEQ, 1);

            // Porovnat lze cokoliv, ale výsledek bude vždy bool hodnota
            return BOOLEAN_DATA;
        }
        case (AST_LT): {
            printLog("AST_LT\n");
            return handleComparisonOperators(currentNode, AST_LT, "<");
        }
        case (AST_GT): {
            printLog("AST_GT\n");
            return handleComparisonOperators(currentNode, AST_GT, ">");
        }
        case (AST_GTE): {
            printLog("AST_GTE\n");
            return handleComparisonOperators(currentNode, AST_GTE, ">=");
        }
        case (AST_LTE): {
            printLog("AST_LTE\n");
            return handleComparisonOperators(currentNode, AST_LTE, "<=");
        }
        case (AST_NOT): {
            printLog("AST_NOT\n");

            recursiveNodeWalker(currentNode->child[0], AST_NOT,
                                0);  // Negace má pouze jedno dítě
            // Vstup může být cokoliv, ale výstup je vždy bool hodnota
            return BOOLEAN_DATA;
        }
        case (AST_AND): {
            printLog("AST_AND\n");

            recursiveNodeWalker(currentNode->child[0], AST_AND, 0);
            recursiveNodeWalker(currentNode->child[1], AST_AND, 1);

            // Nelze říct, co bude návratová hodnota
            return UNKNOWN_DATA;
        }
        case (AST_OR): {
            printLog("AST_OR\n");

            recursiveNodeWalker(currentNode->child[0], AST_OR, 0);
            recursiveNodeWalker(currentNode->child[1], AST_OR, 1);

            // Nelze říct, co bude návratová hodnota
            return UNKNOWN_DATA;
        }
        case (AST_IS): {
            printLog("AST_IS\n");

            SemValidationStates leftChild  = recursiveNodeWalker(currentNode->child[0], AST_IS, 0);
            SemValidationStates rightChild = recursiveNodeWalker(currentNode->child[1], AST_IS, 1);

            if (leftChild == OK || leftChild == IDENTIFIER) {
                EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE,
                         "(is) Leva strana operatoru musi mit jako vysledek datovy typ!\n");
            }

            if (rightChild != DATATYPE_KEYWORD) {
                EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE, "(is) Prava strana operatoru musi byt nazev datoveho typu!\n");
            }

            return BOOLEAN_DATA;
        }
        case (AST_DOUBLE_DOT): {
            printLog("AST_DOUBLE_DOT\n");

            SemValidationStates leftChild  = recursiveNodeWalker(currentNode->child[0], AST_DOUBLE_DOT, 0);
            SemValidationStates rightChild = recursiveNodeWalker(currentNode->child[1], AST_DOUBLE_DOT, 1);

            if ((leftChild == NUMBER_DATA || leftChild == INT_DATA || leftChild == UNKNOWN_DATA) &&
                (rightChild == NUMBER_DATA || rightChild == INT_DATA || rightChild == UNKNOWN_DATA)) {
                return OK;
            }
            EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE, "(..) Operandy musi byt cela cisla!\n");
            break;
        }
        case (AST_TRIPLE_DOT): {
            printLog("AST_TRIPLE_DOT\n");

            SemValidationStates leftChild  = recursiveNodeWalker(currentNode->child[0], AST_TRIPLE_DOT, 0);
            SemValidationStates rightChild = recursiveNodeWalker(currentNode->child[1], AST_TRIPLE_DOT, 1);

            if ((leftChild == NUMBER_DATA || leftChild == INT_DATA || leftChild == UNKNOWN_DATA) &&
                (rightChild == NUMBER_DATA || rightChild == INT_DATA || rightChild == UNKNOWN_DATA)) {
                return OK;
            }
            EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE, "(...) Operandy musi byt cela cisla!\n");
            break;
        }
        case (AST_RETURN): {
            printLog("AST_RETURN\n");

            // SemValidationStates returnDataType = NULL_DATA;
            // IMPORTANT, protože není statická sémantická kontrola je potřeba přetypovat.
            SemValidationStates returnDataType = UNKNOWN_DATA;

            if (currentNode->child[0]->kind != AST_EMPTY) {
                SemValidationStates childValue = recursiveNodeWalker(currentNode->child[0], AST_RETURN, 0);

                if (returnDataType != OK)
                    returnDataType = childValue;
            }

            return returnDataType;
        }
        case (AST_IFJ_CALL): {
            printLog("AST_IFJ_CALL\n");
            return handleBuildinFunctions(currentNode);
        }
        case (AST_EMPTY): {
            printLog("AST_EMPTY\n");
            // Pomocný uzel u parametrů funkce
            return OK;
        }
        case (AST_NULL): {
            printLog("AST_NULL\n");
            if (parentNodeType == AST_IS) {
                // Značí datový typ Null
                return DATATYPE_KEYWORD;
            }
            // Značí prázdný jednořádkový blok (jeho návratovou hodnotu)
            // return NULL_DATA;

            // IMPORTANT, protože není statická sémantická kontrola je potřeba přetypovat.
            return UNKNOWN_DATA;
        }
        case (AST_BREAK): {
            printLog("AST_BREAK\n");
            return OK;
        }
        case (AST_CONTINUE): {
            printLog("AST_CONTINUE\n");
            return OK;
        }
        case (AST_LPAREN): {
            printLog("AST_LPAREN\n");
            // Tento AST uzel bych neměl dostat
            EXEC_END(INTERNAL_ERROR, "Uzel AST_LPAREN neni validni uzel stromu!\n");
            break;
        }
        case (AST_RPAREN): {
            printLog("AST_RPAREN\n");
            // Tento AST uzel bych neměl dostat
            EXEC_END(INTERNAL_ERROR, "Uzel AST_RPAREN neni validni uzel stromu!\n");
            break;
        }
        case (AST_SETTER_CALL): {
            printLog("AST_SETTER_CALL\n");
            // Tento AST uzel bych neměl dostat
            EXEC_END(INTERNAL_ERROR, "Uzel AST_SETTER_CALL neni validni uzel stromu!\n");
            break;
        }
        case (AST_GETTER_CALL): {
            printLog("AST_GETTER_CALL\n");
            // Tento AST uzel bych neměl dostat
            EXEC_END(INTERNAL_ERROR, "Uzel AST_GETTER_CALL neni validni uzel stromu!\n");
            break;
        }
        case (AST_FLOAT_LIT_EXP): {
            printLog("AST_FLOAT_LIT_EXP\n");
            // Tento AST uzel bych neměl dostat
            EXEC_END(INTERNAL_ERROR, "Uzel AST_FLOAT_LIT_EXP neni validni uzel stromu!\n");
            break;
        }
        case (AST_INT_LIT_EXP): {
            printLog("AST_INT_LIT_EXP\n");
            // Tento AST uzel bych neměl dostat
            EXEC_END(INTERNAL_ERROR, "Uzel AST_INT_LIT_EXP neni validni uzel stromu!\n");
            break;
        }
    }

    printWarning("(%s) Neznamy stav: %i!\n", __func__, currentNode->kind);
    return GOT_NODE_NULL;
}

void createScopeStack() {
    DynamicHashTablePtrStackPtr stack = stackHashTablePtrConstruct(DEFAULT_INIT_ELEMENT_COUNT);
    if (stack == NULL) {
        EXEC_END(INTERNAL_ERROR, "Selhalo vytvareni zasobniku!\n");
        return;
    }

    scopesStack = stack;
}

void createNewScope() {
    HashTable* newScope = hashTableInit();
    if (newScope == NULL) {
        EXEC_END(INTERNAL_ERROR, "Selhalo vytvareni ramce!\n");
        return;
    }

    hashTableInitItems(newScope);

    stackHashTablePtrPush(scopesStack, newScope);
}

void deleteScope() {
    if (scopesStack == NULL) {
        EXEC_END(INTERNAL_ERROR, "Zasobnik neexistuje!\n");
        return;
    }
    if (stackHashTablePtrIsEmpty(scopesStack)) {
        EXEC_END(INTERNAL_ERROR, "Snaha odstranit prvek z prazdneho zasobniku!\n");
        return;
    }

    HashTable* newScope = stackHashTablePtrRead(scopesStack);
    if (newScope == NULL) {
        EXEC_END(INTERNAL_ERROR, "Na zasobniku byl prazdny ukazatel!\n");
        return;
    }

    hashTableDealloc(newScope);

    stackHashTablePtrPop(scopesStack);
}

SemSymbol* findSymbolAndModifyAST(AstNode* sourceNode, SemSymbol* destSymbol) {
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
        unsigned symbolNameLength = strlen(key) + 3;  // prostor pro '$_'

        char getterName[symbolNameLength];
        snprintf(getterName, symbolNameLength, "%s$%s", key, "g");
        SemSymbol* foundGetter = findGlobalSymbol(getterName);
        if (foundGetter != NULL) {
            sourceNode->kind = AST_GETTER_CALL;
            return foundGetter;
        }

        char setterName[symbolNameLength];
        snprintf(setterName, symbolNameLength, "%s$%s", key, "s");
        SemSymbol* foundSetter = findGlobalSymbol(setterName);
        if (foundSetter != NULL) {
            sourceNode->kind = AST_SETTER_CALL;
        }

        return NULL;
    }

    SemSymbol* foundSymbol = &(scopesStack->array[toSearchScopeIndex]->items[index].symbol);

    if (destSymbol != NULL) {
        // Potřeba přejmenovat symbol v AST stromě
        destSymbol->symbolType  = VARIABLE;
        destSymbol->dataType    = foundSymbol->dataType;
        destSymbol->returnValue = foundSymbol->returnValue;
        destSymbol->scope       = toSearchScopeIndex;
        initNewSymbol(sourceNode, destSymbol);
    }
    return foundSymbol;
}

SemSymbol* findSymbol(char* key) {
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

SemSymbol* findGlobalSymbol(char* key) {
    int index = hashTableSearchItem(scopesStack->array[0], key);

    if (index == -1) {
        return NULL;
    }

    return &(scopesStack->array[0]->items[index].symbol);
}

SemSymbol* findSymbolCurrentScope(char* key) {
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

void insertSymbolGlobalScope(SemSymbol symbol) {
    if (scopesStack == NULL) {
        EXEC_END(INTERNAL_ERROR, "Neexistuje zasobnik!\n");
        return;
    }

    if (stackHashTablePtrIsEmpty(scopesStack) == true) {
        EXEC_END(INTERNAL_ERROR, "Neni inicializovan zadny ramec!\n");
        return;
    }

    int index = hashTableInsertItem(scopesStack->array[0], symbol.name, symbol);

    if (index == -1) {
        EXEC_END(INTERNAL_ERROR, "Selhalo vlozeni prvku do globalniho ramce!\n");
        return;
    }
}

void insertSymbolCurrentScope(SemSymbol symbol) {
    if (scopesStack == NULL) {
        EXEC_END(INTERNAL_ERROR, "Neexistuje zasobnik!\n");
        return;
    }

    if (stackHashTablePtrIsEmpty(scopesStack) == true) {
        EXEC_END(INTERNAL_ERROR, "Neni inicializovan zadny ramec!\n");
        return;
    }

    HashTable* currentScope = stackHashTablePtrRead(scopesStack);

    int index = hashTableInsertItem(currentScope, symbol.name, symbol);

    if (index == -1) {
        EXEC_END(INTERNAL_ERROR, "Selhalo vlozeni prvku do ramce!\n");
    };
}

void createUndeclaredFunctionCallsTable() {
    undeclaredReferenceTable = hashTableInit();
    if (undeclaredReferenceTable == NULL) {
        EXEC_END(INTERNAL_ERROR, "Selhalo vytvoreni tabulky nedeklarovanych referenci!\n");
    }
}

void removeUndeclaredReference(char* referenceSymbolName) {
    if (undeclaredReferenceTable == NULL) {
        EXEC_END(INTERNAL_ERROR, "Tabulka pro nedeklarovane reference neexistuje!\n");
        return;
    }

    if (hashTableSearchItem(undeclaredReferenceTable, referenceSymbolName) == -1) {
        return;
    }

    hashTableDeleteItem(undeclaredReferenceTable, referenceSymbolName);
}

void insertUndeclaredReference(char* referenceSymbolName, SemSymbol symbol) {
    if (undeclaredReferenceTable == NULL) {
        EXEC_END(INTERNAL_ERROR, "Tabulka pro nedeklarovane reference neexistuje!\n");
        return;
    }

    if (hashTableInsertItem(undeclaredReferenceTable, referenceSymbolName, symbol) == -1) {
        EXEC_END(INTERNAL_ERROR, "Nepodarilo se vlozit prvek do tabulky!\n");
    }
}

void initNewSymbol(AstNode* sourceNode, SemSymbol* destSymbol) {
    // TODO symbol dataType a symbolType a functionReturnValue a scope můžu nastavit před voláním
    if (sourceNode == NULL || destSymbol == NULL) {
        EXEC_END(INTERNAL_ERROR, "Neplatne hodnoty!\n");
        return;
    }

    switch (destSymbol->symbolType) {
        case (FUNCTION): {
            unsigned newNameLength = strlen(sourceNode->child[0]->value.str) + 1 +
                                     digitsCount(sourceNode->child[1]->arity);  // prostor pro separátor '$'

            char* newName = malloc(sizeof(char) * newNameLength);

            size_t numberOfParams = sourceNode->child[1]->arity;
            numberOfParams--;  // Každá deklarace i volání funkce obsahuje v parametrech 1
                               // AST_EMTPY

            if (newName == NULL) {
                EXEC_END(INTERNAL_ERROR, "Selhala alokace pameti!");
                return;
            }

            destSymbol->name = newName;

            // functionName$parametrCount
            snprintf(destSymbol->name, newNameLength, "%s$%lu", sourceNode->child[0]->value.str, numberOfParams);

            destSymbol->paramCount = sourceNode->child[1]->arity;

            free(sourceNode->child[0]->value.str);
            sourceNode->child[0]->value.str = newName;
            break;
        }
        case (VARIABLE_DEFINITION): {
            unsigned newNameLength = strlen(sourceNode->child[0]->value.str) + 2 +
                                     digitsCount(destSymbol->scope);  // prostor pro separátor '$$'

            char* newName = malloc(sizeof(char) * newNameLength);

            if (newName == NULL) {
                EXEC_END(INTERNAL_ERROR, "Selhala alokace pameti!");
                return;
            }

            destSymbol->name = newName;

            snprintf(destSymbol->name, newNameLength, "%s$$%u", sourceNode->child[0]->value.str, destSymbol->scope);

            free(sourceNode->child[0]->value.str);
            sourceNode->child[0]->value.str = newName;
            break;
        }
        case (VARIABLE):
        case (FUNCTION_PARAMETER): {
            unsigned newNameLength =
                strlen(sourceNode->value.str) + 2 + digitsCount(destSymbol->scope);  // prostor pro separátor '$$'

            char* newName = malloc(sizeof(char) * newNameLength);

            if (newName == NULL) {
                EXEC_END(INTERNAL_ERROR, "Selhala alokace pameti!");
                return;
            }

            destSymbol->name = newName;

            // variableName$$scopeNumber ($$ pro rozlišení mezi funkcí a lokální proměnnou)
            snprintf(destSymbol->name, newNameLength, "%s$$%u", sourceNode->value.str, destSymbol->scope);

            free(sourceNode->value.str);
            sourceNode->value.str = newName;
            break;
        }
        case (GLOBAL): {
            destSymbol->name = sourceNode->child[0]->value.str;
            break;
        }
        case (SETTER): {
            unsigned newNameLength = strlen(sourceNode->value.str) + 3;  // prostor pro postfix '$s'

            char* newName = malloc(sizeof(char) * newNameLength);

            if (newName == NULL) {
                EXEC_END(INTERNAL_ERROR, "Selhala alokace pameti!");
                return;
            }

            destSymbol->name = newName;

            // setterName$s ($s pro rozlišení mezi nastavovačem a získávačem)
            snprintf(destSymbol->name, newNameLength, "%s$%s", sourceNode->value.str, "s");

            free(sourceNode->value.str);
            sourceNode->value.str = newName;
            break;
        }
        case (GETTER): {
            unsigned newNameLength = strlen(sourceNode->value.str) + 3;  // prostor pro postfix '$g'

            char* newName = malloc(sizeof(char) * newNameLength);

            if (newName == NULL) {
                EXEC_END(INTERNAL_ERROR, "Selhala alokace pameti!");
                return;
            }

            destSymbol->name = newName;

            // getterName$g ($s pro rozlišení mezi nastavovačem a získávačem)
            snprintf(destSymbol->name, newNameLength, "%s$%s", sourceNode->value.str, "g");

            free(sourceNode->value.str);
            sourceNode->value.str = newName;
            break;
        }
        default: {
            EXEC_END(INTERNAL_ERROR, "Neplatny typ symbolu");
            return;
        }
    }

    return;
}

SemValidationStates handleBuildinFunctions(AstNode* functionCallNode) {
    char* functionName = functionCallNode->child[0]->value.str;

    if (strncmp(functionName, "read_str", 9) == 0) {
        printLog("read_str\n");

        if (functionCallNode->child[1]->arity != 1) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.read_str) Nevhodny pocet parametru!\n");
        }

        return STRING_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "read_num", 9) == 0) {
        printLog("read_num\n");

        if (functionCallNode->child[1]->arity != 1) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.read_num) Nevhodny pocet parametru!\n");
        }

        return NUMBER_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "read_bool", 10) == 0) {
        printLog("read_bool\n");

        if (functionCallNode->child[1]->arity != 1) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.read_bool) Nevhodny pocet parametru!\n");
        }

        return BOOLEAN_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "write", 6) == 0) {
        printLog("write\n");

        if (functionCallNode->child[1]->arity != 2) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.write) Nevhodny pocet parametru!\n");
        }

        recursiveNodeWalker(functionCallNode->child[1], AST_IFJ_CALL, 1);

        // return NULL_DATA;
        // IMPORTANT, protože není statická sémantická kontrola je potřeba přetypovat.
        return UNKNOWN_DATA;
    }

    if (strncmp(functionName, "floor", 6) == 0) {
        printLog("floor\n");

        if (functionCallNode->child[1]->arity != 2) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.floor) Nevhodny pocet parametru!\n");
        }

        SemValidationStates onlyChild = recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (onlyChild == NUMBER_DATA || onlyChild == INT_DATA || onlyChild == UNKNOWN_DATA) {
            return INT_DATA;
        }

        EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.floor) Nevhodny parametr!\n");
    }

    if (strncmp(functionName, "str", 4) == 0) {
        printLog("str\n");

        if (functionCallNode->child[1]->arity != 2) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.str) Nevhodny pocet parametru!\n");
        }

        recursiveNodeWalker(functionCallNode->child[1], AST_IFJ_CALL, 1);

        return STRING_DATA;
    }

    if (strncmp(functionName, "length", 7) == 0) {
        printLog("length\n");

        if (functionCallNode->child[1]->arity != 2) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.length) Nevhodny pocet parametru!\n");
        }

        SemValidationStates onlyChild = recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (onlyChild == STRING_DATA || onlyChild == UNKNOWN_DATA) {
            return INT_DATA;
        }

        EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.length) Nevhodny parametr!\n");
    }

    if (strncmp(functionName, "substring", 10) == 0) {
        printLog("substring\n");

        if (functionCallNode->child[1]->arity != 4) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.substring) Nevhodny pocet parametru!\n");
        }

        SemValidationStates firstChild = recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny prvni parametr!\n");
        }

        SemValidationStates secondChild = recursiveNodeWalker(functionCallNode->child[1]->child[1], AST_IFJ_CALL, 1);
        if (secondChild != NUMBER_DATA && secondChild != INT_DATA && secondChild != UNKNOWN_DATA) {
            EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny druhy parametr!\n");
        }

        SemValidationStates thirdChild = recursiveNodeWalker(functionCallNode->child[1]->child[2], AST_IFJ_CALL, 2);
        if (thirdChild != NUMBER_DATA && thirdChild != INT_DATA && thirdChild != UNKNOWN_DATA) {
            EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.substring) Nevhodny treti parametr!\n");
        }

        return STRING_DATA;  // Předpokládám, že funkce nevrátí chybový datový typ
    }

    if (strncmp(functionName, "strcmp", 7) == 0) {
        printLog("strcmp\n");

        if (functionCallNode->child[1]->arity != 3) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.strcmp) Nevhodny pocet parametru!\n");
        }

        SemValidationStates firstChild = recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.strcmp) Nevhodny prvni parametr!\n");
        }

        SemValidationStates secondChild = recursiveNodeWalker(functionCallNode->child[1]->child[1], AST_IFJ_CALL, 1);
        if (secondChild != STRING_DATA && secondChild != UNKNOWN_DATA) {
            EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.strcmp) Nevhodny druhy parametr!\n");
        }

        return INT_DATA;
    }

    if (strncmp(functionName, "ord", 4) == 0) {
        printLog("ord\n");

        if (functionCallNode->child[1]->arity != 3) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.ord) Nevhodny pocet parametru!\n");
        }

        SemValidationStates firstChild = recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (firstChild != STRING_DATA && firstChild != UNKNOWN_DATA) {
            EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.ord) Nevhodny prvni parametr!\n");
        }

        SemValidationStates secondChild = recursiveNodeWalker(functionCallNode->child[1]->child[1], AST_IFJ_CALL, 1);
        if (secondChild != NUMBER_DATA && secondChild != INT_DATA && secondChild != UNKNOWN_DATA) {
            EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.ord) Nevhodny druhy parametr!\n");
        }

        return INT_DATA;
    }

    if (strncmp(functionName, "chr", 4) == 0) {
        printLog("chr\n");

        if (functionCallNode->child[1]->arity != 2) {
            EXEC_END(SEMANTICS_ERROR_UNEXPECTED, "(Ifj.chr) Nevhodny pocet parametru!\n");
        }

        SemValidationStates firstChild = recursiveNodeWalker(functionCallNode->child[1]->child[0], AST_IFJ_CALL, 0);
        if (firstChild != NUMBER_DATA && firstChild != INT_DATA && firstChild != UNKNOWN_DATA) {
            EXEC_END(SEMANTICS_ERROR_INCORRECT, "(Ifj.char) Nevhodny parametr!\n");
        }

        return STRING_DATA;
    }

    EXEC_END(SEMANTICS_ERROR_UNDEF, "Neznama vestavena funkce: '%s'!\n");
    return GOT_NODE_NULL;
}

SemValidationStates handleComparisonOperators(AstNode* currentNode, AstNodeType operatorCode, char* operatorName) {
    SemValidationStates leftChild  = recursiveNodeWalker(currentNode->child[0], operatorCode, 0);
    SemValidationStates rightChild = recursiveNodeWalker(currentNode->child[1], operatorCode, 1);

    if (leftChild == NUMBER_DATA) {
        if (rightChild == NUMBER_DATA)
            return BOOLEAN_DATA;
        if (rightChild == INT_DATA)
            return BOOLEAN_DATA;
        if (rightChild == UNKNOWN_DATA)
            return BOOLEAN_DATA;
    }

    if (leftChild == INT_DATA) {
        if (rightChild == NUMBER_DATA)
            return BOOLEAN_DATA;
        if (rightChild == INT_DATA)
            return BOOLEAN_DATA;
        if (rightChild == UNKNOWN_DATA)
            return BOOLEAN_DATA;
    }

    if (leftChild == UNKNOWN_DATA) {
        if (rightChild == NUMBER_DATA)
            return BOOLEAN_DATA;
        if (rightChild == INT_DATA)
            return BOOLEAN_DATA;
        if (rightChild == UNKNOWN_DATA)
            return BOOLEAN_DATA;
    }

    printLog("%i %s %i\n", leftChild, operatorName, rightChild);
    size_t errorMessageLength = strlen("(__) Nekompatibilni datove typy!\n") + 1;
    char   errorMessage[errorMessageLength];
    snprintf(errorMessage, errorMessageLength, "(%s) Nekompatibilni datove typy!\n", operatorName);
    EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE, errorMessage);
    return GOT_NODE_NULL;
}

SemValidationStates handleMathOperators(AstNode* currentNode, AstNodeType operatorCode, char* operatorName) {
    SemValidationStates leftChild  = recursiveNodeWalker(currentNode->child[0], operatorCode, 0);
    SemValidationStates rightChild = recursiveNodeWalker(currentNode->child[1], operatorCode, 1);

    if (leftChild == NUMBER_DATA) {
        if (rightChild == NUMBER_DATA)
            return NUMBER_DATA;
        if (rightChild == INT_DATA)
            return NUMBER_DATA;
        if (rightChild == UNKNOWN_DATA)
            return NUMBER_DATA;
    }

    if (leftChild == INT_DATA) {
        if (rightChild == NUMBER_DATA)
            return NUMBER_DATA;
        if (rightChild == INT_DATA)
            return operatorCode == AST_DIV ? NUMBER_DATA : INT_DATA;
        if (rightChild == UNKNOWN_DATA)
            return UNKNOWN_DATA;
    }

    if (leftChild == UNKNOWN_DATA) {
        if (rightChild == NUMBER_DATA)
            return UNKNOWN_DATA;
        if (rightChild == INT_DATA)
            return UNKNOWN_DATA;
        if (rightChild == UNKNOWN_DATA)
            return UNKNOWN_DATA;
        if (rightChild == STRING_DATA) {
            if (operatorCode == AST_PLUS) {
                return STRING_DATA;
            }
        }
    }

    if (leftChild == STRING_DATA) {
        if (operatorCode == AST_PLUS) {
            if (rightChild == STRING_DATA)
                return STRING_DATA;
            if (rightChild == UNKNOWN_DATA)
                return STRING_DATA;
        }

        if (operatorCode == AST_MUL) {
            if (rightChild == INT_DATA)
                return STRING_DATA;
            if (rightChild == UNKNOWN_DATA) {
                return STRING_DATA;
            }
        }
    }

    printLog("%i %s %i\n", leftChild, operatorName, rightChild);
    size_t errorMessageLength = strlen("(_) Nekompatibilni datove typy!\n") + 1;
    char   errorMessage[errorMessageLength];
    snprintf(errorMessage, errorMessageLength, "(%s) Nekompatibilni datove typy!\n", operatorName);
    EXEC_END(SEMANTICS_ERROR_UNCOMPATIBLE, errorMessage);
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

SemSymbDataType convSemValStatesToSemSymbDType(SemValidationStates returnedValue) {
    switch (returnedValue) {
        case (NUMBER_DATA): {
            return NUMBER_VAL;
        }
        case (INT_DATA): {
            return INT_VAL;
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

SemValidationStates convSemSymbDTypeToSemValStates(SemSymbDataType dataType) {
    switch (dataType) {
        case (NUMBER_VAL): {
            return NUMBER_DATA;
        }
        case (INT_VAL): {
            return INT_DATA;
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
