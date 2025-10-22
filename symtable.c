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

#define TABLE_CAPACITY 200 // nastavení počtu položek tabulky

/**
 * @brief Inicializace tabulky, alokace pole prvků
 * @param hashT ukazatel na strukturu tabulky
 */
void hashTableInit(HashTable *hashT)
{
    if (hashT == NULL)
    {
        printError(INTERNAL_ERROR, "Tabulka neni alokovana");
    }
    // nastaveni poctu aktualnich prvku na zadny
    hashT->tableSize = 0;
    // Nastavení maximální kapacity tabulky (může být měněna)
    hashT->tableCapacity = TABLE_CAPACITY;

    // alokace pole prvků tabulky
    hashT->items = malloc(sizeof(TableItem) * hashT->tableCapacity);
    if (hashT->items == NULL)
    {
        printError(INTERNAL_ERROR, "Chyba alokace prvku tabulky");
    }

    // inicializace prvků tabulky na defaultní hodnoty
    hashTableInitItems(hashT);
}
/**
 * @brief Funkce inicializuje všechny prvky v tabulce na defaultní hodnoty s vyjímkou pole key
 * @param hashT Ukazatel na hashovací tabulku
 */
void hashTableInitItems(HashTable *hashT)
{
    for (int indexOfItem = 0; indexOfItem < hashT->tableCapacity; indexOfItem++)
    {
        hashT->items[indexOfItem].key = NULL; // alokace řetězce proběhne jakmile bude to prvku pole vkládáno
        hashT->items[indexOfItem].isEmpty = true;
        hashT->items[indexOfItem].isDeleted = false;
    }
}
/**
 * @brief Funkce vloží prvek do pole položek hashovací tabulky a vrátí index, na kterém byla položka uložena.
 * V případě, že index, který vypočítala hashovací funkce je již obsazený postupuje dál polem a hledá první volný index
 * @return Funkce vraci index v poli polozek
 */
int hashTableInsertItem(HashTable *hashT, char *key, Token token)
{
    int indexToStore = hashItem(key); // zahashovani retezce na index v poli polozek hashovaci tabulky
    if (hashT->tableSize >= hashT->tableCapacity)
    {
        hashTableRealloc(hashT);
    }
    
    // na indexu položky už nějaký prvek je
    if (hashT->items[indexToStore].isEmpty == false)
    {
        int shift = 1; // o kolik prvku oproti puvodnimu indexu je treba se posunout aby se nasel volny index

        while (hashT->items[indexToStore + shift].isEmpty != true) // linearni hledani volneho mista
        {
            shift++;
        }

        hashT->items[indexToStore + shift].key = malloc(sizeof(char) * (strlen(key) + 1)); // alokace řetězce, který bude vkládán do hashovací tabulky

        if (hashT->items[indexToStore + shift].key == NULL) // kontrola alokace
        {
            printError(INTERNAL_ERROR, "Chyba alokace klice prvku v tabulce symbolu pri vkladani");
        }

        strcpy(hashT->items[indexToStore + shift].key, key);
        hashT->items[indexToStore + shift].isDeleted = false;
        hashT->items[indexToStore + shift].isEmpty = false;
        hashT->items[indexToStore + shift].token = token;

        hashT->tableSize++;
        return indexToStore + shift;

    }else{

        hashT->items[indexToStore].key = malloc(sizeof(char) * (strlen(key) + 1)); // alokace řetězce, který bude vkládán do hashovací tabulky

        if (hashT->items[indexToStore].key == NULL) // kontrola alokace
        {
            printError(INTERNAL_ERROR, "Chyba alokace klice prvku v tabulce symbolu pri vkladani");
        }
        // nastavení hodnot jednotlivým prvkům
        strcpy(hashT->items[indexToStore].key, key);
        hashT->items[indexToStore].isDeleted = false;
        hashT->items[indexToStore].isEmpty = false;
        hashT->items[indexToStore].token = token;

        hashT->tableSize++;

        return indexToStore;
    }

}
/**
 * @brief Funkce smaže prvek v poli v tabulce, pokud je nalezen
 * @return Funkce nemá návratovou hodnotu, v případě nenalezení prvku v tabulce tiskne error
 */
void hashTableDeleteItem(HashTable *hashT, char *key)
{
    int indexToSearch = hashItem(key);

    if (hashT->items[indexToSearch].key != NULL && !strcmp(key, hashT->items[indexToSearch].key))
    {
        free(hashT->items[indexToSearch].key);
        hashT->items[indexToSearch].key = NULL;
        hashT->items[indexToSearch].isDeleted = true;
        hashT->items[indexToSearch].isEmpty = true;

    }else{
        int shift = 1;
        bool rounded = false;
        while (hashT->items[indexToSearch + shift].key != NULL && strcmp(key, hashT->items[indexToSearch + shift].key)) // prohledavani dokud se nenarazi na matchujici klic
        {
            shift++;
            rounded = true; // pomocná proměnná pro kontrolu, že nedochází k průchodu pole stále dokola
            if ((indexToSearch + shift) == TABLE_CAPACITY && indexToSearch != 0)
            {
                indexToSearch = 0;
                rounded = true; // pomocná proměnná pro kontrolu, že nedochází k průchodu pole stále dokola

            }else if (rounded && (indexToSearch + shift) == TABLE_CAPACITY)
            {
                printError(INTERNAL_ERROR, "Prvek v tabulce nebyl nalezen");
                return;
            }
        }
        free(hashT->items[indexToSearch + shift].key);
        hashT->items[indexToSearch + shift].key = NULL;
        hashT->items[indexToSearch + shift].isDeleted = true;
        hashT->items[indexToSearch + shift].isEmpty = true;
    }
    hashT->tableSize--;
}

/**
 * @brief Funkce pro vyhledani polozky v tabulce symbolu
 * @return Funkce vraci index v poli hashovaci tabulky
 */
int hashTableSearchItem(HashTable *hashT, char *key)
{
    int indexToSearch = hashItem(key);

    if (!strcmp(key, hashT->items[indexToSearch].key))
    {
        return indexToSearch;

    }else{
        int shift = 1;
        bool rounded = false;
        while (abs(strcmp(key, hashT->items[indexToSearch + shift].key))) // prohledavani dokud se nenarazi na matchujici klic
        {
            shift++;

            if ((indexToSearch + shift) == TABLE_CAPACITY && indexToSearch != 0)
            {
                rounded = true; // pomocná proměnná pro kontrolu, že nedochází k průchodu pole stále dokola
                indexToSearch = 0;

            }else if (rounded && (indexToSearch + shift) == TABLE_CAPACITY)
            {
                printError(INTERNAL_ERROR, "Prvek v tabulce nebyl nalezen");
            }
        }
        return indexToSearch + shift;
    }

}
/**
 * @brief Funkce sloužící pro uvolnění paměti tabulky symbolů
 * @param hashT Ukazatel na tabulku symbolů, která bude uvolněna
 *
 */
void hashTableDealloc(HashTable *hashT)
{
    if (hashT == NULL)
    {
        printError(INTERNAL_ERROR, "Pokud o uvolneni neinicializovane tabulky");
    }
    int indexToDelete = 0; // index prvku, který bude uvolněn, pokud na něm něco je
    if (hashT->items != NULL)
    {
        for (int i = 0; i < TABLE_CAPACITY; i++)
        {
            if (hashT->items[indexToDelete].key != NULL)
            {
                free(hashT->items[indexToDelete].key); // uvolneni retezce polozky v tabulce
            }
            indexToDelete++;
        }
        free(hashT->items); // uvolneni pole struktur tabulky
    }
}
/**
 * @brief Hashovací funkce
 * @param key ukazatel na řetězec, který bude vkládán
 * @return Vrací index v poli struktur, který je využit při hledání, mazání, vkládání
 */
int hashItem(char *key)
{
    if (key == NULL)
    {
        printError(INTERNAL_ERROR, "Nelze hashovat NULL prvek");
        return INTERNAL_ERROR;
    }

    unsigned long hash = 5381;
    int c;


    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }


    return hash % TABLE_CAPACITY;
}

/**
 * @brief Funkce pro realokaci pole položek tabulky symbolů, je volána z funkce hashTableInsertItem
 * @param hashT je ukazatel na tabulku, která bude alokována
 * @return nemá návratovou hodnotu
 */
void hashTableRealloc(HashTable *hashT)
{
    if (hashT == NULL || hashT->tableCapacity <= 0)
    {
        return;
    }

    // navýšení kapacity tabulky o 50%
    int newCapacity = hashT->tableCapacity + (hashT->tableCapacity / 2);

    // Alokace noveho pole
    TableItem *newItems = realloc(hashT->items, sizeof(TableItem) * newCapacity);
    if (newItems == NULL)
    {
        return;
    }
    
    // Inicializace novych prvku
    for (int i = hashT->tableCapacity; i < newCapacity; i++)
    {
        newItems[i].key = NULL;
        newItems[i].isDeleted = false;
        newItems[i].isEmpty = true;
    }

    hashT->items = newItems;
    hashT->tableCapacity = newCapacity;
}
/* Konec hashtable.h */
