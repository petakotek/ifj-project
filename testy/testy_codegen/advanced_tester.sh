#!/bin/bash

# Použití: ./advanced_tester.sh [test_directory]


# Původně: COMPILER="${COMPILER:-./<br>/../../ifj25}" - TOTO JE CHYBNÝ FORMÁT.
COMPILER="../.././ifj25"

# 2. Interpreter: Nastav přesnou cestu k interpreteru.
# Původně: INTERPRETER="${INTERPRETER:-$HOME/Stažené/sys_package/ic25int-linux-x86_64}"
INTERPRETER="$HOME/Stažené/sys_package/ic25int-linux-x86_64"
# Zkus si nastavit:
# INTERPRETER="${INTERPRETER:-$HOME/Stažené/sys_package/ic25int-linux-x86_64}"

TEST_DIR="${1:-.}"
LOG_FILE="test_log_$(date +%Y%m%d_%H%M%S).txt"
RESULTS_JSON="test_results.json"

# Globální počítadla
TOTAL=0
PASSED=0
FAILED=0
MEMORY_LEAKS=0

# Barvy
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# --- Funkce pro ověření spouštěčů ---
check_paths() {
    echo -e "${BLUE}Kontrola cest...${NC}"

    # Ověření compileru
    if [ ! -f "$COMPILER" ] || [ ! -x "$COMPILER" ]; then
        echo -e "${RED}❌ CHYBA: Compiler nenalezen nebo není spustitelný na: ${YELLOW}$COMPILER${NC}"
        echo "Prosím, zkontroluj, zda je cesta v proměnné COMPILER správně nastavena."
        exit 1
    fi
    echo -e "${GREEN}✓ Compiler nalezen: $COMPILER${NC}"

    # Ověření interpreteru
    if [ ! -f "$INTERPRETER" ] || [ ! -x "$INTERPRETER" ]; then
        echo -e "${RED}❌ CHYBA: Interpreter nenalezen nebo není spustitelný na: ${YELLOW}$INTERPRETER${NC}"
        echo "Prosím, zkontroluj, zda je cesta v proměnné INTERPRETER správně nastavena."
        exit 1
    fi
    echo -e "${GREEN}✓ Interpreter nalezen: $INTERPRETER${NC}"
    echo ""
}
# ------------------------------------

# Funkce: Normalizace výstupu (odstraní mezery mezi čísly a normalizuje hex)
normalize() {
    echo "$1" | \
        sed 's/[[:space:]]*//g' | \
        sed 's/0x\([0-9a-fA-F]*\)\.\([0-9a-fA-F]*\)p\([+-]*[0-9]*\)/HEX_FLOAT[\1.\2p\3]/g'
}

# Funkce: Porovnání dvou výstupů (ignoruje mezery, normalizuje hex)
compare_outputs() {
    local actual="$1"
    local expected="$2"

    local actual_norm=$(normalize "$actual")
    local expected_norm=$(normalize "$expected")

    if [ "$actual_norm" = "$expected_norm" ]; then
        return 0 # Shodují se
    else
        return 1 # Neshodují se
    fi
}

# Funkce: Kontrola memory leaků v valgrind logu
check_memory_leaks() {
    local valgrind_log="$1"
    # Hledáme definitely lost, indirectly lost, nebo suppressed/reachable
    if grep -q "definitely lost: [1-9]\|indirectly lost: [1-9]" "$valgrind_log" 2>/dev/null; then
        return 0 # Memory leak nalezen
    fi
    return 1 # OK
}

# Funkce: Spuštění jednoho testu
run_single_test() {
    local test_input="$1"
    local test_name="$2"
    local expected_output="$3"
    local ifjcode_out="$4"
    local actual_out="$5"
    local valgrind_log="$6"

    echo -e "\n${BLUE}[TEST]${NC} $test_name" | tee -a "$LOG_FILE"
    echo "========================================" >> "$LOG_FILE"

    # 1. Kompilace s valgrind
    echo "  Kompilace..." >&2
    valgrind --leak-check=full --log-file="$valgrind_log" \
        --quiet "$COMPILER" < "$test_input" > "$ifjcode_out" 2>>"$LOG_FILE"

    COMPILE_RET=$?
    if [ $COMPILE_RET -ne 0 ]; then
        echo -e "${RED}✗ FAIL${NC}: Kompilace selhala (exit: $COMPILE_RET)"
        echo "FAIL: Compilation failed (Exit Code: $COMPILE_RET)" >> "$LOG_FILE"
        # Přidej log z valgrindu, pokud existuje
        if [ -s "$valgrind_log" ]; then
            echo "--- VALGRIND LOG ---" >> "$LOG_FILE"
            cat "$valgrind_log" >> "$LOG_FILE"
            echo "--------------------" >> "$LOG_FILE"
        fi
        return 1
    fi

    # 2. Spuštění interpretu
    echo "  Interpretace..." >&2
    # Přidána explicitní cesta k interpreteru pro timeout
    timeout 5 "$INTERPRETER" "$ifjcode_out" > "$actual_out" 2>>"$LOG_FILE"

    INTERP_RET=$?
    if [ $INTERP_RET -eq 124 ]; then
        echo -e "${RED}✗ FAIL${NC}: Timeout interpretu"
        echo "FAIL: Interpreter timeout" >> "$LOG_FILE"
        return 1
    elif [ $INTERP_RET -ne 0 ]; then
        echo -e "${RED}✗ FAIL${NC}: Interpret skončil s chybou (exit: $INTERP_RET)"
        echo "FAIL: Interpreter error (Exit Code: $INTERP_RET)" >> "$LOG_FILE"
        return 1
    fi

    # 3. Porovnání výstupů
    echo "  Porovnání..." >&2
    ACTUAL=$(cat "$actual_out")

    if compare_outputs "$ACTUAL" "$expected_output"; then
        echo -e "${GREEN}✓ PASS${NC}"
        echo "PASS" >> "$LOG_FILE"

        # Kontrola memory leaků
        if check_memory_leaks "$valgrind_log"; then
            echo -e "  ${YELLOW}⚠ MEMORY LEAK DETEKOVÁN${NC}"
            MEMORY_LEAKS=$((MEMORY_LEAKS + 1))
            echo "WARNING: Memory leak detected" >> "$LOG_FILE"
        fi

        return 0
    else
        echo -e "${RED}✗ FAIL${NC}: Výstup se neshoduje"
        echo "FAIL: Output mismatch" >> "$LOG_FILE"
        echo "Expected (Normalized): $(normalize "$expected_output")" >> "$LOG_FILE"
        echo "Actual (Normalized):   $(normalize "$ACTUAL")" >> "$LOG_FILE"
        return 1
    fi
}

# Funkce: Vytvoření testovacího adresáře
setup_test_env() {
    mkdir -p "$TEST_DIR/.tests_work"
}

# Funkce: Spuštění všech testů z adresáře
run_all_tests() {
    setup_test_env

    echo "╔══════════════════════════════════════╗"
    echo "║    IFJ25 Advanced Test Runner        ║"
    echo "╚══════════════════════════════════════╝"
    echo ""
    echo "Log: $LOG_FILE"
    echo "JSON: $RESULTS_JSON"
    echo "Testovací adresář: $TEST_DIR"
    echo ""

    # Inicializuj JSON
    echo "[]" > "$RESULTS_JSON"

    # Hledej všechny .in soubory s odpovídajícími .expected
    JSON_ARRAY="["
    FIRST_TEST=true

    find "$TEST_DIR" -maxdepth 1 -name "*.in" | sort | while read input_file; do

        base_name=$(basename "$input_file" .in)
        expected_file="$TEST_DIR/${base_name}.expected"

        if [ ! -f "$expected_file" ]; then
            echo -e "${YELLOW}⚠ SKIP${NC}: $base_name (chybí .expected)"
            continue
        fi

        TOTAL=$((TOTAL + 1))

        # Příprava dočasných souborů
        ifjcode_out="$TEST_DIR/.tests_work/${base_name}.ifjcode"
        actual_out="$TEST_DIR/.tests_work/${base_name}.out"
        valgrind_log="$TEST_DIR/.tests_work/${base_name}.valgrind"

        # Přečti očekávaný výstup
        expected_output=$(cat "$expected_file")

        # Spusť test
        if run_single_test "$input_file" "$base_name" "$expected_output" \
            "$ifjcode_out" "$actual_out" "$valgrind_log"; then
            RESULT="PASS"
            PASSED=$((PASSED + 1))
        else
            RESULT="FAIL"
            FAILED=$((FAILED + 1))
        fi

        # Kontrola memory leaků pro JSON
        LEAK_CHECK=$(check_memory_leaks "$valgrind_log" && echo "true" || echo "false")

        # Sestavení JSON pro test
        TEST_JSON="{
            \"test_name\": \"$base_name\",
            \"status\": \"$RESULT\",
            \"input_file\": \"$input_file\",
            \"expected_file\": \"$expected_file\",
            \"ifjcode_out\": \"$ifjcode_out\",
            \"actual_out\": \"$actual_out\",
            \"valgrind_log\": \"$valgrind_log\",
            \"has_memory_leak\": $LEAK_CHECK
        }"

        # Přidej test do pole JSON
        if [ "$FIRST_TEST" = true ]; then
            JSON_ARRAY="$JSON_ARRAY$TEST_JSON"
            FIRST_TEST=false
        else
            JSON_ARRAY="$JSON_ARRAY,$TEST_JSON"
        fi
    done

    JSON_ARRAY="$JSON_ARRAY]"

    # Uložení JSON s výsledky
    cat > "$RESULTS_JSON" <<EOF
{
  "timestamp": "$(date -Iseconds)",
  "test_directory": "$TEST_DIR",
  "total": $TOTAL,
  "passed": $PASSED,
  "failed": $FAILED,
  "memory_leaks": $MEMORY_LEAKS,
  "success_rate": $(echo "scale=2; if ($TOTAL > 0) $PASSED * 100 / $TOTAL else 0" | bc 2>/dev/null || echo "N/A"),
  "tests": $JSON_ARRAY
}
EOF
}

# Hlavní program
main() {
    check_paths # Kontrola cest hned na začátku

    > "$LOG_FILE" # Vymaž log

    run_all_tests

    # Výstup statistiky
    echo ""
    echo "╔══════════════════════════════════════╗"
    echo "║             TEST SUMMARY             ║"
    echo "╚══════════════════════════════════════╝"
    echo ""
    echo "Celkem testů:         $TOTAL"
    echo -e "Úspěšných:            ${GREEN}$PASSED${NC}"
    echo -e "Selhavších:           ${RED}$FAILED${NC}"
    echo -e "Memory leak warning:  ${YELLOW}$MEMORY_LEAKS${NC}"
    echo ""

    echo "Výsledky uloženy v: $RESULTS_JSON"
    echo "Detailní log v: $LOG_FILE"

    # Vrátí exit code
    [ $FAILED -eq 0 ] && exit 0 || exit 1
}

main "$@"
