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
int hashTableInsertItem(HashTable **hashT, char *key, SemSymbol token)
{
    if (key == NULL) // klic je prazdny
    {
        printError(INTERNAL_ERROR, "Pokus o vlozeni klice s hodnotou NULL");
        return -1;
    }

    if ((*hashT) == NULL)
    {
        printError(INTERNAL_ERROR, "Tabulka neexistuje\n");
        return -1;
    }
    
    if ((*hashT)->tableSize + 1 >= (*hashT)->tableCapacity)
    {
        HashTable *t = hashTableRealloc(hashT);
        (*hashT) = t;
    }

 
    int foundIndex = hashTableSearchItem((*hashT), key);
    if (foundIndex == -1) // polozka s danym klicem neni v tabulce
    {
        int keyHash = hashItem(key, (*hashT)); // dostanu hash klice

        // pripad, kdy to trefim na prvni dobrou
        if ((*hashT)->items[keyHash].key == NULL) // na aktualnim indexu je hodnota klice NULL
        {
            if ((*hashT)->items[keyHash].isEmpty == true) // zaroven je bunka prazdna
            {
                (*hashT)->items[keyHash].key = strdup(key); // nastaveni ukazatele na dany klic        
                (*hashT)->items[keyHash].isDeleted = false;
                (*hashT)->items[keyHash].isEmpty = false;
                (*hashT)->items[keyHash].symbol = token;
                (*hashT)->tableSize++;
                return keyHash;
            }
        }
        // kdyz uz tam neco je, ale neni klic je jiny
        int shift = 1;
        while (true)
        {
            int indexStore = (keyHash + shift) % (*hashT)->tableCapacity; // rounding
            if ((*hashT)->items[indexStore].key == NULL) // klic je prazdny
            {
                if ((*hashT)->items[indexStore].isEmpty == true) // polozka je prazdna
                {
                    if ((*hashT)->items[indexStore].isDeleted == false) // a nebyla nikdy mazana
                    {
                        (*hashT)->items[indexStore].key = strdup(key); // nastaveni ukazatele na dany klic
                        (*hashT)->items[indexStore].isDeleted = false;
                        (*hashT)->items[indexStore].isEmpty = false;
                        (*hashT)->items[indexStore].symbol = token;
                        (*hashT)->tableSize++;
                        return indexStore;
                    }else{
                        shift++;
                        continue;
                    }
                }
            }
            if (shift >= (*hashT)->tableCapacity)
            {
                return -1;
            }

            if (strcmp((*hashT)->items[indexStore].key, key) == 0) // klice se rovnaji, aktualizuji pouze hodnotu
            {
                (*hashT)->items[indexStore].symbol = token;
                return indexStore;
            }
            shift++;         
        }

    }else{ // polozka s danym klicem uz v tabulce je
        (*hashT)->items[foundIndex].symbol = token; // zmenim jenom hodnotu
        return foundIndex;  
    }
    
    return -1;  // nepovedlo se nam to
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
int hashTableSearchItem(HashTable *hashT, char *key)
{
    if (hashT == NULL)
    {
        printError(INTERNAL_ERROR, "Tabulka je prazdna\n");
        return -1;
    }
    if (key == NULL)
    {
        printError(INTERNAL_ERROR, "Klic je prazdny\n");
        return -1;
    }

    int keyHash = hashItem(key, hashT);
    int shift = 0;
    while (true)
    {
        int currentIndex = (keyHash + shift) % hashT->tableCapacity;
        if (hashT->items[currentIndex].key == NULL) // klic aktualni prohledavane polozky je nULL
        {
            if (hashT->items[currentIndex].isDeleted == false) // neni smazan, takze jsem nenasel, protoze ukladam za sebe
            {
                return -1; // nenalezl jsem polozku

            }else{ // polozka je smazana a je potreba ji preskocit
                shift++;
                continue;
            }
        }
        
        if (strcmp(hashT->items[currentIndex].key, key) == 0)
        {
            return currentIndex;
        }
        if (currentIndex == keyHash) // prosel jsem to cele
        {
            // printError(INTERNAL_ERROR, "Prvek se zadanym klicem neni v tabulce\n");
            return -1;
        }
        shift++;
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
HashTable *hashTableRealloc(HashTable **hashT)
{
    if (hashT == NULL || (*hashT)->tableCapacity <= 0) {
        printError(INTERNAL_ERROR, "(%s) Tabulka je prazdna", __func__);
        return NULL;
    }

    // navýšení kapacity tabulky o 50%
    int        newCapacity = (*hashT)->tableCapacity + ((*hashT)->tableCapacity / 2);
    HashTable *newTable    = malloc(sizeof(HashTable) + sizeof(TableItem) * newCapacity);
    
    if (newTable == NULL) {
        printError(INTERNAL_ERROR, "Chyba realokace pameti tabulky symbolu");
        return NULL;
    }
    newTable->tableCapacity = newCapacity;
    newTable->tableSize = 0;
    hashTableInitItems(newTable);


    for (int i = 0; i < (*hashT)->tableCapacity; i++) // naplneni nove tabulky polozkami
    {
        if ((*hashT)->items[i].key != NULL)
        {
            hashTableInsertItem(&newTable, (*hashT)->items[i].key, (*hashT)->items[i].symbol);
        }
    }

    (*hashT) = newTable;
    
    return (*hashT);
}

bool hashTableIsEmpty(HashTable *hashT) { return hashT->tableSize == 0; }
/* Konec hashtable.h */