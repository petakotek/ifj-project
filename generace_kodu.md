# Generování kódu

## Workflow codegenu

Máme funkci:

```c
void codegen_generate(ASTNode *root, SymbolTable *symtab);
```

- Tato funkce spouští celý codegen.
- Nejprve zavolá:

```c
emit_seznam(ASTNode *root, SymbolTable *symtab);
```


ve funkci emit_seznam pro každé dítě  zleva v AST postupně voláme funkci tahle funkce bude volana v kazde funkci jelikoz jedeme postORder:

```c
void emit_command(ASTNode *cmd, SymbolTable *symtab);
```

Tato funkce obsahuje hlavní `switch` blok zajišťující pouziti šablony podle typu uzlu:

```c
void emit_command(ASTNode *cmd, SymbolTable *symtab) {
    switch (cmd->type) {
        case ASSIGNMENT:
            emit_assignment(cmd, symtab);
            break;
        case IF:
            emit_if(cmd, symtab);
            break;
        case WHILE:
            emit_while(cmd, symtab);
            break;
        case FUNCALL:
            emit_funcall(cmd, symtab);
            break;
        case BLOCK:
            // Blok je opět spojení příkazů → rekurzivní průchod poduzly
            for (ASTNode *subcmd = cmd->block_first_cmd; subcmd; subcmd = subcmd->next_cmd)
                emit_command(subcmd, symtab);
            break;
        // ... další konstrukce dle potřeby
    }
}
```
- Pro každý poduzel stromu tak zavoláme odpovídající emitační funkci, často rekurzivně.

***

## Dynamická kontrola typu

Nemame implicitni konverze zakazany vestavene funkce na zvladnuti konverze

Pro každou operaci, která vyžaduje specifický typ, vložíme instrukce pro běhové kontroly.
Ve chvíli, kdy potkáme něco jako `x = a + b`, vkládáme typovou kontrolu, kterou interpret provede za běhu.
Je to podobné jako v assembleru, využívají se skoky:

```c
void emit_arithmetic(ASTNode *expr, SymbolTable *symtab) {
    emit_expression(expr->left, symtab);   // generuj kód pro levý operand, výsledek v GF@temp1
    emit_expression(expr->right, symtab);  // generuj kód pro pravý operand, výsledek v GF@temp2

    // Kontrola typu levého operandu
    printf("TYPE GF@type_left GF@temp1\n");
    // Pomocí skoku se přesuneme na label check_right a přeskočíme exit, který program ukončí při špatném typu
    printf("JUMPIFEQ $check_right_type GF@type_left string@int\n");
    printf("JUMPIFEQ $check_right_type GF@type_left string@float\n");
    printf("EXIT int@25\n");  // Chyba: levý operand není číslo
    macro_jumpifeq()
    printf("LABEL $check_right_type\n");

    // Běhová kontrola typu pravého operandu
    printf("TYPE GF@type_right GF@temp2\n");
    printf("JUMPIFEQ $do_op GF@type_right string@int\n");
    printf("JUMPIFEQ $do_op GF@type_right string@float\n");
    printf("EXIT int@25\n"); // Chyba: pravý operand není číslo

    printf("LABEL $do_op\n");

    // Emise instrukce aritmetické operace (např. ADD)
    printf("ADD GF@result GF@temp1 GF@temp2\n");
}
```

***

## Globální proměnné

Důležité je, aby globální proměnné byly inicializovány na `nil` pomocí funkce `emit_globals(symtab);` abychom předešli běhovým chybám.

***

## Konvence volání funkcí

Funkování konvence volání.
Pouze ve volani funkci vytvarim novy ramec lokalni ramec LF

**Volající (caller):**

```text
CREATEFRAME         # vytvoření nového dočasného rámce pro parametry
DEFVAR TF@param1
MOVE TF@param1 GF@x # předání hodnoty parametru
CALL my_function    # volání funkce

# Po návratu funkce je návratová hodnota v TF@retval
MOVE GF@result TF@retval  # uložení návratové hodnoty do GF
```

**Volaná funkce (callee):**

```text
LABEL my_function
PUSHFRAME            # TF → LF (aktivní lokální rámec)
DEFVAR LF@param1     # definice parametrů v LF
DEFVAR LF@retval     # definice návratové hodnoty

# ... tělo funkce ...

MOVE LF@retval int@42  # nastavení návratové hodnoty
POPFRAME             # ukončení rámce
RETURN               # návrat volajícímu
```

***

## Labely

Je dobré, aby každý LABEL měl jedinečný název. Často se to řeší přidáváním čítače na konec názvu.
to zajisuje funkce
```
char *generatorLable(char * prefix);
```

**Příklad if/else:**

```IFJcode25
JUMPIFEQ if_else_1 GF@cond bool@false
# kód then větve
JUMP if_end_1
LABEL if_else_1
# kód else větve
LABEL if_end_1
```

**Příklad while smyčky (for se budou tvořit obdobně):**

```IFJcode25
LABEL while_start_1
# vyhodnocení podmínky do GF@cond
JUMPIFEQ while_end_1 GF@cond bool@false
# tělo cyklu
JUMP while_start_1
LABEL while_end_1
```

***

## Vestavěné funkce

Vestavěné funkce dostupné v jazyce odpovídají instrukcím IFJcode25 nebo jejich kombinacím podobně jako v ISU, kde některé funkce už byly implementovány.

| Vestavěná funkce        | IFJcode25 instrukce/sekvence                  |
|-------------------------|-----------------------------------------------|
| Ifj.readStr()            | READ pro čtení stringu z inputu                |
| Ifj.readNum()            | READ pro čtení čísla (float či int)            |
| Ifj.write(t)             | WRITE pro výstup na stdout                      |
| Ifj.length(s)            | STRLEN pro délku stringu                        |
| Ifj.strCmp(s1,s2)        | STRI2INT a smyčka/výraz pro porovnání          |
| Ifj.substring(s,i,j)     | Sekvence s bezpečnými kontrolami a výběrem podřetězce |
| Ifj.ord(s,i)             | STRI2INT s kontrolou indexu                     |
| Ifj.chr(i)               | INT2CHAR s kontrolou validní hodnoty           |
| Převody typů             | INT2FLOAT, FLOAT2INT, INT2STR, FLOAT2STR       |

## Rozdeleni:
Jeden clovek ma nastarost celkove aritmetické operace/porovnani a kontrola typu

Druhy clovek vytvoreni for,while,if-else,funkce (delani hops hops na labels)
je tam funkce na generovani labelu kterou jsem tak nejak testoval

Treti clovek vytvoreni getteru a setteru(vubec netusim jak je udelat) a scopy (bloky  vytvareni novych ramcu pro kazdy scopy atd)
