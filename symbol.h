/**
 * Fakulta informačných technologií VUT Brno
 *
 * Hlavičkový súbor pre implementáciu lexikálnej analýzy
 * konkrétne pre struktúru symbolov pre terminalý a neterminály
 * pre projekt do predmetu IFJ
 * @file  parser.h
 * @author Jakub Jambrozi (xjambrj00)
 * upravované: 22.10.2025
 */

#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>

#include "ast_strom.h"
#include "lexer.h"

/**
 * @brief Štruktúra terminalov/neterminálov vkladaná na zásobník
 */
typedef struct Symbol {
    bool        isTerminal;
    int         symbol;
    TokenValue  value;
    bool        valueNeeded;
    bool        isReduce;
    size_t      reduceArity;  // koľko detí zozbierať
    AstNodeType reduceKind;
    bool        captureValue;
    bool        freeNeeded;
} Symbol;

#endif
