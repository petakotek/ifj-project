/**
 * Fakulta informačních technologií VUT Brno
 *
 * Implementace testů pro dynamicStack
 * @file stackTest.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "stack_test.h"

void test1() {
    DynamicStackPtr a = testInit(1, "Test1 size to small");

    if (a == NULL) {
        printf("Ok\n\n");
    }

    testDestruct(a);
}

void test2() {
    DynamicStackPtr a = testInit(2, "Test2 push, read, pop");

    printStack(a);
    char data[] = "a";
    stackPush(a, data);
    printStack(a);
    stackPop(a);
    printStack(a);

    testDestruct(a);
}

void test3() {
    DynamicStackPtr a =
        testInit(2, "Test3 multiple pushes causing resize, multiple pops causing downsize");

    printStack(a);

    stackPush(a, "a");
    printStack(a);

    stackPush(a, "b");
    printStack(a);

    stackPush(a, "c");
    printStack(a);

    stackPush(a, "d");
    printStack(a);

    stackPush(a, "e");
    printStack(a);

    stackPop(a);
    printStack(a);

    stackPop(a);
    printStack(a);

    stackPop(a);
    printStack(a);

    stackPop(a);
    printStack(a);

    stackPop(a);
    printStack(a);

    testDestruct(a);
}

void test4() {
    DynamicStackPtr a = testInit(2, "Test4 multiple upsize and order check");

    char* values[] = {"A", "B", "C", "D", "E", "F"};
    for (int i = 0; i < 6; i++) {
        stackPush(a, values[i]);
        printStack(a);
    }

    for (int i = 0; i < 6; i++) {
        stackPop(a);
        printStack(a);
    }

    testDestruct(a);
}

void test5() {
    DynamicStackPtr a = testInit(2, "Test5 pop from empty stack");

    stackPop(a);
    printStack(a);

    stackPush(a, "X");
    stackPop(a);
    printStack(a);

    stackPop(a);
    printStack(a);

    testDestruct(a);
}

void test6() {
    DynamicStackPtr a = testInit(2, "Test6 upsize and downsize sequence");

    stackPush(a, "1");
    stackPush(a, "2");
    stackPush(a, "3");
    stackPush(a, "4");
    stackPush(a, "5");
    printStack(a);

    stackPop(a);
    stackPop(a);
    stackPop(a);
    printStack(a);

    stackPush(a, "6");
    stackPush(a, "7");
    printStack(a);

    testDestruct(a);
}

void test7() {
    DynamicStackPtr a = testInit(2, "Test7 multiple read consistency");

    stackPush(a, "aaa");
    stackPush(a, "bbb");
    stackPush(a, "ccc");

    printStack(a);
    printf("Read top: %s\n", stackRead(a));
    printf("Read again: %s\n", stackRead(a));

    stackPop(a);
    printStack(a);

    testDestruct(a);
}

void test8() {
    DynamicStackPtr a = testInit(2, "Test8 random push/pop sequence");

    stackPush(a, "A");
    stackPush(a, "B");
    stackPop(a);
    stackPush(a, "C");
    stackPush(a, "D");
    stackPop(a);
    stackPush(a, "E");
    stackPush(a, "F");
    stackPop(a);
    printStack(a);

    testDestruct(a);
}

void test9() {
    DynamicStackPtr a = testInit(2, "Test9 many resizes in both ways");

    char* letters[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
                       "n", "o", "p", "x", "r", "x", "t", "u", "v", "w", "x", "y", "z"};

    for (unsigned i = 0; i < 26; i++) {
        stackPush(a, letters[i]);
        if (a->resized == true) {
            printStack(a);
        }
    }
    printStack(a);

    for (unsigned i = 0; i < 26; i++) {
        stackPop(a);
        if (a->resized == true) {
            printStack(a);
        }
    }

    testDestruct(a);
}

void test10() {
    DynamicStackPtr a =
        testInit(50, "Destruct on a non emtpy stack should not cause memory errors");

    stackPush(a, "A");
    stackPush(a, "B");
    stackPush(a, "C");
    stackPush(a, "D");
    stackPush(a, "E");
    stackPush(a, "F");
    printStack(a);

    testDestruct(a);
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
}
DynamicStackPtr testInit(unsigned size, char* testName) {
    printf("\n\n-----------------------------------------------------------\n");
    printf("[%s]\n", testName);
    return stackConstruct(size);
}

void testDestruct(DynamicStackPtr stack) { stackDestruct(stack); }

void printStack(DynamicStackPtr stack) {
    printf("Stack nextTopElement: %u\n", stack->nextTopElement);
    printf("Stack nextDownsizeAt: %u\n", stack->nextDownsizeAt);
    printf("Stack nextUpsizeAt: %u\n", stack->nextUpsizeAt);
    printf("Stack maxSize: %u\n", stack->maxSize);
    printf("Data:\n---\n");
    for (unsigned i = 0; i < stack->nextTopElement; i++) {
        if (stack->array[i] != NULL) {
            printf("    Element %u with value: %s\n", i, stack->array[i]);
        } else {
            printf("    Element %u with value: NULL\n", i);
        }
    }
    printf("---\n");

    if (stackRead(stack) != NULL) {
        printf("Stack top element: %s\n", stackRead(stack));
    } else {
        printf("Stack top element: NULL\n");
    }
}

/*** konec stackTest.c ***/