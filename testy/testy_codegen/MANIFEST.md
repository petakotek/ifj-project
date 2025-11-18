# IFJ25 TESTOVACÍ SYSTÉM - ÚPLNÝ PŘEHLED

Vytvořil jsem pro tebe kompletní automatizovaný testovací systém s těmito komponenty:

## 📦 VYTVOŘENÉ SOUBORY

### 1. **run_tests.sh** (570 řádků)
Základní testovací skript s 20 zabudovanými testy
- Automaticky vytváří testovací soubory
- Spouští compiler přes `valgrind --leak-check=full`
- Porovnává výstupy (ignoruje mezery a newlines)
- Generuje detailní logg
- **Výstup:** `test_results.txt`, složka `tests/` s veškerými výsledky

### 2. **test_runner.py** (370 řádků)
Pokročilý Python test runner
- Lepší normalizace hexadecimálních čísel
- Porovnání floatů s přesností
- JSON export výsledků
- 10 referenčních testů zabudovaných
- **Výstup:** `test_results_python.json`, složka `tests_python/`

### 3. **advanced_tester.sh** (200 řádků)
Systém pro spouštění vlastních testů
- Skenuje adresář pro `.in` a `.expected` soubory
- Normalizuje výstup (bez vlivu mezer)
- Kontroluje memory leaky v každém testu
- JSON report se statistikou
- **Výstup:** `test_results.json`, detailní log

### 4. **generate_construction_tests.sh** (400+ řádků)
Automaticky generuje 25 testů pro všechny jazykové konstrukce
Pokrývá:
- Návratové hodnoty z funkcí
- Funkce s parametry a přetížení
- Type checking (`is` operátor)
- Aritmetické a logické operace
- If/else podmínky
- While cykly
- Globální proměnné
- Gettery a settery
- String operace
- Scoping a vnořené bloky
- A dalších 10+ konstrukcí

### 5. **README_TESTY.md** (300+ řádků)
Komplexní dokumentace obsahující:
- Přehled všech skriptů
- Podrobné návody na použití
- Příklady testů
- Způsoby vytvoření vlastních testů
- Troubleshooting guide
- Pokročilé použití

### 6. **QUICKSTART.sh** (250+ řádků)
Interaktivní quick start guide
- Vysvětluje co máš a co s tím dělat
- 3 kroky k prvnímu testu
- Příklady výstupů
- Typický workflow
- Odpovědi na časté otázky

### 7. **Makefile** (150+ řádků)
Pohodlný Makefile pro všechny operace
- `make setup` - příprava
- `make run-basic` - základní 20 testů
- `make run-python` - Python testy
- `make run-construction` - 25 stavebních testů
- `make run-all` - všechny najednou
- `make clean` - vyčistit
- `make stats` - statistika

## 🎯 JAK NA TO

### Nejrychlejší start (3 příkazy):
```bash
chmod +x *.sh *.py
make setup
make run-all
```

### Nebo manuálně:
```bash
# Základní testy
./run_tests.sh

# Stavební testy
./generate_construction_tests.sh
./advanced_tester.sh ./construction_tests

# Python verze
python3 test_runner.py
```

## 📊 CO SE TESTUJE

### Základní testy (20 testů):
✓ Jednoduchý výstup (Ifj.write)
✓ Operace s inty (+, -, *, /)
✓ String concatenace
✓ If/else podmínka
✓ While cyklus
✓ Funkce bez parametrů
✓ Funkce s parametry
✓ Porovnávací operátory
✓ Globální proměnné
✓ Float operace
✓ Vnořené bloky a scoping
✓ Type checking (is)
✓ Logické operátory (&&, ||, !)
✓ Přetížené funkce
✓ Built-in funkce (Ifj.str)
✓ String repetice
✓ Null handling
✓ Vnořené podmínky
✓ Komplexní aritmetika
✓ Unární minus

### Stavební testy (25 testů):
✓ Návratové hodnoty
✓ Parametry funkcí
✓ Type checking
✓ Aritmetické operace
✓ Přiřazení
✓ Built-in str()
✓ String concatenace
✓ Getter vlastnosti
✓ Setter vlastnosti
✓ If/else
✓ While cyklus
✓ Globální proměnné
✓ Null kontrola
✓ Null type checking
✓ Block scope
✓ Vnořené if
✓ String repetice
✓ Dělení
✓ Logické operátory AND
✓ Logické operátory OR
✓ Logické operátory NOT
✓ Porovnání menší
✓ Porovnání větší
✓ Přetížení bez parametrů
✓ Přetížení s parametrem

## 🔑 KLÍČOVÉ VLASTNOSTI

### ✅ Automatické normalizace výstupu
- Ignoruje mezery a newlines
- Normalizuje hexadecimální float (`0x1.8p+3` = 12.0)
- Porovnává hodnoty, ne řetězce

**Příklad:**
```
Očekávaný: "  42  \n"
Výstup: "42"
Výsledek: ✓ PASS
```

### ✅ Memory leak detection
Každý test automaticky spouští:
- `valgrind --leak-check=full`
- Detektuje "definitely lost", "indirectly lost"
- Varuje s ⚠ symbol v terminálu

### ✅ Detailní logging
Vytváří tři úrovně loggů:
1. **Terminál** - barevné, stručné výsledky
2. **TXT log** - detailní text se všemi info
3. **JSON** - strojově zpracovatelné výsledky

### ✅ Modulární design
Každý skript je nezávislý:
- `run_tests.sh` - práce bez závislostí
- `test_runner.py` - Python verze
- `advanced_tester.sh` - pro vlastní testy

## 📁 STRUKTURA VÝSTUPŮ

Po spuštění `./run_tests.sh`:
```
./tests/
├── test_01.in                      (vstupní IFJ25 kód)
├── test_01.expected                (očekávaný výstup)
├── test_01.ifjcode                 (vygenerovaný IFJcode25)
├── test_01.out                     (skutečný výstup)
└── test_01_valgrind.log            (memory report)

test_results.txt                     (detailní text log)
test_results.json                    (statistika v JSON)
```

## 🔧 PŘIZPŮSOBENÍ

### Jiná cesta k compileru:
```bash
export COMPILER="/path/to/ifj25"
./run_tests.sh
```

### Jiná cesta k interpretu:
```bash
export INTERPRETER="/path/to/interpreter"
./run_tests.sh
```

### Vlastní testy:
```bash
mkdir my_tests

# Vytvoř my_tests/custom_01.in
# Vytvoř my_tests/custom_01.expected

./advanced_tester.sh my_tests
```

## 📋 VÝSLEDKY

Příklad úspěšného běhu:
```
╔════════════════════════════════════════╗
║    IFJ25 Compiler Testovací Suite      ║
╚════════════════════════════════════════╝

✓ PASS: test_01_basic_output
✓ PASS: test_02_int_operations
✓ PASS: test_03_string_concat
✗ FAIL: test_04_if_condition (výstup se neshoduje)
⚠ MEMORY LEAK: test_05_while_loop

════════════════════════════════════════
           Testování Dokončeno
════════════════════════════════════════

Výsledky:
  Celkem testů:  5
  Úspěšné:       3
  Neúspěšné:     1
  Memory leaky:  1

Detailní výsledky v souboru: test_results.txt
```

## 🐛 TROUBLESHOOTING

### "Permission denied"
```bash
chmod +x *.sh *.py
```

### "Compiler not found"
```bash
export COMPILER="./ifj25"
./run_tests.sh
```

### "Test times out"
- Máš nekonečnou smyčku v kódu?
- Změň timeout v scriptu (výchozí 5 sekund)

### "Výstup OK, ale test selhává"
- Pravděpodobně problém s mezerami
- `advanced_tester.sh` to ignoruje

## 📞 POKROČILÉ FUNKCE

### Ručné spuštění jednoho testu:
```bash
valgrind --leak-check=full ./ifj25 < tests/test_01.in > test.ifjcode
/path/to/interpreter test.ifjcode
cat test.out
```

### Analyzování memory leaku:
```bash
cat tests/test_XX_valgrind.log | grep -A 20 "LEAK SUMMARY"
```

### Hledání selhavších testů:
```bash
grep "FAIL" test_results.txt
```

## 🎓 PŘÍKLADY TESTŮ

### Nejjednoduší test - Hello World:
```ifj25
import "ifj25" for Ifj
class Program {
    static main() {
        Ifj.write(42)
    }
}
```
Očekávaný výstup: `42`

### Test s funkcí:
```ifj25
import "ifj25" for Ifj
class Program {
    static add(a, b) {
        return a + b
    }
    static main() {
        Ifj.write(add(5, 3))
    }
}
```
Očekávaný výstup: `8`

### Test s hexadecimálním outputem:
```ifj25
import "ifj25" for Ifj
class Program {
    static main() {
        var x
        x = 5.5
        Ifj.write(x)
    }
}
```
Očekávaný výstup: `0x1.6p+2` (= 5.5)

## 📈 SOUBOR s PŘÍKAZY

Ke spuštění ALL testů jedním řádkem:
```bash
chmod +x *.sh *.py && make setup && make run-all
```

## ✨ ZÁVĚR

Máš kompletní, produkční testovací systém s:
- ✅ 45+ zabudovaných testů
- ✅ Automatickou normalizací
- ✅ Memory leak detekci
- ✅ Detailním logováním
- ✅ JSON exportem
- ✅ Makefile pro pohodlné spuštění
- ✅ Komplexní dokumentací

**Všechno je ready-to-use. Jen spusť:**

```bash
make setup
make run-all
```

A máš přehled, co u tvého compileru funguje a co ne! 🚀
