/**
 * Fakulta informačních technologií VUT Brno
 *
 * Hlavičkový soubor pro implementaci hashovací tabulky s implicitním zřetězením
 * pro projekt do předmětu IFJ
 * @file hashtable.h
 * @author Petr Kotek (xkotekp00)
 * Upraveno: 19.10.2025
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "semantic_analysis.h"

#include <stdbool.h>

/**
 * @brief Struktura položky v hashovací tabulce
 */
typedef struct {
    char          *key;     // identifikator
    SemSymbol symbol;  // hodnota toho tokenu
    bool isEmpty;  // Proměnná sloužící pro ověření, zda je položka již obsazena nějakým prvkem nebo
                   // ne
    bool isDeleted;  //
    // implementovat pole jako kruhove, operace %

} TableItem;

/**
 * @brief Struktura hashovaci tabulky
 * @param items Pole položek v hashovací tabulce
 * @param tableSize Aktuální velikost tabulky
 * @param tableCapacity kapacita tabulky, která může být použita
 */
typedef struct {
    int tableSize; /** Aktuální velikost tabulky @param tableCapacity Kapacita tabulky nemůže být
                      větší jak její velikost */
    int       tableCapacity;  // Kapacita tabulky, která může být využita
    TableItem *items;        // Pole položek v Hashovací tabulce
} HashTable;

HashTable *hashTableInit();
void       hashTableInitItems(HashTable *);  // Funkce pro základní inicializaci
int        hashTableInsertItem(HashTable *, char *key,
                               SemSymbol token);  // Funkce pro vložení položky do hashovací tabulky s
                                                       // alokací řetězce, který bude vkládán
void hashTableDeleteItem(HashTable *, char *key);  // Funkce pro smazání položky v hashovací tabulce
int hashTableSearchItem(HashTable *, char *key);  // Funkce pro nalezení položky v hashovací tabulce
void       hashTableDealloc(HashTable *);         // funkce pro dealokaci hashovací tabulky
int        hashItem(char *key, HashTable *);
void hashTableRealloc(HashTable *);  // Funkce provede reallokaci tabulky symbolu v pripade,
                                           // ze doslo k naplneni kapacity

                                           /**
 * @brief Ověření zda je tabulka prázdná
 * 
 * @param hashT Ukazatel na tabulku
 * @return true Ano je prázdná
 * @return false Ne není prázdná
 */
bool hashTableIsEmpty(HashTable * hashT);
// TODO obecně uklidit symtable.* a podepsat se

#endif  // SYMTABLE_H

/* Konec hashtable.h */