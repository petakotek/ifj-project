#!/bin/bash

# Testovací skript pro IFJ25 compiler
# Spuštění: chmod +x run_tests.sh && ./run_tests.sh

COMPILER="../.././ifj25"
INTERPRETER="$HOME/Stažené/sys_package/ic25int-linux-x86_64"
TEST_DIR="./tests"
RESULTS_FILE="test_results.txt"

# Barvy pro output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Počítadla
PASSED=0
FAILED=0
TOTAL=0

# Vytvoření adresáře pro testy
mkdir -p "$TEST_DIR"

# Funkce pro normalizaci výstupu (odstraní bílé znaky a normalizuje hex)
normalize_output() {
    local output="$1"
    # Odstraní všechny bílé znaky (spaces, newlines, tabs) kromě důležitých
    echo "$output" | tr -d '\n' | sed 's/ //g'
}

# Funkce pro spuštění jednoho testu
run_test() {
    local test_name="$1"
    local input_file="$2"
    local expected_file="$3"

    TOTAL=$((TOTAL + 1))

    echo "========================================" >> "$RESULTS_FILE"
    echo "Test: $test_name" >> "$RESULTS_FILE"
    echo "========================================" >> "$RESULTS_FILE"

    # Spuštění kompileru přes valgrind
    echo "Spouštím: valgrind --leak-check=full $COMPILER < $input_file" >> "$RESULTS_FILE"
    valgrind --leak-check=full --log-file="${TEST_DIR}/${test_name}_valgrind.log" \
        --leak-check=full "$COMPILER" < "$input_file" > "${TEST_DIR}/${test_name}.ifjcode" 2>> "$RESULTS_FILE"

    COMPILE_EXIT=$?

    if [ $COMPILE_EXIT -ne 0 ]; then
        echo -e "${RED}✗ FAIL${NC}: $test_name (kompilace selhala, exit code: $COMPILE_EXIT)"
        echo "RESULT: FAIL (kompilace)" >> "$RESULTS_FILE"
        FAILED=$((FAILED + 1))
        return 1
    fi

    # Spuštění generovaného kódu přes interpret
    echo "Spouštím interpret: $INTERPRETER $input_file" >> "$RESULTS_FILE"
    "$INTERPRETER" "${TEST_DIR}/${test_name}.ifjcode" > "${TEST_DIR}/${test_name}.out" 2>> "$RESULTS_FILE"
    INTERPRET_EXIT=$?

    # Čtení očekávaného výstupu
    if [ ! -f "$expected_file" ]; then
        echo -e "${RED}✗ FAIL${NC}: $test_name (chybí soubor s očekávaným výstupem)"
        echo "RESULT: FAIL (chybí očekávaný výstup)" >> "$RESULTS_FILE"
        FAILED=$((FAILED + 1))
        return 1
    fi

    # Normalizace výstupů
    ACTUAL=$(normalize_output "$(cat "${TEST_DIR}/${test_name}.out")")
    EXPECTED=$(normalize_output "$(cat "$expected_file")")

    echo "Očekávaný výstup: $EXPECTED" >> "$RESULTS_FILE"
    echo "Skutečný výstup: $ACTUAL" >> "$RESULTS_FILE"

    # Porovnání
    if [ "$ACTUAL" = "$EXPECTED" ]; then
        echo -e "${GREEN}✓ PASS${NC}: $test_name"
        echo "RESULT: PASS" >> "$RESULTS_FILE"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗ FAIL${NC}: $test_name"
        echo "RESULT: FAIL (výstup se neshoduje)" >> "$RESULTS_FILE"
        echo "Diff:" >> "$RESULTS_FILE"
        diff -u <(echo "$EXPECTED") <(echo "$ACTUAL") >> "$RESULTS_FILE" || true
        FAILED=$((FAILED + 1))
    fi

    # Kontrola valgrind výstupu
    if grep -q "definitely lost\|indirectly lost\|possibly lost" "${TEST_DIR}/${test_name}_valgrind.log" 2>/dev/null; then
        if ! grep -q "0 bytes in 0 blocks" "${TEST_DIR}/${test_name}_valgrind.log" 2>/dev/null; then
            echo "⚠ MEMORY LEAK v $test_name" >> "$RESULTS_FILE"
        fi
    fi

    echo "" >> "$RESULTS_FILE"
}

# Inicializace log souboru
> "$RESULTS_FILE"

echo "╔════════════════════════════════════════╗"
echo "║    IFJ25 Compiler Testovací Suite      ║"
echo "╚════════════════════════════════════════╝"
echo ""

# ===== TEST 1: Základní funkce =====
cat > "${TEST_DIR}/test_01.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write(42)
    }
}
EOF

cat > "${TEST_DIR}/test_01.expected" << 'EOF'
42
EOF

run_test "test_01_basic_output" "${TEST_DIR}/test_01.in" "${TEST_DIR}/test_01.expected"

# ===== TEST 2: Operace s inty =====
cat > "${TEST_DIR}/test_02.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var a
        a = 10
        var b
        b = 5
        Ifj.write(a + b)
        Ifj.write(a - b)
        Ifj.write(a * b)
        Ifj.write(a / b)
    }
}
EOF

cat > "${TEST_DIR}/test_02.expected" << 'EOF'
15
5
50
2
EOF

run_test "test_02_int_operations" "${TEST_DIR}/test_02.in" "${TEST_DIR}/test_02.expected"

# ===== TEST 3: String operace =====
cat > "${TEST_DIR}/test_03.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var s1
        s1 = "Hello"
        var s2
        s2 = " World"
        Ifj.write(s1 + s2)
    }
}
EOF

cat > "${TEST_DIR}/test_03.expected" << 'EOF'
HelloWorld
EOF

run_test "test_03_string_concat" "${TEST_DIR}/test_03.in" "${TEST_DIR}/test_03.expected"

# ===== TEST 4: If podmínka =====
cat > "${TEST_DIR}/test_04.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 10
        if (x > 5) {
            Ifj.write("greater")
        } else {
            Ifj.write("lesser")
        }
    }
}
EOF

cat > "${TEST_DIR}/test_04.expected" << 'EOF'
greater
EOF

run_test "test_04_if_condition" "${TEST_DIR}/test_04.in" "${TEST_DIR}/test_04.expected"

# ===== TEST 5: While cyklus =====
cat > "${TEST_DIR}/test_05.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var i
        i = 3
        while (i > 0) {
            Ifj.write(i)
            i = i - 1
        }
    }
}
EOF

cat > "${TEST_DIR}/test_05.expected" << 'EOF'
3
2
1
EOF

run_test "test_05_while_loop" "${TEST_DIR}/test_05.in" "${TEST_DIR}/test_05.expected"

# ===== TEST 6: Funkce bez parametrů =====
cat > "${TEST_DIR}/test_06.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static getValue() {
        return 99
    }

    static main() {
        var result
        result = getValue()
        Ifj.write(result)
    }
}
EOF

cat > "${TEST_DIR}/test_06.expected" << 'EOF'
99
EOF

run_test "test_06_function_no_params" "${TEST_DIR}/test_06.in" "${TEST_DIR}/test_06.expected"

# ===== TEST 7: Funkce s parametry =====
cat > "${TEST_DIR}/test_07.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static add(a, b) {
        return a + b
    }

    static main() {
        var sum
        sum = add(3, 7)
        Ifj.write(sum)
    }
}
EOF

cat > "${TEST_DIR}/test_07.expected" << 'EOF'
10
EOF

run_test "test_07_function_with_params" "${TEST_DIR}/test_07.in" "${TEST_DIR}/test_07.expected"

# ===== TEST 8: Porovnávání =====
cat > "${TEST_DIR}/test_08.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 5
        if (x == 5) {
            Ifj.write("equal")
        }
        if (x < 10) {
            Ifj.write("less")
        }
        if (x > 3) {
            Ifj.write("greater")
        }
    }
}
EOF

cat > "${TEST_DIR}/test_08.expected" << 'EOF'
equal
less
greater
EOF

run_test "test_08_comparisons" "${TEST_DIR}/test_08.in" "${TEST_DIR}/test_08.expected"

# ===== TEST 9: Globální proměnné =====
cat > "${TEST_DIR}/test_09.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        __global = 42
        Ifj.write(__global)
    }
}
EOF

cat > "${TEST_DIR}/test_09.expected" << 'EOF'
42
EOF

run_test "test_09_global_vars" "${TEST_DIR}/test_09.in" "${TEST_DIR}/test_09.expected"

# ===== TEST 10: Float operace =====
cat > "${TEST_DIR}/test_10.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 10.5
        var y
        y = 2.5
        Ifj.write(x + y)
    }
}
EOF

cat > "${TEST_DIR}/test_10.expected" << 'EOF'
0x1.8p+3
EOF

run_test "test_10_float_operations" "${TEST_DIR}/test_10.in" "${TEST_DIR}/test_10.expected"

# ===== TEST 11: Nested bloky a scope =====
cat > "${TEST_DIR}/test_11.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 1
        {
            var x
            x = 2
            Ifj.write(x)
        }
        Ifj.write(x)
    }
}
EOF

cat > "${TEST_DIR}/test_11.expected" << 'EOF'
2
1
EOF

run_test "test_11_scope_nesting" "${TEST_DIR}/test_11.in" "${TEST_DIR}/test_11.expected"

# ===== TEST 12: Type checking (is operator) =====
cat > "${TEST_DIR}/test_12.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 42
        if (x is Num) {
            Ifj.write("number")
        }
        var s
        s = "text"
        if (s is String) {
            Ifj.write("string")
        }
    }
}
EOF

cat > "${TEST_DIR}/test_12.expected" << 'EOF'
number
string
EOF

run_test "test_12_type_checking" "${TEST_DIR}/test_12.in" "${TEST_DIR}/test_12.expected"

# ===== TEST 13: Logické operátory =====
cat > "${TEST_DIR}/test_13.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        if (true && true) {
            Ifj.write("and_true")
        }
        if (true || false) {
            Ifj.write("or_true")
        }
        if (!false) {
            Ifj.write("not_true")
        }
    }
}
EOF

cat > "${TEST_DIR}/test_13.expected" << 'EOF'
and_true
or_true
not_true
EOF

run_test "test_13_logical_operators" "${TEST_DIR}/test_13.in" "${TEST_DIR}/test_13.expected"

# ===== TEST 14: Přetížené funkce =====
cat > "${TEST_DIR}/test_14.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static getValue() {
        return 10
    }

    static getValue(x) {
        return x * 2
    }

    static main() {
        Ifj.write(getValue())
        Ifj.write(getValue(5))
    }
}
EOF

cat > "${TEST_DIR}/test_14.expected" << 'EOF'
10
10
EOF

run_test "test_14_overloaded_functions" "${TEST_DIR}/test_14.in" "${TEST_DIR}/test_14.expected"

# ===== TEST 15: Built-in funkcí Ifj.str() =====
cat > "${TEST_DIR}/test_15.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var num
        num = 42
        var str
        str = Ifj.str(num)
        Ifj.write(str)
    }
}
EOF

cat > "${TEST_DIR}/test_15.expected" << 'EOF'
42
EOF

run_test "test_15_str_builtin" "${TEST_DIR}/test_15.in" "${TEST_DIR}/test_15.expected"

# ===== TEST 16: String repetice (string * int) =====
cat > "${TEST_DIR}/test_16.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var s
        s = "a"
        Ifj.write(s * 3)
    }
}
EOF

cat > "${TEST_DIR}/test_16.expected" << 'EOF'
aaa
EOF

run_test "test_16_string_repetition" "${TEST_DIR}/test_16.in" "${TEST_DIR}/test_16.expected"

# ===== TEST 17: Vrácení null =====
cat > "${TEST_DIR}/test_17.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static getNull() {
        return null
    }

    static main() {
        var x
        x = getNull()
        if (x == null) {
            Ifj.write("null_value")
        }
    }
}
EOF

cat > "${TEST_DIR}/test_17.expected" << 'EOF'
null_value
EOF

run_test "test_17_null_handling" "${TEST_DIR}/test_17.in" "${TEST_DIR}/test_17.expected"

# ===== TEST 18: Vnořené if/else =====
cat > "${TEST_DIR}/test_18.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 15
        if (x > 10) {
            if (x < 20) {
                Ifj.write("between")
            }
        }
    }
}
EOF

cat > "${TEST_DIR}/test_18.expected" << 'EOF'
between
EOF

run_test "test_18_nested_conditions" "${TEST_DIR}/test_18.in" "${TEST_DIR}/test_18.expected"

# ===== TEST 19: Komplexní aritmetika =====
cat > "${TEST_DIR}/test_19.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var result
        result = (10 + 5) * 2 - 10
        Ifj.write(result)
    }
}
EOF

cat > "${TEST_DIR}/test_19.expected" << 'EOF'
20
EOF

run_test "test_19_complex_arithmetic" "${TEST_DIR}/test_19.in" "${TEST_DIR}/test_19.expected"

# ===== TEST 20: Unární minus =====
cat > "${TEST_DIR}/test_20.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 0 - 5
        Ifj.write(x)
    }
}
EOF

cat > "${TEST_DIR}/test_20.expected" << 'EOF'
-5
EOF

run_test "test_20_unary_minus" "${TEST_DIR}/test_20.in" "${TEST_DIR}/test_20.expected"

echo ""
echo "╔════════════════════════════════════════╗"
echo "║           Testování Dokončeno          ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Výsledky:"
echo -e "  Celkem testů:  $TOTAL"
echo -e "  ${GREEN}Úspěšné:${NC}   $PASSED"
echo -e "  ${RED}Neúspěšné:${NC}  $FAILED"
echo ""
echo "Detailní výsledky v souboru: $RESULTS_FILE"
echo "Vygenerované soubory v adresáři: $TEST_DIR"
