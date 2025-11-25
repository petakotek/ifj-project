/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Hlavičkový soubor pro pomocné funkce
 * @file support_functions.h
 * @author Vojtěch Voldřich (xvoldrv00), Josef Pasek (xpasekj00)
 */

#ifndef SUPPORT_FUNCTIONS
#define SUPPORT_FUNCTIONS

/**
 * @brief Číselník s jednotlivými návratovými kódy, dle zadání
 *
 * Obsahuje jak chybové kódy z překladu, tak i chybové kódy z běhu výstupního programu
 */
enum errorCodes {
    /// Vše v pořádku
    NO_ERROR = 0,
    /// Chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému).
    LEX_ERROR = 1,
    /// Chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu, chybějící kostra atp.).
    SYNTAX_ERROR = 2,
    /// Sémantická chyba -- použití nedefinované funkce či proměnné.
    SEMANTICS_ERROR_UNDEF = 3,
    /// Sémantická chyba -- redefinice funkce či proměnné.
    SEMANTICS_ERROR_REDEF = 4,
    /// Statická sémantická chyba -- neočekávaný počet argumentů při volání funkce nebo @see SEMANTIC_ERROR_INCORRECT
    SEMANTICS_ERROR_UNEXPECTED = 5,
    /// Statická sémantická chyba -- špatný typ parametru vestavěné funkce.
    SEMANTICS_ERROR_INCORRECT = 5,
    /// Statická sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech
    SEMANTICS_ERROR_UNCOMPATIBLE = 6,
    /// Ostatní sémantické chyby
    SEMANTICS_ERROR_OTHERS = 10,
    /// Interní chyba překladače (tj. neovlivněná vstupním programem, např. chyba alokace paměti.
    INTERNAL_ERROR = 99,
    /// BĚHOVÁ sémantická chyba -- špatný typ parametru vestavěné funkce.
    RUNTIME_SEMANTICS_ERROR_BAD_TYPE = 25,
    /// BĚHOVÁ sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech
    RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE = 26,
};

/**
 * @brief Ukončuje běh programu, zajistí uvolnění paměti a vrací zadaný chybový kód
 *
 * @param errCode Chybový kód
 */
void memorySafeExit(enum errorCodes errCode);

/**
 * @brief Výpis chyb na standardní chybový výstup
 *
 * Zadání specifikuje, že veškeré výpisy chyb, varování nebo ladění máme provádět na standardní
 * chybový výstup
 * @attention Nepřidává znak nového řádku na konec fmt!
 * @param errCode Kód chyby
 * @param fmt Text a formát pro vypsání
 * @param ... Parametry požadované formátem
 */
void printError(enum errorCodes errCode, const char *fmt, ...);

/**
 * @brief Výpis varování na standardní chybový výstup
 *
 * Zadání specifikuje, že veškeré výpisy chyb, varování nebo ladění máme provádět na standardní
 * chybový výstup
 * @attention Nepřidává znak nového řádku na konec fmt!
 * @param fmt Text a formát pro vypsání
 * @param ... Parametry požadované formátem
 */
void printWarning(const char *fmt, ...);

/**
 * @brief Výpis ladících zpráv na standardní chybový výstup
 *
 * Zadání specifikuje, že veškeré výpisy chyb, varování nebo ladění máme provádět na standardní
 * chybový výstup
 * @attention Nepřidává znak nového řádku na konec fmt!
 * @param fmt Text a formát pro vypsání
 * @param ... Parametry požadované formátem
 */
void printLog(const char *fmt, ...);

/**
 * @brief Pomocná funkce pro výpis tokenů lexikální analýzy
 * 
 */
void print_and_free_tokens();

/**
 * @brief Funkce na načtení vstupu do řetězce
 *
 * @param void
 * @return Ukazatel na alokovaný řetězec
 */
char *readAllStdin(void);
#endif

/*** konec support_functions.h ***/
