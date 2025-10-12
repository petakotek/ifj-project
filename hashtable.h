/**
 * Fakulta informačních technologií VUT Brno
 * 
 * Hlavičkový soubor pro implementaci hashovací tabulky s implicitním zřetězením
 * pro projekt do předmětu IFJ
 * @file hashtable.h
 * @author Petr Kotek (xkotekp00)
 * Upraveno: 04.10.2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lexer.h"
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
void hashTableInitItems(HashTable *);
void hashTableInsertItem(HashTable *, const char *key, Token token);
void hashTableDeleteItem(HashTable *, const char *key);
void hashTableSearchItem(HashTable *, const char *key);
int hashItem(const char *key);

/* Konec hashtable.h */