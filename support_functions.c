/**
 * Fakulta informačních technologií VUT Brno
 *
 * Soubor pro implementaci zpracování chyb a dalších pomocných funkcí
 * pro projekt do předmětu IFJ
 * @file support_functions.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "support_functions.h"
#include "lexer.h"

void printError(enum errorCodes errCode, bool exit, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ERROR (%i) : ", errCode);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (exit == true) {
        memorySafeExit(errCode);
    }
}

void printWarning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void printLog(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "LOG: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

bool memoryClean() {
    // TODO jednotlivé funkce pro vyčištění struktur a podobně
    // TODO asi nastavit ať se to volá vždy při exit (atexit) nebo volat ručně i guess
    //      https://man7.org/linux/man-pages/man3/exit.3.html
    // TODO už můžu přidat lexer_free() a zavření souboru/free na načtený soubor
    lexer_free();
    printError(INTERNAL_ERROR, false, "Funkce memoryClean není implementovaná!\n");
    return false;
}

void memorySafeExit(enum errorCodes errCode) {
    // TODO idk co bude potřeba udělat
    memoryClean();
    exit(errCode);
}

/*** konec error_handler.c ***/