/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Implementace pomocných funkcí
 * @file support_functions.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "support_functions.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "semantic_analysis.h"

// TODO pepoooooooooo!
/*
 * Definujeme zakladni kapacitu cteni souboru
 */
#define INITIAL_CAPACITY 1024

// TODO pepoooooooooo!
/**
 * @brief Ukazatel na vstupní znakové pole
 *
 */
extern char *source_file;

/**
 * @brief Zajišťuje volání funkcí pro uvolnění dynamicky alokovaných struktur
 *
 */
void memoryClean();

void printError(enum errorCodes errCode, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ERROR (%i) : ", errCode);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

#ifdef ENABLE_DEVELOPMENT_LOGGING
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
#else
void printWarning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_end(args);
}

void printLog(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_end(args);
}
#endif

void memoryClean() {
    lexer_free();
    parserFree();
    freeSemanticsMemory();
    free(source_file);
}

void memorySafeExit(enum errorCodes errCode) {
    memoryClean();
    exit(errCode);
}

// TODO pepoooooooooo!
/*
 *sorry pouze testovaci funkce ta se bude mazat tezce vygenerovana gemini
 */
void print_and_free_tokens() {
    Token token = lexer_token_next();
    while (token.type != TOKEN_EOF) {
        if (token.type == TOKEN_ERROR) {
            printLog("%s\n", token_type_to_string(token.type));
            break;
        }
        printLog("%s", token_type_to_string(token.type));
        if (token.type == TOKEN_STRING || token.type == TOKEN_STRING_BLOCK) {
            printLog(" -> value: \"%s\"\n", token.value.identifier);
            free(token.value.identifier);
        } else if (token.type == TOKEN_INTEGER || token.type == TOKEN_HEXA || token.type == TOKEN_FLOAT ||
                   token.type == TOKEN_FLOAT_EXP || token.type == TOKEN_INTEGER_EXP) {
            printLog(" -> value: \"%s\"\n", token.value.identifier);
        } else if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_GLOBAL_ID) {
            printLog(" -> id: \"%s\"\n", token.value.identifier);
            free(token.value.identifier);
        } else if (token.type == TOKEN_KEYWORD_VAR || token.type == TOKEN_KEYWORD_IF ||
                   token.type == TOKEN_KEYWORD_FOR || token.type == TOKEN_KEYWORD_WHILE ||
                   token.type == TOKEN_KEYWORD_ELSE || token.type == TOKEN_KEYWORD_RETURN ||
                   token.type == TOKEN_KEYWORD_CLASS || token.type == TOKEN_KEYWORD_IMPORT ||
                   token.type == TOKEN_KEYWORD_IFJ || token.type == TOKEN_KEYWORD_NUM ||
                   token.type == TOKEN_KEYWORD_STRING || token.type == TOKEN_KEYWORD_NULL ||
                   token.type == TOKEN_KEYWORD_STATIC || token.type == TOKEN_KEYWORD_IS ||
                   token.type == TOKEN_KEYWORD_NULL_SMALL) {
            printLog("\n");
        } else {
            printLog("\n");
        }

        token = lexer_token_next();
    }

    if (token.type == TOKEN_ERROR) {
        printWarning("Lexer error on line %d.\n", token.line);
    } else {
        printLog("%s\n", token_type_to_string(token.type));
    }
}
/*
 * Funkce na cteni pro nacteni ze vstupu jelikoz muj fseek byl nebezpecny
 */
char *read_all_stdin(void) {
    char *buffer = malloc(INITIAL_CAPACITY);
    if (buffer == NULL) {
        return NULL;
    }

    size_t total_size = 0;
    size_t capacity   = INITIAL_CAPACITY;

    while (1) {
        size_t bytes_read = fread(buffer + total_size, 1, capacity - total_size - 1, stdin);
        // dostali jsem se na konec
        if (bytes_read == 0) {
            break;
        }

        total_size += bytes_read;

        // zvětšovani bufferu aby nedoslo k preteceni
        if (total_size >= capacity - 1) {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity);
            if (new_buffer == NULL) {
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
    }

    // Null-terminate řetězec
    buffer[total_size] = '\0';
    return buffer;
}
/*** konec support_functions.c ***/
