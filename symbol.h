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
#include "ast_strom.h"



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
    AstNodeType reduce_kind;
    struct AstNode *ast_ptr;
    bool is_astnode;
    bool capture_value;
} Symbol;
#endif // SYMBOL_H
