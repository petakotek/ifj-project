/**
 * Fakulta informačních technologií VUT Brno
 *
 * Soubor pro implementaci zpracování chyb a dalších pomocných funkcí
 * pro projekt do předmětu IFJ
 * @file error_handler.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "error_handler.h"

inline void printError(enum errorCodes errCode, bool exit, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ERROR (%i) : ", errCode);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (exit == true) {
        errorExit(errCode);
    }
}

inline void printWarning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

inline void printLog(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "LOG: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}

inline bool memoryClean() {
    // TODO jednotlivé funkce pro vyčištění struktur a podobně
    // TODO asi nastavit ať se to volá vždy při exit (atexit) nebo volat ručně i guess
    printError(INTERNAL_ERROR, false, "Funkce memoryClean není implementovaná!\n");
    return false;
}

inline void errorExit(enum errorCodes errCode) {
    // TODO idk co bude potřeba udělat
    memoryClean();
    exit(errCode);
}

/*** konec error_handler.c ***/