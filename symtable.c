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
        hashT->items[indexOfItem].key =
            NULL;  // alokace řetězce proběhne jakmile bude to prvku pole vkládáno
        hashT->items[indexOfItem].isEmpty     = true;
        hashT->items[indexOfItem].isDeleted   = false;
        hashT->items[indexOfItem].symbol.name = NULL;
    }
}

// TODO v celém symtable.* opravit názvy proměnných a přesunout dokumentační komentáře na správné
// místo a obecná oprava chování, hashTableInsertItem neumí signalizovat že došlo k chybě (index =
// -1)
/**
 * @brief Funkce vloží prvek do pole položek hashovací tabulky a vrátí index, na kterém byla položka
 * uložena. V případě, že index, který vypočítala hashovací funkce je již obsazený postupuje dál
 * polem a hledá první volný index
 * @return Funkce vraci index v poli polozek
 */
int hashTableInsertItem(HashTable *hashT, char *key, SemanticSymbol token) {
    int indexToStore =
        hashItem(key);  // zahashovani retezce na index v poli polozek hashovaci tabulky
    if (hashT->tableSize >= hashT->tableCapacity) {
        hashTableRealloc(hashT);
    }

    // na indexu položky už nějaký prvek je
    if (hashT->items[indexToStore].isEmpty == false) {
        int shift = 1;  // o kolik prvku oproti puvodnimu indexu je treba se posunout aby se nasel
                        // volny index

        while (hashT->items[indexToStore + shift].isEmpty !=
               true)  // linearni hledani volneho mista
        {
            shift++;
        }

        hashT->items[indexToStore + shift].key =
            malloc(sizeof(char) *
                   (strlen(key) + 1));  // alokace řetězce, který bude vkládán do hashovací tabulky

        if (hashT->items[indexToStore + shift].key == NULL)  // kontrola alokace
        {
            printError(INTERNAL_ERROR, "Chyba alokace klice prvku v tabulce symbolu pri vkladani");
        }

        strcpy(hashT->items[indexToStore + shift].key, key);
        hashT->items[indexToStore + shift].isDeleted = false;
        hashT->items[indexToStore + shift].isEmpty   = false;
        hashT->items[indexToStore + shift].symbol    = token;

        hashT->tableSize++;
        return indexToStore + shift;

    } else {
        hashT->items[indexToStore].key =
            malloc(sizeof(char) *
                   (strlen(key) + 1));  // alokace řetězce, který bude vkládán do hashovací tabulky

        if (hashT->items[indexToStore].key == NULL)  // kontrola alokace
        {
            printError(INTERNAL_ERROR, "Chyba alokace klice prvku v tabulce symbolu pri vkladani");
        }
        // nastavení hodnot jednotlivým prvkům
        strcpy(hashT->items[indexToStore].key, key);
        hashT->items[indexToStore].isDeleted = false;
        hashT->items[indexToStore].isEmpty   = false;
        hashT->items[indexToStore].symbol    = token;

        hashT->tableSize++;

        return indexToStore;
    }
}
/**
 * @brief Funkce smaže prvek v poli v tabulce, pokud je nalezen
 * @return Funkce nemá návratovou hodnotu, v případě nenalezení prvku v tabulce tiskne error
 */
void hashTableDeleteItem(HashTable *hashT, char *key) {
    int indexToSearch = hashItem(key);

    if (hashT->items[indexToSearch].key != NULL && !strcmp(key, hashT->items[indexToSearch].key)) {
        free(hashT->items[indexToSearch].key);
        hashT->items[indexToSearch].key       = NULL;
        hashT->items[indexToSearch].isDeleted = true;
        hashT->items[indexToSearch].isEmpty   = true;

    } else {
        int  shift   = 1;
        bool rounded = false;
        while (hashT->items[indexToSearch + shift].key != NULL &&
               strcmp(key, hashT->items[indexToSearch + shift]
                               .key))  // prohledavani dokud se nenarazi na matchujici klic
        {
            shift++;
            rounded =
                true;  // pomocná proměnná pro kontrolu, že nedochází k průchodu pole stále dokola
            if ((indexToSearch + shift) == TABLE_CAPACITY && indexToSearch != 0) {
                indexToSearch = 0;
                rounded = true;  // pomocná proměnná pro kontrolu, že nedochází k průchodu pole
                                 // stále dokola

            } else if (rounded && (indexToSearch + shift) == TABLE_CAPACITY) {
                printError(INTERNAL_ERROR, "Prvek v tabulce nebyl nalezen");
                return;
            }
        }
        free(hashT->items[indexToSearch + shift].key);
        hashT->items[indexToSearch + shift].key       = NULL;
        hashT->items[indexToSearch + shift].isDeleted = true;
        hashT->items[indexToSearch + shift].isEmpty   = true;
        hashT->items[indexToSearch + shift].symbol.name =
            NULL;  // Uvolnění paměti zajišťují funkce pro AS strom
    }
    hashT->tableSize--;
}

/**
 * @brief Funkce pro vyhledani polozky v tabulce symbolu
 * @return Funkce vraci index v poli hashovaci tabulky, pokud nenajde vraci -1
 */
int hashTableSearchItem(HashTable *hashT, char *key) {
    int  indexToSearch = hashItem(key);
    int  shift         = 1;
    bool rounded       = false;
    int  originalIndex = indexToSearch;

    // test jestli neni rovnou na prvni pozici
    if (hashT->items[indexToSearch].key != NULL && !strcmp(key, hashT->items[indexToSearch].key)) {
        return indexToSearch;
    }

    while (true) {
        shift++;
        int currentIndex = (indexToSearch + shift) % hashT->tableCapacity;

        // Pokud narazíme na prázdnou pozici, klíč není v tabulce
        if (hashT->items[currentIndex].key == NULL) {
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
    if (hashT == NULL) {
        printError(INTERNAL_ERROR, "Pokud o uvolneni neinicializovane tabulky");
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
int hashItem(char *key) {
    if (key == NULL) {
        printError(INTERNAL_ERROR, "Nelze hashovat NULL prvek");
        return INTERNAL_ERROR;
    }

    unsigned long hash = 5381;
    int           c;

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
