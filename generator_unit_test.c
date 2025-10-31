#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "codegen.h"
#include "dynamic_stack.h"

// --- Mock Závislosti (Nahraď skutečnými hlavičkovými soubory, pokud existují) ---

// Globální proměnné pro kontrolu volání mock funkcí
int mock_error_called = 0;
char *mock_next_prefix = NULL;
bool mock_stack_empty_flags[] = {true, true, true}; // pro různé testy
int mock_stack_read_count = 0;
char *mock_stack_push_result = NULL;
bool mock_malloc_fail = false;
bool mock_resize_called = false;
char *mock_resize_return = NULL;


// Mock pro resize_label_name
char *resize_label_name(char *old_label, size_t *old_size) {
    mock_resize_called = true;
    if (mock_resize_return != NULL) {
        // Simulace úspěšné realokace
        *old_size = 256; // Změníme velikost bufferu
        // Zkopírujeme starý obsah do nového bufferu
        char *new_label = (char*)malloc(*old_size);
        if (new_label) {
             strcpy(new_label, old_label);
             free(old_label); // Uvolnění starého bufferu
             return new_label;
        }
        return NULL;
    }
    // Simulace selhání
    free(old_label);
    return NULL;
}


// --- Testovaná Funkce (Zkopírovaná z tvého zadání, pro kompilaci v jednom souboru) ---
// Tady vlož tvou funkci generatorLable
char *generatorLable(char *func_name, DynamiclabelStackStackPtr label_stack, int counter) {
    size_t size_of_buffer = 128;
    // POUŽITÍ mock_malloc_fail pro simulaci selhání malloc
    char *result_label = (char *) (mock_malloc_fail ? NULL : malloc(size_of_buffer));

    if (result_label == NULL) {
        printError(INTERNAL_ERROR, "error behem tvoreni lablu ");
        memorySafeExit(INTERNAL_ERROR);
        return NULL; // Abychom nevraceli NULL mimo memorySafeExit
    }
    //vlozeni func_name do bufferu kdyz nevejde hazime error
    int current_len = snprintf(result_label, size_of_buffer, "%s", func_name);
    if (current_len < 0 || (size_t)current_len >= size_of_buffer) {
        free(result_label);
        printError(INTERNAL_ERROR, "error behem tvoreni lablu ");
        memorySafeExit(INTERNAL_ERROR);
        return NULL;
    }
    while (!stacklabelStackIsEmpty(label_stack)) {
        char *prefix = stacklabelStackRead(label_stack);
        size_t prefix_len = strlen(prefix);
        if (current_len + prefix_len + 5 >= size_of_buffer) {
            result_label = resize_label_name(result_label, &size_of_buffer);
            if (result_label == NULL) {
            printError(INTERNAL_ERROR, "error behem tvoreni lablu ");
            memorySafeExit(INTERNAL_ERROR);
            return NULL;
            }
        }
        int written = snprintf(result_label + current_len, size_of_buffer - current_len, "%s", prefix);
        if (written > 0) {
            current_len += written;
        } else {
            break;
        }
    }
    if (current_len + 1 + 10 >= size_of_buffer) {
        result_label = resize_label_name(result_label, &size_of_buffer);
        if (result_label == NULL) {
        printError(INTERNAL_ERROR, "error behem tvoreni lablu ");
        memorySafeExit(INTERNAL_ERROR);
        return NULL;
        }
    }
    result_label[current_len] = '$';
    current_len++;

    size_t remaining_space = size_of_buffer - current_len;

    int written_counter = snprintf(result_label + current_len,remaining_space,"%d",counter);

    if (written_counter > 0) {
        current_len += written_counter;
    }
    stacklabelStackPush(label_stack, result_label);
    return result_label;
}

// --- Resetovací funkce pro každý test ---
void setup_test() {
    mock_error_called = 0;
    mock_next_prefix = NULL;
    mock_stack_empty_flags[0] = true;
    mock_stack_empty_flags[1] = true;
    mock_stack_read_count = 0;
    mock_stack_push_result = NULL;
    mock_malloc_fail = false;
    mock_resize_called = false;
    mock_resize_return = NULL;
}

// --- Jednoduchá ASSERT funkce ---
void assert_true(bool condition, const char *message) {
    if (!condition) {
        printf("FAILED: %s\n", message);
    } else {
        printf("PASSED: %s\n", message);
    }
}
void assert_string_equal(const char *expected, const char *actual, const char *message) {
    if (strcmp(expected, actual) != 0) {
        printf("FAILED: %s - Expected: \"%s\", Actual: \"%s\"\n", message, expected, actual);
    } else {
        printf("PASSED: %s\n", message);
    }
}


// --- Testovací případy ---

void test_generatorLable_basic() {
    setup_test();
    printf("\n--- Test 1: Základní vytvoření ---\n");
    // Prázdný stack, jednoduchý název funkce
    char *result = generatorLable("main", NULL, 1);

    assert_string_equal("main$1", result, "Label by měl být 'main$1'");
    assert_true(mock_error_called == 0, "Neměla nastat žádná chyba");
    assert_string_equal("main$1", mock_stack_push_result, "Správný label byl pushnut");

    free(result);
}

void test_generatorLable_with_prefix() {
    setup_test();
    printf("\n--- Test 2: Vytvoření s prefixem ze stacku ---\n");
    // Nastavíme stack, aby vrátil jeden prefix a pak byl prázdný
    mock_stack_empty_flags[0] = false; // Není prázdný poprvé
    mock_stack_empty_flags[1] = true;  // Je prázdný podruhé (kvůli mock_stack_read_count v mocku)

    char *result = generatorLable("myfunc", NULL, 42);

    assert_string_equal("myfuncprefix$42", result, "Label by měl být 'myfuncprefix$42'");
    assert_true(mock_error_called == 0, "Neměla nastat žádná chyba");
    assert_string_equal("myfuncprefix$42", mock_stack_push_result, "Správný label byl pushnut");

    free(result);
}


void test_generatorLable_malloc_fail() {
    setup_test();
    printf("\n--- Test 3: Selhání malloc ---\n");
    // Simulujeme selhání prvního malloc
    mock_malloc_fail = true;
    char *result = generatorLable("test", NULL, 1);

    assert_true(mock_error_called == 2, "Měla být zavolána printError a memorySafeExit");
    assert_true(result == NULL, "Funkce by měla vrátit NULL po exitu");
}


void test_generatorLable_resize_needed() {
    setup_test();
    printf("\n--- Test 4: Nutná realokace (resize) ---\n");
    // Velmi dlouhý funkční název + prefix, aby došlo k realokaci v cyklu while
    char func_name_long[100];
    memset(func_name_long, 'A', 99);
    func_name_long[99] = '\0'; // 99 A's

    mock_stack_empty_flags[0] = false;
    mock_stack_empty_flags[1] = true;
    mock_resize_return = (char*)1; // Nastavíme, že mock resize je úspěšný

    char *result = generatorLable(func_name_long, NULL, 500);

    assert_true(mock_resize_called, "resize_label_name měla být zavolána");
    // Očekávaný výsledek je 99 A's + prefix + $500
    char expected_result[110];
    snprintf(expected_result, 110, "%s%s$%d", func_name_long, "prefix", 500);
    assert_string_equal(expected_result, result, "Label se měl správně sestavit i po resize");
    assert_true(mock_error_called == 0, "Neměla nastat žádná chyba");

    free(result);
}

// --- Hlavní funkce pro spuštění testů ---
int main() {
    // Definuj konstantu INTERNAL_ERROR pro mocky
    #define INTERNAL_ERROR 99

    test_generatorLable_basic();
    test_generatorLable_with_prefix();
    test_generatorLable_malloc_fail();
    test_generatorLable_resize_needed();

    return 0;
}
