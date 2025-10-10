/**
 * Fakulta informačních technologií VUT Brno
 *
 * Soubor pro implementaci zpracování chyb a dalších pomocných funkcí
 * pro projekt do předmětu IFJ
 * @file error_handler.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

 // TODO možná přejmenovat na helper_functions nebo support_functions?

#include "error_handler.h"

bool memoryClean() {
    // TODO jednotlivé funkce pro vyčištění struktur a podobně
    // TODO asi nastavit ať se to volá vždy při exit (atexit) nebo volat ručně i guess
    //      https://man7.org/linux/man-pages/man3/exit.3.html
    // TODO už můžu přidat lexer_free() a zavření souboru/free na načtený soubor
    printError(INTERNAL_ERROR, false, "Funkce memoryClean není implementovaná!\n");
    return false;
}

void errorExit(enum errorCodes errCode) {
    // TODO idk co bude potřeba udělat
    memoryClean();
    exit(errCode);
}

/*** konec error_handler.c ***/