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

typedef enum {
    NK_Program, NK_Prolog,
    NK_String, NK_Int, NK_Float,
    // ... ďalšie (NK_MainClass, NK_Block, NK_Expr, ...)
} NodeKind;

/**
 * @brief Štruktúra terminalov/neterminálov vkladaná na zásobník
 */
typedef struct Symbol
{
    bool is_terminal; 
    int symbol; 
    TokenValue value;
    bool value_needed;
    bool is_reduce;
    size_t reduce_arity;     // koľko detí zozbierať
    NodeKind reduce_kind; 
} Symbol;



#endif // SYMBOL_H