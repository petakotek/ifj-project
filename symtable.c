/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Hlavičkový soubor pro implementaci hashovací tabulky s implicitním zřetězením
 * pro projekt do předmětu IFJ
 * @file symtable.c
 * @author Petr Kotek (xkotekp00)
 */

#include "symtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "support_functions.h"

#define TABLE_CAPACITY 20  // nastavení počtu položek tabulky

/**
 * @brief Inicializace tabulky, alokace pole prvků
 * @param hashT ukazatel na strukturu tabulky
 */
HashTable *hashTableInit() {
    HashTable *hashT;
    hashT = malloc(sizeof(HashTable));

    if (hashT == NULL) {
        printError(INTERNAL_ERROR, "Chyba alokace pameti tabulky");
        return NULL;
    }

    hashT->items = malloc(sizeof(TableItem) * TABLE_CAPACITY);

    hashT->tableCapacity = TABLE_CAPACITY;
    hashT->tableSize     = 0;

    // inicializace prvků tabulky na základní hodnoty
    hashTableInitItems(hashT);
    return hashT;
}
/**
 * @brief Funkce inicializuje všechny prvky v tabulce na defaultní hodnoty s vyjímkou pole key
 * @param hashT Ukazatel na hashovací tabulku
 */
void hashTableInitItems(HashTable *hashT) {
    for (int indexOfItem = 0; indexOfItem < hashT->tableCapacity; indexOfItem++) {
        // alokace řetězce proběhne jakmile bude to prvku pole vkládáno
        hashT->items[indexOfItem].key         = NULL;
        hashT->items[indexOfItem].isEmpty     = true;
        hashT->items[indexOfItem].isDeleted   = false;
        hashT->items[indexOfItem].symbol.name = NULL;
    }
}

/**
 * @brief Funkce vloží prvek do pole položek hashovací tabulky a vrátí index, na kterém byla položka
 * uložena. V případě, že index, který vypočítala hashovací funkce je již obsazený postupuje dál
 * polem a hledá první volný index
 * @return Funkce vraci index v poli polozek
 */
int hashTableInsertItem(HashTable *hashT, char *key, SemSymbol token) {
    if (hashT == NULL || key == NULL)  // tabulka je prazdna nebo zadany klic je NULL
    {
        return -1;
    }

    int nasobek = hashT->tableSize * 1.2;
    if (nasobek > hashT->tableCapacity) {
        hashTableRealloc(hashT);
    }

    int existing = hashTableSearchItem(hashT, key);
    if (existing != -1) {
        hashT->items[existing].symbol = token;
        return existing;
    }

    // int nasobek = hashT->tableSize * 1.2;

    int index = hashItem(key, hashT);
    int steps = 0;

    while (steps < hashT->tableCapacity) {
        int        curr = (index + steps) % hashT->tableCapacity;
        TableItem *item = &hashT->items[curr];

        if (item->isEmpty || item->isDeleted) {
            if (item->key != NULL) {
                free(item->key);
            }

            item->key = malloc(strlen(key) + 1);
            if (item->key == NULL) {
                printError(INTERNAL_ERROR, "Chyba alokace klice");
                return -1;
            }
            // uložení klíče do parametru položky tabulky
            strcpy(item->key, key);

            item->symbol    = token;
            item->isEmpty   = false;
            item->isDeleted = false;

            hashT->tableSize++;
            return curr;
        }
        steps++;
    }

    return -1;
}
/**
 * @brief Funkce smaže prvek v poli v tabulce, pokud je nalezen
 * @return Funkce nemá návratovou hodnotu, v případě nenalezení prvku v tabulce tiskne error
 */
void hashTableDeleteItem(HashTable *hashT, char *key) {
    int deleteIndex = hashTableSearchItem(hashT, key);
    if (deleteIndex == -1) {
        printError(INTERNAL_ERROR, "Polozka v tabulce neni\n");
    } else {
        free(hashT->items[deleteIndex].key);
        hashT->items[deleteIndex].key         = NULL;
        hashT->items[deleteIndex].isDeleted   = true;
        hashT->items[deleteIndex].isEmpty     = false;
        hashT->items[deleteIndex].symbol.name = NULL;  // Uvolnění paměti zajišťují funkce pro AS strom
    }
    hashT->tableSize--;
}

/**
 * @brief Funkce pro vyhledání polozky v tabulce symbolů
 * @return Funkce vrací index v poli tabulky symbolů, pokud nenajde, vraci -1
 */
int hashTableSearchItem(HashTable *hashT, char *key) {
    int indexToSearch = hashItem(key, hashT);
    int shift         = 0;
    int originalIndex = indexToSearch;

    // test jestli neni rovnou na prvni pozici
    if (hashT->items[indexToSearch].key != NULL && !strcmp(key, hashT->items[indexToSearch].key)) {
        return indexToSearch;
    }

    while (true) {
        shift++;
        int currentIndex = (indexToSearch + shift) % hashT->tableCapacity;
        // Pokud narazíme na prázdnou pozici, klíč není v tabulce
        if (hashT->items[currentIndex].key == NULL && hashT->items[currentIndex].isDeleted == false) {
            return -1;
        }

        if (!strcmp(key, hashT->items[currentIndex].key)) {
            return currentIndex;
        }

        // prošli jsme celou tabulku
        if (currentIndex == originalIndex && shift >= hashT->tableCapacity) {
            // printError(INTERNAL_ERROR, "Prvek \"%s\" v tabulce nebyl nalezen\n", key);
            return -1;
        }
    }
    return -1;
}
/**
 * @brief Funkce sloužící pro uvolnění paměti tabulky symbolů
 * @param hashT Ukazatel na tabulku symbolů, která bude uvolněna
 *
 */
void hashTableDealloc(HashTable *hashT) {
    if (hashT == NULL) {
        printError(INTERNAL_ERROR, "Pokus o uvolneni neinicializovane tabulky\n");
        return;
    }

    for (int i = 0; i < hashT->tableCapacity; i++) {
        if (hashT->items[i].key != NULL) {
            free(hashT->items[i].key);  // uvolneni retezce polozky v tabulce
            hashT->items[i].key         = NULL;
            hashT->items[i].symbol.name = NULL;  // Uvolnění paměti zajišťují funkce pro AS strom
        }
    }
    free(hashT->items);  // uvolneni alokace pole

    free(hashT);  // uvolneni alokace tabulky
}

/**
 * @brief Implementace hashovací funkce polocí metody Daniela J. Bernsteina
 * @param key ukazatel na řetězec, který bude vkládán
 * @return Vrací index v poli struktur, který je využit při hledání, mazání, vkládání
 */
int hashItem(char *key, HashTable *hashT) {
    if (key == NULL) {
        printError(INTERNAL_ERROR, "Nelze hashovat NULL prvek");
        return INTERNAL_ERROR;
    }

    unsigned long hash = 5381;
    int           c;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }

    // modulo k zajištění kruhovosti vraceného indexu
    return hash % hashT->tableCapacity; 
}

/**
 * @brief Funkce pro realokaci pole položek tabulky symbolů, je volána z funkce hashTableInsertItem
 * @param hashT je ukazatel na tabulku, která bude alokována
 */
void hashTableRealloc(HashTable *hashT) {

    if (hashT == NULL) {
        printError(INTERNAL_ERROR, "Tabulka je prazdna");
        return;
    }

    int oldCapacity = hashT->tableCapacity;
    int newCapacity = hashT->tableCapacity + (hashT->tableCapacity / 2);  // + 50 %

    TableItem *oldArray = hashT->items;

    // Alokace nové velikosti tabulky
    TableItem *newArray = malloc(sizeof(TableItem) * newCapacity);  

    if (newArray == NULL) {
        printError(INTERNAL_ERROR, "Chyba realokace pameti pole symbolu");
        hashTableDealloc(hashT);
        return;
    }

    hashT->items         = newArray;
    hashT->tableCapacity = newCapacity;  // nastaveni nove kapacity tabulky
    hashTableInitItems(hashT);

    for (int i = 0; i < oldCapacity; i++) {
        bool      empty  = oldArray[i].isEmpty;
        char     *key    = oldArray[i].key;
        SemSymbol symbol = oldArray[i].symbol;

        if (empty == false) {
            if (oldArray[i].key != NULL) {
                hashTableInsertItem(hashT, key, symbol);
                free(key); // uvolnění klíče položky staré tabulky
            }
        }
    }

    free(oldArray); // uvolnění staré tabulky
    oldArray = NULL;
}

/**
 * @brief Funkce pro ověřující prázdnost tabulky
 * @param hashT Ukazatel na tabulku symbolů
 * @return Funkce navrací hodnotu true v případě, že je tabulka prádná, jinak false
 */
bool hashTableIsEmpty(HashTable *hashT) { return hashT->tableSize == 0; }

/* Konec symtable.c */