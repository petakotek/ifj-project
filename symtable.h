/**
 * Fakulta informačních technologií VUT Brno
 *
 * @brief Hlavičkový soubor pro implementaci hash tabulky s implicitním zřetězením
 * pro projekt do předmětu IFJ
 * @file symtable.h
 * @author Petr Kotek (xkotekp00)
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

#include "semantic_analysis.h"

typedef enum{
    ITEM_NOT_FOUND = -1,
    INSERT_ERROR = -2
} SymTableErrors;
/**
 * @brief Struktura položky v hash tabulce
 */
typedef struct {
    char     *key;      // identifikator
    SemSymbol symbol;   // hodnota toho tokenu
    bool      isEmpty;  // Proměnná sloužící pro ověření, zda je položka již obsazena nějakým prvkem nebo
                        // ne
    bool isDeleted;     //

} TableItem;

/**
 * @brief Struktura hash tabulky
 * @param items Pole položek v hash tabulce
 * @param tableSize Aktuální velikost tabulky
 * @param tableCapacity kapacita tabulky, která může být použita
 */
typedef struct {
    int tableSize;             /** Aktuální velikost tabulky @param tableCapacity Kapacita tabulky nemůže být
                                  větší jak její velikost */
    int        tableCapacity;  // Kapacita tabulky, která může být využita
    TableItem *items;          // Pole položek v Hash tabulce
} HashTable;

HashTable *hashTableInit();
void       hashTableInitItems(HashTable *);  // Funkce pro základní inicializaci
int        hashTableInsertItem(HashTable *, char *key,
                               SemSymbol token);   // Funkce pro vložení položky do hash tabulky s
                                                   // alokací řetězce, který bude vkládán
void hashTableDeleteItem(HashTable *, char *key);  // Funkce pro smazání položky v hash tabulce
int  hashTableSearchItem(HashTable *, char *key);  // Funkce pro nalezení položky v hash tabulce
void hashTableDealloc(HashTable *);                // funkce pro dealokaci hash tabulky
int  hashItem(char *key, HashTable *);
void hashTableRealloc(HashTable *);  // Funkce provede realokaci tabulky symbolu v pripade,
                                     // ze doslo k naplneni kapacity

/**
 * @brief Ověření zda je tabulka prázdná
 *
 * @param hashT Ukazatel na tabulku
 * @return true Ano je prázdná
 * @return false Ne není prázdná
 */
bool hashTableIsEmpty(HashTable *hashT);

#endif  // SYMTABLE_H

/* Konec hashtable.h */