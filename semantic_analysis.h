// TODO hlavička

#ifndef SEMANTIC_ANALYSIS
#define SEMANTIC_ANALYSIS

#include <stddef.h>

#include "ast_strom.h"

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
     * @brief Definice nebo volání nedefinované funkce
     *
     */
    FUNCTION,
    /**
     * @brief Parametr u definice funkce
     *
     */
    FUNCTION_PARAMETER,
    /**
     * @brief // Definice nastavovače
     *
     */
    SETTER,
    /**
     * @brief // Definice získávače
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
     * @brief Hodnota je (pravděpodobně desetinné) číslo
     *
     */
    NUMBER_VAL,
    /**
     * @brief Celé číslo
     *
     */
    INT_VAL,
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
     * @note Používán jako základní hodnota (např. při deklaraci funkce)
     */
    NULL_VAL,
    /**
     * @brief Nelze určit hodnotu (parametry a návratová hodnota funkcí)
     *
     * @note Všechny operátory berou tento typ jako validní
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
     * @brief Výstupem zpracování uzlu je (pravděpodobně desetinné) číslo
     *
     */
    NUMBER_DATA,
    /**
     * @brief Výstupem zpracování uzlu je celé číslo
     *
     */
    INT_DATA,
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
     * @note Všechny operátory berou tuto hodnotu jako validní, ale snaží se ji nevracet (domýšlí si
     * na základě sémantických pravidel nebo druhého operandu vhodnou návratovou hodnotu)
     */
    UNKNOWN_DATA,
    /**
     * @brief Zpracovaný uzel je identifikátor
     *
     * @note Nerozlišuje jestli lokální nebo globální
     * @note Existenci operátoru si ověřuje zpracovaný uzel
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
 * @return true Sémantická kontrola je v pořádku
 * @return false Byla nalezena sémantická chyba
 */
bool startSematicValidation(AstNode* root);

/**
 * @brief Uklidí paměť používanou sémantickou analýzou
 *
 */
void freeSemanticsMemory();

#endif
// TODO patička