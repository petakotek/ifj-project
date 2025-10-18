/**
 * Fakulta informačních technologií VUT Brno
 *
 * Soubor pro implementaci syntaktické analýzy překladače
 * @file analysis.c
 * @author Petr Kotek & ...
 */


/**
 * @brief Funkce vracící hodnotu priority operátor od 5 do 1, 5 značí největší prioritu
 * @param op Operátor, který bude zkoumán
 * @return Vrací prioritu operátoru, id 5 po 1, 5 značí nejvyšší prioritu
 */
int get_precedence(const char* op) {
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 5;  // Priorita 1 -> 5
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 4;  // Priorita 2 -> 4
    if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 || strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) return 3;  // Priorita 3 -> 3
    if (strcmp(op, "is") == 0) return 2;  // Priorita 4 -> 2
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) return 1;  // Priorita 5 -> 1
    return 0;  // Nejedná se o validní operátor
}