/**
 * Fakulta informačních technologií VUT Brno
 * 
 * Hlavičkový soubor pro implementaci hashovací tabulky s implicitním zřetězením
 * pro projekt do předmětu IFJ
 * @file hashtable.c
 * @author Petr Kotek (xkotekp00)
 * Upraveno: 04.10.2025
 */
#include "hashtable.h"
#include "error_handler.h"

#define TABLE_CAPACITY 200 // nastavení počtu položek tabulky

/**
 * @brief Inicializace tabulky, alokace pole prvků
 * @param hashT ukazatel na strukturu tabulky
 */
void hashTableInit(HashTable *hashT)
{
    if (hashT == NULL)
    {
        printError(INTERNAL_ERROR, true, "Tabulka neni alokovana");
        return;
    }
    // nastaveni poctu aktualnich prvku na zadny
    hashT->tableSize = 0;
    // Nastavení maximální kapacity tabulky (může být měněna)
    hashT->tableCapacity = TABLE_CAPACITY;

    // alokace pole prvků tabulky
    hashT->items = malloc(sizeof(TableItem) * hashT->tableCapacity);
    if (hashT->items == NULL)
    {
        printError(INTERNAL_ERROR, true, "Chyba alokace prvku tabulky");
        return;
    }

    // inicializace prvků tabulky na defaultní hodnoty
    hashTableInitItems(hashT);
}

void hashTableInitItems(HashTable *hashT)
{
    for (int indexOfItem = 0; indexOfItem < hashT->tableCapacity; indexOfItem++)
    {
        hashT->items[indexOfItem].key = NULL; // alokace řetězce proběhne jakmile bude to prvku pole vkládáno
        hashT->items[indexOfItem].isEmpty = true;
        hashT->items[indexOfItem].isDeleted = false;
        hashT->items[indexOfItem].token.type = NULL;
    }
}

void hashTableInsertItem(HashTable *hashT, const char *key, Token token)
{
    int indexToStore = hashItem(key);

    // na indexu položky už nějaký prvek je
    if (hashT->items[indexToStore].isEmpty == false)
    {

        // TODO

    }else{
        TableItem  item = hashT->items[indexToStore];
        item.key = malloc(sizeof(char) * (strlen(key) + 1));
        if (item.key == NULL)
        {
            printError(INTERNAL_ERROR, true, "Chyba alokace klice prvku v tabulce symbolu pri vkladani");
            return;
        }
        item.key = *key;
        item.isDeleted = false;
        item.isEmpty = false;
        item.token = token;
    }
}
/**
 * @brief Hashovací funkce 
 * @return Vrací index v poli struktur, který je využit při hledání, mazání, vkládání
 */
int hashItem(const char *key)
{
    if (key == NULL)
    {
        printError(INTERNAL_ERROR, true, "Nelze hashovat NULL prvek");
        return INTERNAL_ERROR;
    }

    unsigned long hash = 5381; 
    int c;


    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; 
    }


    return hash % TABLE_CAPACITY;
}
/* Konec hashtable.h */