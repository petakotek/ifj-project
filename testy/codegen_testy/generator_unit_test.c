#include "../codegen.h"
#include <stdbool.h>
#include <stdio.h>
#include "../ast_strom.h"
#include "../dynamic_stack.h"
#include "../parser.h"
#include "../support_functions.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// Deklarace testovaných funkcí

// 💥 Test 1: Ověření dynamické alokace a zvětšení (resize_label_name)
void test_resize_robust() {
    printf("🧪 Test 1: Robustní ověření resize_label_name...\n");
    size_t initial_size = 4;
    char *buf = (char*)malloc(initial_size);
    assert(buf != NULL && "Chyba: malloc selhal pro resize test.");
    strcpy(buf, "abc");

    size_t original_size = initial_size;
    char *new_buf = resize_label_name(buf, &initial_size);

    assert(new_buf != NULL && "Chyba: realloc selhal při zvětšování.");
    assert(initial_size == original_size * 2 && "Chyba: Velikost se nezvětšila dvakrát.");
    assert(strcmp(new_buf, "abc") == 0 && "Chyba: Obsah se ztratil po resize.");
    printf("   ✅ Základní resize (4 -> 8) je v pořádku.\n");

    // Zkusíme donutit vícenásobnou resize (např. 8 -> 16 -> 32)
    char long_string[20];
    memset(long_string, 'X', 19);
    long_string[19] = '\0'; // 19 znaků + '\0'
    strcpy(new_buf, long_string);

    size_t current_size = initial_size; // Nyní 8

    // Zkusíme resize znovu (8 -> 16)
    new_buf = resize_label_name(new_buf, &current_size);
    assert(new_buf != NULL && current_size == 16 && "Chyba: Druhá resize selhala.");

    // Zkusíme resize potřetí (16 -> 32)
    new_buf = resize_label_name(new_buf, &current_size);
    assert(new_buf != NULL && current_size == 32 && "Chyba: Třetí resize selhala.");
    assert(strcmp(new_buf, long_string) == 0 && "Chyba: Obsah se ztratil po vícenásobném resize.");

    printf("   ✅ Vícenásobné zvětšení paměti funguje.\n");
    free(new_buf);
}

// 🎯 Test 2: Prázdný zásobník a ověření PUSH/NO-PUSH
void test_generatorLable_push_control() {
    printf("🧪 Test 2: Kontrola push logiky se startovním stavem...\n");
    DynamiclabelStackStackPtr stack = stacklabelStackConstruct(2); // Malý zásobník pro test resize

    // Scénář 2.1: Prázdný stack, NEpřidávat
    char *label_no_push = generatorLable("fun", stack, 1, false);
    assert(strcmp(label_no_push, "$fun$1") == 0 && "Chyba: Špatný label bez prefixu (no push).");
    assert(stacklabelStackIsEmpty(stack) && "Chyba: Stack má obsah, i když pridaniDoStacku=false.");
    free(label_no_push);
    printf("   ✅ Scénář 2.1: Startovní label bez push je OK.\n");

    // Scénář 2.2: Prázdný stack, PŘIDAT
    char *label_with_push = generatorLable("while", stack, 1, true);
    assert(strcmp(label_with_push, "$while$1") == 0 && "Chyba: Špatný label bez prefixu (with push).");
    assert(stack->nextTopElement == 1 && "Chyba: Prvek nebyl pushnut.");
    assert(strcmp(stacklabelStackRead(stack), label_with_push) == 0 && "Chyba: Pushnutý obsah je nesprávný.");

    // Scénář 2.3: S prefixem (z 2.2), NEpřidávat
    char *label_with_prefix = generatorLable("if", stack, 2, false);
    assert(strcmp(label_with_prefix, "$while$1$if$2") == 0 && "Chyba: Špatný label s prefixem.");
    assert(stack->nextTopElement == 1 && "Chyba: Stack se neměl měnit (nextTopElement=1).");
    free(label_with_prefix);

    // Uvolnění paměti pro pushnutý prvek
    free(stack->array[0]);
    stacklabelStackDestruct(stack);
    printf("   ✅ Scénář 2.2 & 2.3: Kontrola push/no-push po prvním zanoření je OK.\n");
}

// 💣 Test 3: Testování kritické paměťové hranice (vynucení resize)
void test_generatorLable_resize_boundary() {
    printf("🧪 Test 3: Vynucení resize na hranici bufferu...\n");
    DynamiclabelStackStackPtr stack = stacklabelStackConstruct(2);

    // Vytvoříme prefix, který je těsně pod 128 (max size je 128)
    char long_prefix[120];
    memset(long_prefix, 'P', 119);
    long_prefix[119] = '\0';

    // Uložíme prefix na zásobník (simulace předchozích zanoření)
    stacklabelStackPush(stack, strdup(long_prefix));

    // Nyní voláme generatorLable s func_name, který má 10 znaků (např. 'long_block')
    // Prefix (119) + $ (1) + long_block (10) + $ (1) + 99 (2) = 133 znaků. PŘESÁHNE 128!
    // Toto musí spustit resize_label_name.

    char *result = generatorLable("long_block", stack, 99, true);

    const char *expected_suffix = "$long_block$99";
    char expected_label[200];
    snprintf(expected_label, 200, "%s%s", long_prefix, expected_suffix);

    assert(strcmp(result, expected_label) == 0 && "Chyba: Výsledný label po resize je špatný.");
    assert(strlen(result) > 128 && "Chyba: Resize nebyla správně provedena pro velký string.");
    assert(stack->nextTopElement == 2 && "Chyba: Label nebyl po resize pushnut.");

    printf("   ✅ Resize vyvolané velkým suffixem funguje a výsledek je správný.\n");

    // Uvolnění alokované paměti (dvou pushnutých stringů)
    free(stack->array[0]);
    free(stack->array[1]);
    stacklabelStackDestruct(stack);
}

// 🔄 Test 4: Zanoření více úrovní a správné čtení
void test_generatorLable_deep_nesting() {
    printf("🧪 Test 4: Testování hlubokého zanoření...\n");
    DynamiclabelStackStackPtr stack = stacklabelStackConstruct(10);

    // 1. fun$1 (push)
    char *l1 = generatorLable("fun", stack, 1, true);
    // 2. fun$1$while$1 (push)
    char *l2 = generatorLable("while", stack, 1, true);
    // 3. fun$1$while$1$if$1 (no push)
    char *l3 = generatorLable("if", stack, 1, false);
    // 4. fun$1$while$1$for$2 (push)
    char *l4 = generatorLable("for", stack, 2, true);

    // Ověření výsledků
    assert(strcmp(l1, "$fun$1") == 0);
    assert(strcmp(l2, "$fun$1$while$1") == 0);
    assert(strcmp(l3, "$fun$1$while$1$if$1") == 0);
    assert(strcmp(l4, "$fun$1$while$1$for$2") == 0);

    // Ověření stacku
    assert(stack->nextTopElement == 3 && "Chyba: Počet prvků na stacku je špatný.");
    assert(strcmp(stack->array[0], l1) == 0 && "Chyba: 1. prvek na stacku je špatný.");
    assert(strcmp(stack->array[1], l2) == 0 && "Chyba: 2. prvek na stacku je špatný.");
    assert(strcmp(stack->array[2], l4) == 0 && "Chyba: 3. prvek na stacku je špatný.");

    printf("   ✅ Všechny úrovně zanoření fungují a stack je konzistentní.\n");

    // Uvolnění paměti (alokované stringy na heapu)
    free(l3); // l3 je jediný, který nebyl pushnut (volající je odpovědný)
    // Uvolnění pushnutých prvků
    free(stack->array[0]);
    free(stack->array[1]);
    free(stack->array[2]);
    stacklabelStackDestruct(stack);
}


// =======================================================
// === MAIN RUNNER ===
// =======================================================

int main() {
    printf("--- 🧪 Spouštění expanzivních testů pro Generátor Návěští ---\n\n");

    test_resize_robust();
    printf("\n");

    test_generatorLable_push_control();
    printf("\n");

    test_generatorLable_resize_boundary();
    printf("\n");

    test_generatorLable_deep_nesting();
    printf("\n");

    printf("--- 🎉 VŠECHNY EXPANZIVNÍ TESTY PROŠLY ÚSPĚŠNĚ! ---\n");
    return 0;
}
