#!/bin/bash

# ALL-IN-ONE Test Runner
# Spusť vše a zadrž si shrnutí výsledků

set -e  # Zastavit na chybě

cat << 'EOF'

╔══════════════════════════════════════════════════════════════════╗
║           IFJ25 ALL-IN-ONE TEST RUNNER                           ║
║        Spustím všechny testy a vytvořím přehled                 ║
╚══════════════════════════════════════════════════════════════════╝

EOF

# Ověř, že máš compiler
if [ ! -f "../.././ifj25" ]; then
    echo "❌ CHYBA: Soubor ./ifj25 nenalezen!"
    echo "Prosím, nejprve vytvořit compiler: make"
    exit 1
fi

# Ověř, že máš interpreter
INTERPRETER="$HOME/Stažené/sys_package/ic25int-linux-x86_64"
if [ ! -f "$INTERPRETER" ]; then
    echo "❌ CHYBA: Interpreter nenalezen na: $INTERPRETER"
    echo "Prosím, zkontroluj cestu."
    exit 1
fi

echo "✓ Compiler nalezen: $(./ifj25 --version 2>/dev/null || echo 'OK')"
echo "✓ Interpreter nalezen"
echo ""

# Příprava
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📋 KROK 1: Příprava skriptů"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

chmod +x run_tests.sh test_runner.py advanced_tester.sh generate_construction_tests.sh 2>/dev/null || true
echo "✓ Scripty připraveny"
echo ""

# Čistění starých výsledků
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🧹 KROK 2: Čistění starých výsledků"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

rm -rf tests tests_python construction_tests 2>/dev/null || true
rm -f test_results*.txt test_results*.json test_log*.txt 2>/dev/null || true
echo "✓ Vyčištěno"
echo ""

# Test 1: Základní testy
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🧪 KROK 3: Spouštění základních 20 testů"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

export COMPILER="./ifj25"
export INTERPRETER="$INTERPRETER"

if ./run_tests.sh > /tmp/basic_tests.log 2>&1; then
    BASIC_PASS=$(grep -c "RESULT: PASS" test_results.txt 2>/dev/null || echo 0)
    BASIC_FAIL=$(grep -c "RESULT: FAIL" test_results.txt 2>/dev/null || echo 0)
    echo "✓ Základní testy hotovy"
    echo "  Úspěšné: $BASIC_PASS"
    echo "  Neúspěšné: $BASIC_FAIL"
else
    echo "⚠ Základní testy měly chyby"
    BASIC_PASS=0
    BASIC_FAIL=20
fi
echo ""

# Test 2: Stavební testy
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🏗️  KROK 4: Generování a spuštění 25 stavebních testů"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

./generate_construction_tests.sh > /dev/null 2>&1

if ./advanced_tester.sh ./construction_tests > /tmp/construction_tests.log 2>&1; then
    CONSTRUCTION_PASS=$(grep -c '"status": "PASS"' test_results.json 2>/dev/null || echo 0)
    CONSTRUCTION_FAIL=$(grep -c '"status": "FAIL"' test_results.json 2>/dev/null || echo 0)
    echo "✓ Stavební testy hotovy"
    echo "  Úspěšné: $CONSTRUCTION_PASS"
    echo "  Neúspěšné: $CONSTRUCTION_FAIL"
else
    echo "⚠ Stavební testy měly chyby"
    CONSTRUCTION_PASS=0
    CONSTRUCTION_FAIL=25
fi
echo ""

# Shrnutí
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 FINÁLNÍ SHRNUTÍ"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

TOTAL=$((BASIC_PASS + BASIC_FAIL + CONSTRUCTION_PASS + CONSTRUCTION_FAIL))
PASS=$((BASIC_PASS + CONSTRUCTION_PASS))
FAIL=$((BASIC_FAIL + CONSTRUCTION_FAIL))
PERCENT=$((PASS * 100 / TOTAL))

cat << EOF
┌──────────────────────────────────────────────────────────────────┐
│                    CELKOVÉ VÝSLEDKY                               │
├──────────────────────────────────────────────────────────────────┤
│  Celkem testů:              $TOTAL                                │
│  ✓ Úspěšné:                 $PASS                                 │
│  ✗ Neúspěšné:               $FAIL                                 │
│  Success rate:              $PERCENT%                             │
└──────────────────────────────────────────────────────────────────┘

DETAILY:
  Základní testy (20):         $BASIC_PASS/$((BASIC_PASS + BASIC_FAIL))
  Stavební testy (25):         $CONSTRUCTION_PASS/$((CONSTRUCTION_PASS + CONSTRUCTION_FAIL))
EOF

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📁 DOSTUPNÉ VÝSLEDKY:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📄 Textové loggy:"
echo "   test_results.txt              (základní testy - detailně)"
echo "   test_log_*.txt                (stavební testy - detailně)"
echo ""
echo "📊 JSON výsledky:"
echo "   test_results.json             (stavební testy - strojově čitelné)"
echo ""
echo "📂 Složky s testy:"
echo "   ./tests/                      (základní - .in, .ifjcode, .out)"
echo "   ./construction_tests/         (stavební - stejná struktura)"
echo ""

# Pokročilý výstup
if [ $FAIL -gt 0 ]; then
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🔍 SELHAVŠÍ TESTY (pro debug):"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""

    echo "Základní testy s chybou:"
    grep "Test: test_" test_results.txt 2>/dev/null | grep -A 1 "FAIL" | head -10 || true

    echo ""
    echo "Pro detailní výstup konkrétního testu:"
    echo "  cat test_results.txt | grep -A 10 'test_XX'"
    echo ""
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Závěrečné doporučení
if [ $FAIL -eq 0 ]; then
    echo "✨ ÚSPĚCH! Všechny testy prošly! 🎉"
else
    echo "⚠️  Některé testy selhaly."
    echo ""
    echo "Doporučení pro debug:"
    echo "1. Podívej se na test_results.txt pro detaily"
    echo "2. Zkontroluj ./tests/ pro vygenerované soubory"
    echo "3. Spusť: valgrind --leak-check=full ./ifj25 < tests/test_XX.in"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║              Testování dokončeno!                                ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""

# Exit code
[ $FAIL -eq 0 ] && exit 0 || exit 1
