#!/bin/bash

# QUICK START GUIDE - IFJ25 Testovací Systém
# Tento skript ti automaticky nastaví a spustí testy

cat << 'EOF'

╔══════════════════════════════════════════════════════════════════╗
║                   IFJ25 QUICK START GUIDE                        ║
║              Kompletní testovací systém pro váš compiler          ║
╚══════════════════════════════════════════════════════════════════╝

📁 CO JI MÁTE:
─────────────────────────────────────────────────────────────────
✓ run_tests.sh                    - 20 základních testů
✓ test_runner.py                  - Python verze s lepší normalizací
✓ advanced_tester.sh              - Systém pro vlastní testy
✓ generate_construction_tests.sh   - Generátor 25 stavebních testů
✓ README_TESTY.md                 - Detailní dokumentace

🚀 ZAČÍNÁME (3 kroky):
─────────────────────────────────────────────────────────────────

KROK 1: Udělej scripty spustitelné
─────────────────────────────────
chmod +x run_tests.sh
chmod +x test_runner.py
chmod +x advanced_tester.sh
chmod +x generate_construction_tests.sh

KROK 2: Zkus základní testy
──────────────────────────
./run_tests.sh

Výsledky se budou vypisovat v terminálu s barvami:
  ✓ PASS  - test prošel
  ✗ FAIL  - test selhal
  ⚠ MEMORY LEAK - zjištěn memory leak v testě

KROK 3: Zkus pokročilé testy na základě všech konstrukcí
─────────────────────────────────────────────────────────
./generate_construction_tests.sh
./advanced_tester.sh ./construction_tests

═════════════════════════════════════════════════════════════════════

📊 PŘÍKLADY VÝSTUPU:
──────────────────────────────────────────────────────────────────

Když je test OK:
$ ./run_tests.sh
✓ PASS: test_01_basic_output
✓ PASS: test_02_int_operations
✓ PASS: test_03_string_concat
...

Detailní log:
$ cat test_results.txt
========================================
Test: test_01_basic_output
========================================
Spouštím: valgrind --leak-check=full ./ifj25 < tests/test_01.in
Očekávaný výstup: 42
Skutečný výstup: 42
RESULT: PASS

═════════════════════════════════════════════════════════════════════

⚙️ PŘIZPŮSOBENÍ:
──────────────────────────────────────────────────────────────────

Jestli máš compiler jinde:
$ export COMPILER="/path/to/your/ifj25"
$ ./run_tests.sh

Jestli máš interpreter jinde:
$ export INTERPRETER="/path/to/interpreter"
$ ./run_tests.sh

═════════════════════════════════════════════════════════════════════

📝 VYTVOŘENÍ VLASTNÍCH TESTŮ:
──────────────────────────────────────────────────────────────────

1. Vytvoř složku
   mkdir my_tests

2. Přidej test_example.in:
   my_tests/test_example.in
   ─────────────────────────
   import "ifj25" for Ifj
   class Program {
       static main() {
           Ifj.write(42)
       }
   }

3. Přidej test_example.expected:
   my_tests/test_example.expected
   ────────────────────────────────
   42

4. Spusť tester:
   ./advanced_tester.sh my_tests

═════════════════════════════════════════════════════════════════════

🔍 STRUKTURA VÝSTUPŮ:
──────────────────────────────────────────────────────────────────

Po spuštění testů si najdeš:

📂 ./tests/
   ├── test_01.in               (vstupní kod)
   ├── test_01.expected         (očekávaný výstup)
   ├── test_01.ifjcode          (vygenerovaný IFJcode25)
   ├── test_01.out              (skutečný výstup)
   └── test_01_valgrind.log     (memory report)

📄 test_results.txt             (detailní text log)
📄 test_results.json            (JSON se statistikou)

═════════════════════════════════════════════════════════════════════

💡 UŽITEČNÉ PŘÍKAZY:
──────────────────────────────────────────────────────────────────

# Spustit jen jeden test
./advanced_tester.sh ./tests | grep test_05

# Vidět memory leak report
cat tests/test_05_while_loop_valgrind.log | grep "LEAK SUMMARY" -A 20

# Ručně otestovat jeden soubor
valgrind --leak-check=full ./ifj25 < my_tests/test_example.in > output.ifjcode
/home/tjoslef/Stažené/sys_package/.ic25int-linux-x86_64 output.ifjcode

# Vidět detailní výsledky v JSON
cat test_results.json | python3 -m json.tool

═════════════════════════════════════════════════════════════════════

🎯 TYPICKÉ WORKFLOW:
──────────────────────────────────────────────────────────────────

1. Vyvíjíš svůj compiler
   $ make           # nebo tvůj build příkaz
   $ ./ifj25 --version

2. Spustíš automatické testy
   $ ./run_tests.sh

3. Pokud vidíš FAIL, zkontroluj:
   $ cat test_results.txt          # co se stalo
   $ cat tests/test_XX.ifjcode     # co se vygenerovalo
   $ cat tests/test_XX.out         # jaký byl výstup
   $ cat tests/test_XX_valgrind.log # je memory leak?

4. Opravíš bug v compileru
   $ nano src/codegen.c

5. Znova vyvítej
   $ make
   $ ./run_tests.sh

═════════════════════════════════════════════════════════════════════

❓ TROUBLESHOOTING:
──────────────────────────────────────────────────────────────────

Q: "Permission denied"
A: chmod +x *.sh *.py

Q: "Compiler not found"
A: Zkontroluj cestu, použij: export COMPILER="./ifj25"

Q: "Timeout"
A: Máš nekonečnou smyčku? Zkontroluj test kód.

Q: "Memory leak warning"
A: V test resultech je adresa na valgrind log, tam vidíš detaily.

Q: "Test selhává, ale vidím OK výstup"
A: Pravděpodobně problem s mezerami/newlines. 
   Advanced_tester to ignoruje.

═════════════════════════════════════════════════════════════════════

📚 VÍCE INFORMACÍ:
──────────────────────────────────────────────────────────────────

Detailní dokumentace: README_TESTY.md

Obsah README_TESTY.md:
- Přehled všech 3 skriptů
- Detailní příklady testů
- Jak tvořit vlastní testy
- Advanced features
- Kompletní troubleshooting

═════════════════════════════════════════════════════════════════════

🎉 TAKŽE JDEME NA TO!

Zkopíruj a spusť přímo:
───────────────────────────────────────

chmod +x run_tests.sh test_runner.py advanced_tester.sh generate_construction_tests.sh

echo "Test 1: Základní testy..."
./run_tests.sh

echo ""
echo "Test 2: Stavební testy..."
./generate_construction_tests.sh
./advanced_tester.sh ./construction_tests

echo ""
echo "Hotovo! 🎉"

═════════════════════════════════════════════════════════════════════

Otázky? Podívej se do README_TESTY.md nebo spusť:
  ./run_tests.sh --help

Hodně štěstí! 🚀

EOF
