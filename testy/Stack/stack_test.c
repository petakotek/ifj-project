/**
 * Fakulta informačních technologií VUT Brno
 *
 * Implementace testů pro dynamicStack
 * @file stackTest.c
 * @author Vojtěch Voldřich (xvoldrv00)
 */

#include "stack_test.h"

void test1() {
    DynamicStringStackPtr a = testInit(1, "Test1 size to small");

    if (a == NULL) {
        printf("Ok\n\n");
    }

    testDestruct(a);
}

void test2() {
    DynamicStringStackPtr a = testInit(2, "Test2 push, read, pop");

    printStack(a);
    char data[] = "a";
    stackStringPush(a, data);
    printStack(a);
    stackStringPop(a);
    printStack(a);

    testDestruct(a);
}

void test3() {
    DynamicStringStackPtr a =
        testInit(2, "Test3 multiple pushes causing resize, multiple pops causing downsize");

    printStack(a);

    stackStringPush(a, "a");
    printStack(a);

    stackStringPush(a, "b");
    printStack(a);

    stackStringPush(a, "c");
    printStack(a);

    stackStringPush(a, "d");
    printStack(a);

    stackStringPush(a, "e");
    printStack(a);

    stackStringPop(a);
    printStack(a);

    stackStringPop(a);
    printStack(a);

    stackStringPop(a);
    printStack(a);

    stackStringPop(a);
    printStack(a);

    stackStringPop(a);
    printStack(a);

    testDestruct(a);
}

void test4() {
    DynamicStringStackPtr a = testInit(2, "Test4 multiple upsize and order check");

    char* values[] = {"A", "B", "C", "D", "E", "F"};
    for (int i = 0; i < 6; i++) {
        stackStringPush(a, values[i]);
        printStack(a);
    }

    for (int i = 0; i < 6; i++) {
        stackStringPop(a);
        printStack(a);
    }

    testDestruct(a);
}

void test5() {
    DynamicStringStackPtr a = testInit(2, "Test5 pop from empty stack");

    stackStringPop(a);
    printStack(a);

    stackStringPush(a, "X");
    stackStringPop(a);
    printStack(a);

    stackStringPop(a);
    printStack(a);

    testDestruct(a);
}

void test6() {
    DynamicStringStackPtr a = testInit(2, "Test6 upsize and downsize sequence");

    stackStringPush(a, "1");
    stackStringPush(a, "2");
    stackStringPush(a, "3");
    stackStringPush(a, "4");
    stackStringPush(a, "5");
    printStack(a);

    stackStringPop(a);
    stackStringPop(a);
    stackStringPop(a);
    printStack(a);

    stackStringPush(a, "6");
    stackStringPush(a, "7");
    printStack(a);

    testDestruct(a);
}

void test7() {
    DynamicStringStackPtr a = testInit(2, "Test7 multiple read consistency");

    stackStringPush(a, "aaa");
    stackStringPush(a, "bbb");
    stackStringPush(a, "ccc");

    printStack(a);
    printf("Read top: %s\n", stackStringRead(a));
    printf("Read again: %s\n", stackStringRead(a));

    stackStringPop(a);
    printStack(a);

    testDestruct(a);
}

void test8() {
    DynamicStringStackPtr a = testInit(2, "Test8 random push/pop sequence");

    stackStringPush(a, "A");
    stackStringPush(a, "B");
    stackStringPop(a);
    stackStringPush(a, "C");
    stackStringPush(a, "D");
    stackStringPop(a);
    stackStringPush(a, "E");
    stackStringPush(a, "F");
    stackStringPop(a);
    printStack(a);

    testDestruct(a);
}

void test9() {
    DynamicStringStackPtr a = testInit(2, "Test9 many resizes in both ways");

    char* letters[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
                       "n", "o", "p", "x", "r", "x", "t", "u", "v", "w", "x", "y", "z"};

    for (unsigned i = 0; i < 26; i++) {
        stackStringPush(a, letters[i]);
        if (a->resized == true) {
            printStack(a);
        }
    }
    printStack(a);

    for (unsigned i = 0; i < 26; i++) {
        stackStringPop(a);
        if (a->resized == true) {
            printStack(a);
        }
    }

    testDestruct(a);
}

void test10() {
    DynamicStringStackPtr a =
        testInit(50, "Destruct on a non empty stack should not cause memory errors");

    stackStringPush(a, "A");
    stackStringPush(a, "B");
    stackStringPush(a, "C");
    stackStringPush(a, "D");
    stackStringPush(a, "E");
    stackStringPush(a, "F");
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
DynamicStringStackPtr testInit(unsigned size, char* testName) {
    printf("\n\n-----------------------------------------------------------\n");
    printf("[%s]\n", testName);
    return stackStringConstruct(size);
}

void testDestruct(DynamicStringStackPtr stack) { stackStringDestruct(stack); }

void printStack(DynamicStringStackPtr stack) {
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

    if (stackStringRead(stack) != NULL) {
        printf("Stack top element: %s\n", stackStringRead(stack));
    } else {
        printf("Stack top element: NULL\n");
    }
}

/*** konec stackTest.c ***/