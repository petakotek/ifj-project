// TODO hlavička

#include "symtable.h"
#include "support_functions.h"
#include "ast_strom.h"
#include "dynamic_stack.h"

// TODO doxygen
void startSematicValidation(AstNode* root);

//TODO doxygen
void freeSematicsMemory();

// TODO uložené hodnoty co potřebuji v hash table
typedef struct {
    char* key;
    //asi enum dataType
    // A další věci (co nepotřebuji bude UB a nebude mě to zajímat (viz dataType)
    // asi bool určují zda je to volání nedeklarované (to bude v samostatné tabulce)
    // scope (úroveň v zásobníku)

} idk_jak_to_pojmenovat; // TODO

// TODO patička