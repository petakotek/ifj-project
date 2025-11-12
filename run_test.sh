#!/bin/bash

# --- Konfigurace ---
# Změňte tyto cesty a názvy souborů, pokud je to nutné
IFJ25_EXECUTABLE="./ifj25"
IFJ25_INPUT_FILE="testy/syntax/01_simple_main.in"
IFJ25_OUTPUT_FILE="prog.out"
IFJCODE_TEMP_FILE="test.in"
IC25INT_EXECUTABLE="/home/tjoslef/Stažené/sys_package/ic25int-linux-x86_64"
IC25INT_OUTPUT_FILE="prg.out"

# Slovo, od kterého se má kopírovat
MAGIC_WORD=".IFJcode25"
# -------------------

echo "🛠️ Spouštění kompilátoru $IFJ25_EXECUTABLE..."
# Krok 1: Spuštění kompilátoru a uložení výstupu
$IFJ25_EXECUTABLE < "$IFJ25_INPUT_FILE" > "$IFJ25_OUTPUT_FILE" 2>&1
IFJ25_EXIT_CODE=$?

if [ $IFJ25_EXIT_CODE -ne 0 ]; then
    echo "❌ Kompilátor selhal s návratovým kódem: $IFJ25_EXIT_CODE"
    echo "Podrobnosti ve $IFJ25_OUTPUT_FILE"
    exit 1
fi
echo "✅ Kompilace dokončena. Výstup uložen do $IFJ25_OUTPUT_FILE"


echo "🔍 Extrahování kódu $MAGIC_WORD do $IFJCODE_TEMP_FILE..."
# Krok 2: Extrahování kódu .IFJcode25 od nalezeného slova dolů
# Používáme 'awk' k nalezení slova a následnému tisku všech řádků
awk '/'"$MAGIC_WORD"'/ {p=1} p' "$IFJ25_OUTPUT_FILE" > "$IFJCODE_TEMP_FILE"
# Kontrola, zda se kód úspěšně extrahoval
if [ ! -s "$IFJCODE_TEMP_FILE" ]; then
    echo "⚠️ Slovo $MAGIC_WORD nebylo nalezeno v $IFJ25_OUTPUT_FILE nebo je soubor prázdný."
    exit 1
fi
echo "✅ Kód extrahován do $IFJCODE_TEMP_FILE"


echo "💻 Spouštění interpretu $IC25INT_EXECUTABLE..."
# Krok 3: Spuštění interpretu s extrahovaným kódem
# '-v' (volitelné, ale ponecháno dle požadavku)
$IC25INT_EXECUTABLE -v "$IFJCODE_TEMP_FILE" > "$IC25INT_OUTPUT_FILE" 2>&1
IC25INT_EXIT_CODE=$?

if [ $IC25INT_EXIT_CODE -ne 0 ]; then
    echo "❌ Interpret selhal s návratovým kódem: $IC25INT_EXIT_CODE"
    echo "Podrobnosti ve $IC25INT_OUTPUT_FILE"
    exit 2
fi
echo "✅ Interpretace dokončena. Výstup uložen do $IC25INT_OUTPUT_FILE"

echo ""
echo "🎉 Skript úspěšně dokončen!"
