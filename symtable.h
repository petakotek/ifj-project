/**
 * Fakulta informačních technologií VUT Brno
 * 
 * Hlavičkový soubor pro implementaci hashovací tabulky s implicitním zřetězením
 * pro projekt do předmětu IFJ
 * @file hashtable.h
 * @author Petr Kotek (xkotekp00)
 * Upraveno: 19.10.2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "support_functions.h"


/**
 * @brief Struktura položky v hashovací tabulce
 */
typedef struct
{
    char *key; // identifikator
    Token token; // hodnota toho tokenu
    bool isEmpty; // Proměnná sloužící pro ověření, zda je položka již obsazena nějakým prvkem nebo ne
    bool isDeleted; // 
    // implementovat pole jako kruhove, operace % 

} TableItem;

/**
 * @brief Struktura hashovaci tabulky
 * @param items Pole položek v hashovací tabulce
 * @param tableSize Aktuální velikost tabulky
 * @param tableCapacity kapacita tabulky, která může být použita
 */
typedef struct
{
    TableItem *items; // Pole položek v Hashovací tabulce
    int tableSize; /** Aktuální velikost tabulky @param tableCapacity Kapacita tabulky nemůže být větší jak její velikost !!! */ 
    int tableCapacity; // Kapacita tabulky, která může být využita

} HashTable, *HashTablePtr;

void hashTableInit(HashTable *);
void hashTableInitItems(HashTable *); // Funkce pro základní inicializaci 
int hashTableInsertItem(HashTable *, char *key, Token token); // Funkce pro vložení položky do hashovací tabulky s alokací řetězce, který bude vkládán
void hashTableDeleteItem(HashTable *, char *key); // Funkce pro smazání položky v hashovací tabulce
int hashTableSearchItem(HashTable *, char *key); // Funkce pro nalezení položky v hashovací tabulce
void hashTableDealloc(HashTable *); // funkce pro dealokaci hashovací tabulky
int hashItem(char *key);
void hashTableRealloc(HashTable *); // Funkce provede reallokaci tabulky symbolu v pripade, ze doslo k naplneni kapacity

#endif // SYMTABLE_H
/* Konec hashtable.h */