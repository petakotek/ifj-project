/**
 * Fakulta informačních technologií VUT Brno
 *
 * Hlavičkový soubor pro implementaci zpracování chyb a dalších pomocných funkcí
 * pro projekt do předmětu IFJ
 * @file support_functions.h
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#ifndef ERROR_HANDLER

#define ERROR_HANDLER 1

// ----------------- Závislosti -----------------
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// ----------------- Datové typy -----------------
/**
 * @brief Číselník s jednotlivými návratovými kódy, jak definuje zadání
 *
 * Obsahuje jak chybové kódy z překladu, tak i chybové kódy z běhu výstupního programu
 */
enum errorCodes {
    /// Vše v pořádku
    NO_ERROR = 0,
    /// Chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému).
    LEX_ERROR = 1,
    /// Chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu, chybějící kostra
    /// atp.).
    SYNTAX_ERROR = 2,
    /// Sémantická chyba -- použití nedefinované funkce či proměnné.
    SEMANTICS_ERROR_UNDEF = 3,
    /// Sémantická chyba -- redefinice funkce či proměnné.
    SEMANTICS_ERROR_REDEF = 4,
    /// Statická sémantická chyba -- neočekávaný počet argumentů při volání funkce nebo
    SEMANTICS_ERROR_UNEXPECTED = 5,
    /// Statická sémantická chyba -- špatný typ parametru vestavěné funkce.
    SEMANTICS_ERROR_INCORRECT = 5,
    /// Statická sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních
    /// výrazech
    SEMANTICS_ERROR_UNCOMPATIBLE = 6,
    /// Ostatní sémantické chyby
    SEMANTICS_ERROR_OTHERS = 10,
    /// Interní chyba překladače (tj. neovlivněná vstupním programem, např. chyba alokace paměti.
    INTERNAL_ERROR = 99,
    /// BĚHOVÁ sémantická chyba -- špatný typ parametru vestavěné funkce.
    RUNTIME_SEMANTICS_ERROR_BAD_TYPE = 25,
    /// BĚHOVÁ sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních
    /// výrazech
    RUNTIME_SEMANTICS_ERROR_UNCOMPATIBLE = 26,
};

// ----------------- deklarace funkcí -----------------
/**
 * @brief Zajišťuje volání destruktorů dynamicky alokovaných struktur
 *
 * @return true Vyčištění paměti proběhlo v pořádku
 * @return false Při čištění došlo k chybě (viz standardní chybový výstup)
 */
bool memoryClean();

/**
 * @brief Ukončuje běh programu, zajistí vyčištění paměti a vrací zadaný chybový kód
 *
 * @param errCode Chybový kód
 */
void memorySafeExit(enum errorCodes errCode);

/**
 * @brief Výpis chyb na standardní chybový výstup
 *
 * Zadání specifikuje, že veškeré výpisy chyb, varování nebo ladění máme provádět na standardní
 * chybový výstup
 * @param errCode Kód chyby
 * @param exit Určuje zda má funkce zajistit ukončení běhu programu s uvolněním paměti a vrácením
 * chybového kódu
 * @param fmt Text a formát pro vypsání
 * @param ... Parametry požadované formátem
 */
void printError(enum errorCodes errCode, bool exit, const char *fmt, ...);

/**
 * @brief Výpis varování na standardní chybový výstup
 *
 * Zadání specifikuje, že veškeré výpisy chyb, varování nebo ladění máme provádět na standardní
 * chybový výstup
 * @param fmt Text a formát pro vypsání
 * @param ... Parametry požadované formátem
 */
void printWarning(const char *fmt, ...);

/**
 * @brief Výpis ladících zpráv na standardní chybový výstup
 *
 * Zadání specifikuje, že veškeré výpisy chyb, varování nebo ladění máme provádět na standardní
 * chybový výstup
 * @param fmt Text a formát pro vypsání
 * @param ... Parametry požadované formátem
 */
void printLog(const char *fmt, ...);
void print_and_free_tokens();
#endif

/*** konec error_handler.h ***/
