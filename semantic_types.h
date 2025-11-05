// TODO hlavička

#ifndef SEMANTIC_TYPES
#define SEMANTIC_TYPES 1

#include <stddef.h>

/**
 * @brief Číselník typů sémantických symbolů
 *
 */
typedef enum {
    /**
     * @brief Deklarace lokální proměnné
     *
     */
    VARIABLE,
    /**
     * @brief Deklarace globální proměnné
     *
     */
    GLOBAL,
    /**
     * @brief Definice nebo volání nedeklarované funkce
     *
     */
    FUNCTION,
    /**
     * @brief // TODO
     *
     */
    SETTER,
    /**
     * @brief // TODO
     *
     */
    GETTER,
    /**
     * @brief Parametr u deklarace funkce
     *
     */
    FUNCTION_PARAMETER
} SemanticSymbolTypes;

/**
 * @brief Číselník datových typů symbolů
 *
 */
typedef enum {
    A,
    /**
     * @brief Celé nebo desetinné číslo
     *
     */
    NUMBER_VAL,
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
} SemanticSymbolDataType;

/**
 * @brief Číselník návratových hodnot při rekurzivním průchodu přes AST
 *
 */
typedef enum {
    /**
     * @brief Uzel (a jeho potomci) je sémanticky v pořádku
     *
     */
    OK,
    /**
     * @brief V uzlu (nebo jeho potomcích) je sémantická chyba
     *
     */
    ERROR,
    /**
     * @brief Výstupem zpracování uzlu (a jeho potomků) je číslo
     *
     */
    NUMBER_DATA,
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
     * @brief Funkce dostala ukazatel na NULL jako vstupní hodnotu
     *
     * Zda se jedná o chybu rozhodne volající funkce
     */
    GOT_NODE_NULL,
} RecursiveSemanticAnalysisReturnValues;

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
    SemanticSymbolTypes symbolType;
    /**
     * @brief Příslušný rámec symbolu
     *
     */
    unsigned scope;
    /**
     * @brief Datový typ dat, která symbol obsahuje
     *
     */
    SemanticSymbolDataType dataType;
    /**
     * @brief Počet parametrů
     *
     */
    size_t paramCount;
    /**
     * @brief Návratová hodnota funkce
     * 
     */
    SemanticSymbolDataType returnValue;
} SemanticSymbol;

#endif
// TODO patička