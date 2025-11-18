/**
 * Fakulta informačních technologií VUT Brno
 *
 * Hlavičkový soubor pro implementaci hashovací tabulky s implicitním zřetězením
 * pro projekt do předmětu IFJ
 * @file hashtable.c
 * @author Petr Kotek (xkotekp00)
 * Upraveno: 13.10.2025
 */
#include "symtable.h"

#include "support_functions.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define TABLE_CAPACITY 200  // nastavení počtu položek tabulky

/**
 * @brief Inicializace tabulky, alokace pole prvků
 * @param hashT ukazatel na strukturu tabulky
 */
HashTable *hashTableInit() {
    HashTable *hashT;
    hashT = malloc(sizeof(HashTable) + sizeof(TableItem) * TABLE_CAPACITY);

    if (hashT == NULL) {
        printError(INTERNAL_ERROR, "Chyba alokace pameti tabulky");
        return NULL;
    }
    hashT->tableCapacity = TABLE_CAPACITY;
    hashT->tableSize     = 0;

    // inicializace prvků tabulky na defaultní hodnoty
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
        hashT->items[indexOfItem].key = NULL;  
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
    if (hashT == NULL || key == NULL) return -1;

 
    int existing = hashTableSearchItem(hashT, key);
    if (existing != -1)
    {
        hashT->items[existing].symbol = token;
        return existing;
    }

    if (hashT->tableSize >= hashT->tableCapacity)
    {
        if (!hashTableRealloc(hashT)) {
            printError(INTERNAL_ERROR, "Selhani realokace");
        return -1;
    }
    }

    int index = hashItem(key, hashT);
    int steps = 0;
 
    while (steps < hashT->tableCapacity) {
        int curr = (index + steps) % hashT->tableCapacity;
        TableItem *item = &hashT->items[curr];

        if (item->isEmpty || item->isDeleted)
        {
            if (item->key != NULL)
            {
                free(item->key);
            }

            item->key = malloc(strlen(key) + 1);
            if (item->key == NULL) {
                printError(INTERNAL_ERROR, "Chyba alokace klice");
                return -1;
        }
            strcpy(item->key, key);

            item->symbol = token;
            item->isEmpty = false;
            item->isDeleted = false;

                        hashT->tableSize++;
            return curr;
            }

        steps++;
        }

    // Tabulka plná → realokuj a zkus znovu
    if (!hashTableRealloc(hashT)) {
        printError(INTERNAL_ERROR, "Selhani realokace po vycerpani");
        return -1;
    }
    return hashTableInsertItem(hashT, key, token);  // rekurze

}
/**
 * @brief Funkce smaže prvek v poli v tabulce, pokud je nalezen
 * @return Funkce nemá návratovou hodnotu, v případě nenalezení prvku v tabulce tiskne error
 */
void hashTableDeleteItem(HashTable *hashT, char *key) {

    int deleteIndex = hashTableSearchItem(hashT, key);
    if (deleteIndex == -1)
    {
        printError(INTERNAL_ERROR, "Polozka v tabulce neni\n");
    }else{
        free(hashT->items[deleteIndex].key);
        hashT->items[deleteIndex].key = NULL;
        hashT->items[deleteIndex].isDeleted = true;
        hashT->items[deleteIndex].isEmpty = false;
        hashT->items[deleteIndex].symbol.name = NULL;  // Uvolnění paměti zajišťují funkce pro AS strom
    }
    hashT->tableSize--;
}

/**
 * @brief Funkce pro vyhledani polozky v tabulce symbolu
 * @return Funkce vraci index v poli hashovaci tabulky, pokud nenajde vraci -1
 */
int hashTableSearchItem(HashTable *hashT, char *key) {
    int  indexToSearch = hashItem(key, hashT);
    int  shift         = 0;
    int  originalIndex = indexToSearch;

    // test jestli neni rovnou na prvni pozici
    if (hashT->items[indexToSearch].key != NULL && !strcmp(key, hashT->items[indexToSearch].key)) {
        return indexToSearch;
    }

    while (true)
    {
        shift++;
        int currentIndex = (indexToSearch + shift) % hashT->tableCapacity;
        // Pokud narazíme na prázdnou pozici, klíč není v tabulce
        if (hashT->items[currentIndex].key == NULL && hashT->items[currentIndex].isDeleted == false) {
            return -1;
        }
        
        if (!strcmp(key, hashT->items[currentIndex].key)) {
            return currentIndex;
        }

        // prosli jsme celou tabulku
        if (currentIndex == originalIndex && shift >= hashT->tableCapacity) {
            printError(INTERNAL_ERROR, "Prvek \"%s\" v tabulce nebyl nalezen\n", key);
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
    if (hashT == NULL)
    {
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

    free(hashT);
}
/**
 * @brief Hashovací funkce
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

    return hash % hashT->tableCapacity;
}

/**
 * @brief Funkce pro realokaci pole položek tabulky symbolů, je volána z funkce hashTableInsertItem
 * @param hashT je ukazatel na tabulku, která bude alokována
 * @return nemá návratovou hodnotu
 */
HashTable *hashTableRealloc(HashTable *hashT) {
    if (hashT == NULL || hashT->tableCapacity <= 0) {
        printError(INTERNAL_ERROR, "Tabulka je prazdna");
        return NULL;
    }

    int oldCapacity = hashT->tableCapacity;
    // navýšení kapacity tabulky o 50%
    int        newCapacity = hashT->tableCapacity + (hashT->tableCapacity / 2);
    HashTable *newTable    = realloc(hashT, sizeof(HashTable) + sizeof(TableItem) * newCapacity);

    if (newTable == NULL) {
        printError(INTERNAL_ERROR, "Chyba realokace pameti tabulky symbolu");
        return NULL;
    }

    // Inicializace novych prvku
    for (int i = oldCapacity; i < newCapacity; i++) {
        newTable->items[i].key         = NULL;
        newTable->items[i].isDeleted   = false;
        newTable->items[i].isEmpty     = true;
        newTable->items[i].symbol.name = NULL;
    }
    newTable->tableCapacity = newCapacity;

    return newTable;
}

bool hashTableIsEmpty(HashTable *hashT) { return hashT->tableSize == 0; }
/* Konec hashtable.h */