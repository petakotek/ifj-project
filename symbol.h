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
#include "lexer.h"   // musí obsahovať: TokenType, TokenValue (union), Token

/**
 * @brief Štruktúra terminalov/neterminálov vkladaná na zásobník
 */
typedef struct Symbol
{
    bool is_terminal; 
    int symbol; 
    TokenValue value;
    bool value_needed;
} Symbol;

#endif // SYMBOL_H