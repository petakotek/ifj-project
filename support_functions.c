/**
 * Fakulta informačních technologií VUT Brno
 *
 * Soubor pro implementaci zpracování chyb a dalších pomocných funkcí
 * pro projekt do předmětu IFJ
 * @file support_functions.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "support_functions.h"

// ----------------- deklarace pomocných funkcí -----------------
/**
 * @brief Zajišťuje volání destruktorů dynamicky alokovaných struktur
 *
 * @return true Vyčištění paměti proběhlo v pořádku
 * @return false Při čištění došlo k chybě (viz standardní chybový výstup)
 */
bool memoryClean();


// ----------------- Implementace funkcí -----------------
void printError(enum errorCodes errCode, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ERROR (%i) : ", errCode);
    vfprintf(stderr, fmt, args);
    va_end(args);
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
/*
*sorry pouze testovaci funkce ta se bude mazat tezce vygenerovana gemini
*/
void print_and_free_tokens() {
    Token token = lexer_token_next();
    while (token.type != TOKEN_EOF) {

        if (token.type == TOKEN_ERROR) {
            printf("%s\n", token_type_to_string(token.type));
            break;
        }
        printf("%s", token_type_to_string(token.type));
        if (token.type == TOKEN_STRING || token.type == TOKEN_STRING_BLOCK) {
            printf(" -> value: \"%s\"\n", token.value.string);
            free(token.value.string);
        } else if (token.type == TOKEN_INTEGER || token.type == TOKEN_HEXA ||
                   token.type == TOKEN_FLOAT || token.type == TOKEN_FLOAT_EXP ||
                   token.type == TOKEN_INTEGER_EXP) {
            printf(" -> value: \"%s\"\n", token.value.string);
        } else if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_GLOBAL_ID) {
            printf(" -> id: \"%s\"\n", token.value.identifier);
            free(token.value.identifier);
        } else if (token.type == TOKEN_KEYWORD_VAR || token.type == TOKEN_KEYWORD_IF ||
                   token.type == TOKEN_KEYWORD_FOR || token.type == TOKEN_KEYWORD_WHILE ||
                   token.type == TOKEN_KEYWORD_ELSE || token.type == TOKEN_KEYWORD_RETURN ||
                   token.type == TOKEN_KEYWORD_CLASS || token.type == TOKEN_KEYWORD_IMPORT ||
                   token.type == TOKEN_KEYWORD_IFJ || token.type == TOKEN_KEYWORD_NUM ||
                   token.type == TOKEN_KEYWORD_STRING || token.type == TOKEN_KEYWORD_NULL ||
                   token.type == TOKEN_KEYWORD_STATIC || token.type == TOKEN_KEYWORD_IS ||
                   token.type == TOKEN_KEYWORD_NULL_SMALL) {
            printf("\n");
        } else {
            printf("\n");
        }

        token = lexer_token_next();
    }

    if (token.type == TOKEN_ERROR) {
        fprintf(stderr, "Lexer error on line %d.\n", token.line);
    } else {
        printf("%s\n", token_type_to_string(token.type));
    }
}
/*** konec error_handler.c ***/
