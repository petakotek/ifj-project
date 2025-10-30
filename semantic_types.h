// TODO hlavička

#ifndef SEMANTIC_TYPES
#define SEMANTIC_TYPES 1

#include <stddef.h>

typedef enum {
    VARIABLE,
    GLOBAL,
    FUNCTION,
    SETTER,
    GETTER,
} SemanticSymbolTypes;

typedef enum {
NUMBER_VAL,
STRING_VAL,
BOOLEAN_VAL,
NULL_VAL,
} SemanticSymbolDataType;

typedef enum {
    OK,
    ERROR,
    NUMBER_DATA,
    STRING_DATA,
    GOT_NODE_NULL,
} RecursiveSemanticAnalysisReturnValues;

// TODO uložené hodnoty co potřebuji v hash table
typedef struct {
    char* name;
    SemanticSymbolTypes symbolType;
    unsigned scope;
    SemanticSymbolDataType dataType;
    size_t paramCount;
} SemanticSymbol;

#endif
// TODO patička