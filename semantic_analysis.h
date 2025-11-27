/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Hlavička pro implementaci sémantické analýzy
 * @file semantic_analysis.h
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#ifndef SEMANTIC_ANALYSIS
#define SEMANTIC_ANALYSIS

#include <stddef.h>

#include "ast.h"

/**
 * @brief Číselník typů sémantických symbolů
 *
 */
typedef enum {
    /**
     * @brief Definice lokální proměnné
     *
     */
    VARIABLE_DEFINITION,
    /**
     * @brief Práce s hodnotou lokální proměnné
     *
     */
    VARIABLE,
    /**
     * @brief Definice nebo práce s hodnotou globální proměnné
     *
     */
    GLOBAL,
    /**
     * @brief Definice nebo volání funkce
     *
     */
    FUNCTION,
    /**
     * @brief Parametr u definice funkce / proměnná u for cyklu
     *
     */
    FUNCTION_PARAMETER,
    /**
     * @brief Definice nastavovače
     *
     */
    SETTER,
    /**
     * @brief Definice získávače
     *
     */
    GETTER,
} SemSymbolTypes;

/**
 * @brief Číselník datových typů symbolů
 *
 */
typedef enum {
    /**
     * @brief Hodnota je číslo
     *
     */
    NUMBER_VAL,
    /**
     * @brief Celé číslo
     *
     */
    INT_VAL,
    /**
     * @brief Desetinné číslo
     *
     */
    FLOAT_VAL,
    /**
     * @brief Řetězec
     *
     */
    STRING_VAL,
    /**
     * @brief Bool hodnota
     *
     */
    BOOLEAN_VAL,
    /**
     * @brief Nemá hodnotu (např při vytvoření proměnné bez přiřazení)
     *
     * @note Používán jako základní hodnota (například při deklaraci funkce)
     */
    NULL_VAL,
    /**
     * @brief Nelze určit hodnotu (parametry a návratová hodnota funkcí)
     *
     * @note Všechny operátory berou tento typ jako platný
     */
    UNKNOWN_VAL
} SemSymbDataType;

/**
 * @brief Číselník návratových hodnot při rekurzivním průchodu přes AST
 *
 */
typedef enum {
    /**
     * @brief Zpracování uzlu proběhlo v pořádku
     *
     */
    OK,
    /**
     * @brief Výstupem zpracování uzlu je číslo
     *
     */
    NUMBER_DATA,
    /**
     * @brief Výstupem zpracování uzlu je celé číslo
     *
     */
    INT_DATA,
    /**
     * @brief Výstupem je desetinné číslo
     *
     */
    FLOAT_DATA,
    /**
     * @brief Výstupem zpracování uzlu (a jeho potomků) je řetězec
     *
     */
    STRING_DATA,
    /**
     * @brief Výstupem zpracování uzlu je bool hodnota
     *
     */
    BOOLEAN_DATA,
    /**
     * @brief Výstupem zpracování uzlu (a jeho potomků) je hodnota null
     *
     */
    NULL_DATA,
    /**
     * @brief Nelze určit hodnotu (parametry a návratová hodnota funkcí)
     *
     * @note Všechny operátory berou tuto hodnotu jako validní, ale snaží se ji nevracet
     */
    UNKNOWN_DATA,
    /**
     * @brief Zpracovaný uzel je identifikátor
     *
     * @note Nerozlišuje zda je lokální, globální, nastavovač nebo získávač
     * @note Existenci operandu si ověřuje zpracovaný uzel
     */
    IDENTIFIER,
    /**
     * @brief zpracovaný uzel je klíčové slovo pro datový typ
     *
     */
    DATATYPE_KEYWORD,
    /**
     * @brief Funkce dostala ukazatel na NULL jako vstupní hodnotu
     *
     * Zda se jedná o chybu rozhodne volající funkce
     */
    GOT_NODE_NULL,
} SemValidationStates;

/**
 * @brief Struktura pro uložení symbolu do tabulky symbolů
 *
 * V závislosti na symbolType nemusí být některé hodnoty relevantní
 */
typedef struct {
    /**
     * @brief Název symbolu
     *
     */
    char* name;
    /**
     * @brief Typ symbolu
     *
     */
    SemSymbolTypes symbolType;
    /**
     * @brief Příslušný rámec symbolu
     *
     */
    unsigned scope;
    /**
     * @brief Datový typ dat, která symbol obsahuje
     *
     */
    SemSymbDataType dataType;
    /**
     * @brief Počet parametrů
     *
     */
    size_t paramCount;
    /**
     * @brief Návratová hodnota funkce
     *
     */
    SemSymbDataType returnValue;
} SemSymbol;

/**
 * @brief Funkce pro zahájení sémantické kontroly
 *
 * @param root Vrchol abstraktního syntaktického stromu
 */
void startSematicValidation(AstNode* root);

/**
 * @brief Uklidí paměť používanou sémantickou analýzou
 *
 */
void freeSemanticsMemory();

#endif

/*** konec semantic_analysis.h ***/
