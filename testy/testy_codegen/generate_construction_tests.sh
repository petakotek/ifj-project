#!/bin/bash

# Generátor testů z ex0 souboru s formátem testů
# Vytváří strukturovanou sadu testů pro všechny konstrukce jazyka

# Parametry
OUTPUT_DIR="${1:-.}/construction_tests"
mkdir -p "$OUTPUT_DIR"

echo "═════════════════════════════════════════"
echo "  IFJ25 Construction Tests Generator"
echo "═════════════════════════════════════════"
echo ""
echo "Cílový adresář: $OUTPUT_DIR"
echo ""

# Test 1: Návratová hodnota z funkce
cat > "$OUTPUT_DIR/test_01_return_value.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static getAnswer() {
        return 42
    }

    static main() {
        Ifj.write(getAnswer())
    }
}
EOF

cat > "$OUTPUT_DIR/test_01_return_value.expected" << 'EOF'
42
EOF

# Test 2: Funkce s parametrem
cat > "$OUTPUT_DIR/test_02_function_param.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static getAnswer(arg) {
        if (arg is Num) {
            return arg * 2
        } else {
            return null
        }
    }

    static main() {
        Ifj.write(getAnswer(21))
    }
}
EOF

cat > "$OUTPUT_DIR/test_02_function_param.expected" << 'EOF'
42
EOF

# Test 3: Type checking s "is"
cat > "$OUTPUT_DIR/test_03_type_checking.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static processValue(arg) {
        if (arg is Num) {
            Ifj.write("number")
            return arg
        } else {
            Ifj.write("not_number")
            return null
        }
    }

    static main() {
        var result
        result = processValue(100)
        Ifj.write(result)
    }
}
EOF

cat > "$OUTPUT_DIR/test_03_type_checking.expected" << 'EOF'
number
100
EOF

# Test 4: Aritmetické operace
cat > "$OUTPUT_DIR/test_04_arithmetic.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var a
        a = 32
        var b
        b = 84
        if (a - 1 > b * 1) {
            Ifj.write("greater")
        } else {
            Ifj.write("lesser")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_04_arithmetic.expected" << 'EOF'
lesser
EOF

# Test 5: Přiřazení proměnné
cat > "$OUTPUT_DIR/test_05_assignment.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 10
        var y
        y = x * 42
        Ifj.write(y)
    }
}
EOF

cat > "$OUTPUT_DIR/test_05_assignment.expected" << 'EOF'
420
EOF

# Test 6: Built-in funkce str()
cat > "$OUTPUT_DIR/test_06_str_builtin.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var num
        num = 100
        var str
        str = Ifj.str(num)
        Ifj.write(str)
    }
}
EOF

cat > "$OUTPUT_DIR/test_06_str_builtin.expected" << 'EOF'
100
EOF

# Test 7: String concatenace
cat > "$OUTPUT_DIR/test_07_string_concat.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var ansStr
        ansStr = Ifj.str(99)
        var result
        result = "Odpoved je " + ansStr
        Ifj.write(result)
    }
}
EOF

cat > "$OUTPUT_DIR/test_07_string_concat.expected" << 'EOF'
Odpoved je 99
EOF

# Test 8: Getter vlastnost
cat > "$OUTPUT_DIR/test_08_getter.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static unicorn {
        if (__a) {
            return __a + 10
        } else {
            return null
        }
    }

    static main() {
        Ifj.write(unicorn)
    }
}
EOF

cat > "$OUTPUT_DIR/test_08_getter.expected" << 'EOF'
null
EOF

# Test 9: Setter vlastnost
cat > "$OUTPUT_DIR/test_09_setter.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static unicorn=(val) {
        __a = val
        Ifj.write("set")
    }

    static main() {
        unicorn = 5
    }
}
EOF

cat > "$OUTPUT_DIR/test_09_setter.expected" << 'EOF'
set
EOF

# Test 10: Podmínka se else
cat > "$OUTPUT_DIR/test_10_if_else.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 10
        if (x > 20) {
            Ifj.write("greater")
        } else {
            Ifj.write("not_greater")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_10_if_else.expected" << 'EOF'
not_greater
EOF

# Test 11: While cyklus
cat > "$OUTPUT_DIR/test_11_while.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var i
        i = 3
        while (i > 0) {
            Ifj.write(i)
            i = i - 1
        }
        Ifj.write("done")
    }
}
EOF

cat > "$OUTPUT_DIR/test_11_while.expected" << 'EOF'
3
2
1
done
EOF

# Test 12: Globální proměnná
cat > "$OUTPUT_DIR/test_12_global_var.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        __a = 42
        Ifj.write(__a)
    }
}
EOF

cat > "$OUTPUT_DIR/test_12_global_var.expected" << 'EOF'
42
EOF

# Test 13: Null kontrola
cat > "$OUTPUT_DIR/test_13_null_check.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        if (x == null) {
            Ifj.write("is_null")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_13_null_check.expected" << 'EOF'
is_null
EOF

# Test 14: Type check Null
cat > "$OUTPUT_DIR/test_14_null_type_check.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        if (x is Null) {
            Ifj.write("null_type")
        } else {
            Ifj.write("not_null_type")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_14_null_type_check.expected" << 'EOF'
null_type
EOF

# Test 15: Block scope
cat > "$OUTPUT_DIR/test_15_block_scope.in" << 'EOF'
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

cat > "$OUTPUT_DIR/test_15_block_scope.expected" << 'EOF'
2
1
EOF

# Test 16: Vnořené if
cat > "$OUTPUT_DIR/test_16_nested_if.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var x
        x = 15
        if (x > 10) {
            if (x < 20) {
                Ifj.write("in_range")
            }
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_16_nested_if.expected" << 'EOF'
in_range
EOF

# Test 17: String repetice
cat > "$OUTPUT_DIR/test_17_string_repeat.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var s
        s = "a"
        Ifj.write(s * 3)
    }
}
EOF

cat > "$OUTPUT_DIR/test_17_string_repeat.expected" << 'EOF'
aaa
EOF

# Test 18: Division
cat > "$OUTPUT_DIR/test_18_division.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var a
        a = 84
        var b
        b = 2
        var c
        c = 4
        Ifj.write(a / b / c)
    }
}
EOF

cat > "$OUTPUT_DIR/test_18_division.expected" << 'EOF'
10
EOF

# Test 19: Logické operátory - AND
cat > "$OUTPUT_DIR/test_19_logical_and.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        if (true && true) {
            Ifj.write("true_and")
        }
        if (true && false) {
            Ifj.write("should_not_print")
        } else {
            Ifj.write("false_and")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_19_logical_and.expected" << 'EOF'
true_and
false_and
EOF

# Test 20: Logické operátory - OR
cat > "$OUTPUT_DIR/test_20_logical_or.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        if (false || true) {
            Ifj.write("true_or")
        }
        if (false || false) {
            Ifj.write("should_not_print")
        } else {
            Ifj.write("false_or")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_20_logical_or.expected" << 'EOF'
true_or
false_or
EOF

# Test 21: Unární NOT
cat > "$OUTPUT_DIR/test_21_logical_not.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        if (!false) {
            Ifj.write("not_false")
        }
        if (!true) {
            Ifj.write("should_not_print")
        } else {
            Ifj.write("not_true")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_21_logical_not.expected" << 'EOF'
not_false
not_true
EOF

# Test 22: Porovnání menší
cat > "$OUTPUT_DIR/test_22_less_than.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var a
        a = 5
        var b
        b = 10
        if (a < b) {
            Ifj.write("less")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_22_less_than.expected" << 'EOF'
less
EOF

# Test 23: Porovnání větší
cat > "$OUTPUT_DIR/test_23_greater_than.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static main() {
        var a
        a = 10
        var b
        b = 5
        if (a > b) {
            Ifj.write("greater")
        }
    }
}
EOF

cat > "$OUTPUT_DIR/test_23_greater_than.expected" << 'EOF'
greater
EOF

# Test 24: Přetížená funkce (bez parametrů)
cat > "$OUTPUT_DIR/test_24_overload_no_params.in" << 'EOF'
import "ifj25" for Ifj

class Program {
    static getValue() {
        return 42
    }

    static main() {
        var result
        result = getValue()
        Ifj.write(result)
    }
}
EOF

cat > "$OUTPUT_DIR/test_24_overload_no_params.expected" << 'EOF'
42
EOF

# Test 25: Přetížená funkce (s parametrem)
cat > "$OUTPUT_DIR/test_25_overload_with_param.in" << 'EOF'
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

cat > "$OUTPUT_DIR/test_25_overload_with_param.expected" << 'EOF'
10
10
EOF

echo ""
echo "✓ Vygenerováno $OUTPUT_DIR/test_*.{in,expected}"
echo ""
echo "Chcete-li spustit testy:"
echo "  ./advanced_tester.sh $OUTPUT_DIR"
echo ""
