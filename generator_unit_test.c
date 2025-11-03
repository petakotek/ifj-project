#include "codegen.h"
#include <stdbool.h>
#include <stdio.h>
#include "ast_strom.h"
#include "dynamic_stack.h"
#include "parser.h"
#include "support_functions.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// Deklarace testovaných funkcí
char *resize_label_name(char *prefix, size_t *size);
char *generatorLable(char *func_name, DynamiclabelStackStackPtr label_stack, int counter, bool pridaniDoStacku);

// Test funkce resize_label_name
void test_resize_label_name() {
    size_t size = 4;
    char *buf = malloc(size);
    strcpy(buf, "abc");
    char *new_buf = resize_label_name(buf, &size);
    assert(new_buf != NULL);
    assert(size == 8);
    assert(strcmp(new_buf, "abc") == 0);
    free(new_buf);
}

// Test generatorLable s prázdným zásobníkem
void test_generatorLable_empty_stack() {
    DynamiclabelStackStackPtr stack = stacklabelStackConstruct(10);
    char *label = generatorLable("if", stack, 1, false);
    assert(strstr(label, "$if$1") != NULL);
    free(label);
    free(stack->array);
    free(stack);
}

// Test generatorLable s prefixem v zásobníku
void test_generatorLable_with_prefix() {
    DynamiclabelStackStackPtr stack = stacklabelStackConstruct(10);
    char *prefix = strdup("fun$1$while$2");
    stacklabelStackPush(stack, prefix);
    char *label = generatorLable("if", stack, 3, true);
    assert(strstr(label, "fun$1$while$2$if$3") != NULL);
    char *top_label = stacklabelStackRead(stack);
    assert(strcmp(top_label, label) == 0);
    free(label);
    free(prefix);
    free(stack->array);
    free(stack);
}

int main() {
    test_resize_label_name();
    test_generatorLable_empty_stack();
    test_generatorLable_with_prefix();
    printf("All tests passed.\n");
    return 0;
}
