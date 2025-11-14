#!/bin/bash

# --- Konfigurace ---
# Změňte tyto cesty a názvy souborů, pokud je to nutné
IFJ25_EXECUTABLE="./ifj25"
IFJ25_INPUT_FILE="testy/syntax/03_if_with_else.in"
IFJ25_OUTPUT_FILE="prog.out"
IC25INT_EXECUTABLE="/home/tjoslef/Stažené/sys_package/ic25int-linux-x86_64"
IC25INT_INPUT_FILE="$IFJ25_OUTPUT_FILE"
IC25INT_OUTPUT_FILE="prg.out"
# -------------------

echo "🛠️ Spouštění kompilátoru $IFJ25_EXECUTABLE..."
# Krok 1: Spuštění kompilátoru a uložení celého výstupu
# Přesměrujeme stdout i stderr do výstupního souboru
$IFJ25_EXECUTABLE < "$IFJ25_INPUT_FILE" > "$IFJ25_OUTPUT_FILE" 2>&1
IFJ25_EXIT_CODE=$?

if [ $IFJ25_EXIT_CODE -ne 0 ]; then
    echo "❌ Kompilátor selhal s návratovým kódem: $IFJ25_EXIT_CODE"
    echo "Pravděpodobně se jedná o chybu syntaxe/sémantiky. Celý výstup je v $IFJ25_OUTPUT_FILE"
    exit 1
fi
echo "✅ Kompilace dokončena. Výstup uložen do $IFJ25_OUTPUT_FILE"

---

echo "💻 Spouštění interpretu $IC25INT_EXECUTABLE s celým výstupem..."
# Krok 2: Spuštění interpretu s celým výstupním souborem z kroku 1
# IC25INT_INPUT_FILE je nyní alias pro IFJ25_OUTPUT_FILE ("prog.out")
$IC25INT_EXECUTABLE -v "$IC25INT_INPUT_FILE" > "$IC25INT_OUTPUT_FILE" 2>&1
IC25INT_EXIT_CODE=$?

if [ $IC25INT_EXIT_CODE -ne 0 ]; then
    echo "❌ Interpret selhal s návratovým kódem: $IC25INT_EXIT_CODE"
    echo "Podrobnosti ve $IC25INT_OUTPUT_FILE"
    exit 2
fi
echo "✅ Interpretace dokončena. Výstup uložen do $IC25INT_OUTPUT_FILE"

echo ""
echo "🎉 Skript úspěšně dokončen!"
