# README - IFJ25 Testovací Systém

## Přehled

Vytvořil jsem pro tebe kompletní testovací systém pro IFJ25 compiler s těmito skrippy:

### 1. **run_tests.sh** - Základní testovací skript (20 testů)
- Automaticky vytváří testovací soubory
- Spouští compiler přes `valgrind --leak-check=full`
- Ověřuje výstup
- Kontroluje memory leaky
- Vytváří detailní log

**Použití:**
```bash
chmod +x run_tests.sh
./run_tests.sh
```

**Výstup:**
- Barevné výsledky v terminálu
- Detailní log v souboru `test_results.txt`
- Vygenerované testy v adresáři `./tests`

---

### 2. **test_runner.py** - Pokročilý Python runner
- Lepší normalizace hexadecimálních čísel
- Porovnání floatů s lepší přesností
- JSON export výsledků
- 10 referenčních testů

**Použití:**
```bash
chmod +x test_runner.py
python3 test_runner.py
```

**Výstup:**
- Barevné výsledky v terminálu
- JSON soubor: `test_results_python.json`
- Testy v adresáři `./tests_python`

---

### 3. **advanced_tester.sh** - Pro vaše vlastní testy
- Skenuje adresář pro `.in` a `.expected` soubory
- Normalizuje output (odstraňuje mezery bez vlivu na logiku)
- Kontroluje memory leaky
- Vytváří JSON report

**Jak jej použít:**

1. Vytvořte soubory pro testy:
```bash
mkdir my_tests
```

2. Přidejte testovací soubory:
```bash
# my_tests/test_custom_01.in
import "ifj25" for Ifj
class Program {
    static main() {
        Ifj.write(123)
    }
}

# my_tests/test_custom_01.expected
123
```

3. Spusťte tester:
```bash
chmod +x advanced_tester.sh
./advanced_tester.sh my_tests
```

---

## Důležité parametry

### Přizpůsobení cesty k compileru/interpretu

```bash
# Bash skrippty
export COMPILER="./ifj25"
export INTERPRETER="/path/to/interpreter"
./run_tests.sh

# Python
python3 test_runner.py  # Automaticky najde cesty
```

---

## Specifika testovacího systému

### Normalizace výstupu

**Systém automaticky:**
- Odstraňuje všechny bílé znaky (spaces, newlines, tabs)
- Normalizuje hexadecimální floaty (0x1.8p+3 = 12.0)
- Porovnává hodnoty, ne řetězce

**Příklad:**
```
Očekávaný: "42\n"
Výstup:    "  42  "
Výsledek:  ✓ PASS
```

### Memory Leak Checking

Každý test automaticky kontroluje:
- `valgrind --leak-check=full`
- Zaznamenává "definitely lost", "indirectly lost"
- Varuje na výstup s ⚠

---

## Příklady testů

### Test 1: Jednoduché sčítání
```ifj25
import "ifj25" for Ifj
class Program {
    static main() {
        var a
        a = 5
        var b
        b = 3
        Ifj.write(a + b)
    }
}
```
**Očekávaný výstup:** `8`

### Test 2: Float operace (HEX výstup)
```ifj25
import "ifj25" for Ifj
class Program {
    static main() {
        var x
        x = 5.5
        var y
        y = 2.5
        Ifj.write(x + y)
    }
}
```
**Očekávaný výstup:** `0x1p+3` (= 8.0 v hexu)

### Test 3: String operace
```ifj25
import "ifj25" for Ifj
class Program {
    static main() {
        var s
        s = "Hello"
        Ifj.write(s + " " + "World")
    }
}
```
**Očekávaný výstup:** `HelloWorld`

### Test 4: While cyklus
```ifj25
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
```
**Očekávaný výstup:** `3` `2` `1` (bez mezer)

### Test 5: Funkce s parametry
```ifj25
import "ifj25" for Ifj
class Program {
    static add(x, y) {
        return x + y
    }
    
    static main() {
        Ifj.write(add(10, 20))
    }
}
```
**Očekávaný výstup:** `30`

---

## Struktura vytváření vlastních testů

Nejjednoduší způsob je přidat testy přímo do skriptu:

```bash
# V run_tests.sh přidat nový test:

cat > "${TEST_DIR}/test_21.in" << 'EOF'
import "ifj25" for Ifj
class Program {
    static main() {
        var x
        x = 42
        Ifj.write(x)
    }
}
EOF

cat > "${TEST_DIR}/test_21.expected" << 'EOF'
42
EOF

run_test "test_21_my_test" "${TEST_DIR}/test_21.in" "${TEST_DIR}/test_21.expected"
```

---

## Logování a Debug

### Kde najít výsledky?

1. **Terminál** - barevné výstupy (zelená = PASS, červená = FAIL)
2. **test_results.txt** - detailní textový log
3. **tests/.** - všechny generované soubory:
   - `test_XX.in` - vstupní kod
   - `test_XX.expected` - očekávaný výstup
   - `test_XX.ifjcode` - vygenerovaný IFJcode25
   - `test_XX.out` - skutečný výstup
   - `test_XX_valgrind.log` - memory report

### Memory Leak Report

```bash
# Otevřít valgrind log
cat tests/test_05_while_loop_valgrind.log | grep -A 10 "LEAK SUMMARY"
```

---

## Pokročilé použití

### Spuštění specifického testu

```bash
# Ručně otestovat jeden soubor
valgrind --leak-check=full ./ifj25 < tests/test_05.in > output.ifjcode
/home/tjoslef/Stažené/sys_package/.ic25int-linux-x86_64 output.ifjcode
```

### Automatické generování testů z příkladu

Pokud máš soubor `ex0-vsechny-konstrukce-Kopie.txt`, můžeš extrahovat jednotlivé ukázky a vytvořit testy:

```bash
# Rozdělit soubor na jednotlivé funkce
csplit ex0-vsechny-konstrukce-Kopie.txt '/^    static/' '{*}'

# Každý xx00 soubor je nová funkce - umístit do my_tests/ jako .in
```

---

## Troubleshooting

### Chyba: "Permission denied"
```bash
chmod +x run_tests.sh advanced_tester.sh test_runner.py
```

### Chyba: "Compiler not found"
```bash
export COMPILER="/path/to/ifj25"
./run_tests.sh
```

### Chyba: "Interpreter timeout"
- Zkontroluj nekonečné smyčky v kódu
- Zvýší timeout v scriptu (výchozí 5 sekund)

### Memory leak v testu

Pokud vidíš varování ⚠ MEMORY LEAK:
```bash
# Detailnější analýza
cat tests/test_XX_valgrind.log | grep -E "definitely|indirectly|possibly"
```

---

## Souhrn příkazů

```bash
# Základní testování
chmod +x run_tests.sh
./run_tests.sh

# Pokročilé testování
chmod +x advanced_tester.sh
./advanced_tester.sh ./my_tests

# Python version
python3 test_runner.py

# Vlastní test
mkdir custom_tests
echo 'import "ifj25" for Ifj
class Program {
    static main() {
        Ifj.write(42)
    }
}' > custom_tests/my_test.in

echo '42' > custom_tests/my_test.expected
./advanced_tester.sh custom_tests
```

---

**Vytvořeno:** 2025-11-17
**Systém:** IFJ25 Compiler Testing Suite
