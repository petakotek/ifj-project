#!/usr/bin/env python3
"""
Pokročilý testovací skript pro IFJ25 compiler
Zvládá porovnávání výstupů včetně hexadecimálních čísel
"""

import os
import subprocess
import sys
from pathlib import Path
import re
import json
from datetime import datetime

class IFJ25Tester:
    def __init__(self, compiler="../.././ifj25", interpreter="$HOME/Stažené/sys_package/ic25int-linux-x86_64"):
        self.compiler = compiler
        self.interpreter = interpreter
        self.test_dir = Path("./tests_python")
        self.results = []
        self.test_dir.mkdir(exist_ok=True)

    def normalize_hex(self, value):
        """Normalizuje hexadecimální reprezentaci floatu"""
        # Kontrola zda je to hex reprezentace floatu (0x1.23p+4)
        hex_pattern = r'0x[0-9a-fA-F]+(\.[0-9a-fA-F]+)?p[+-]?\d+'
        if re.match(hex_pattern, value):
            try:
                # Převeď hex na float
                float_val = float(value)
                return float_val
            except:
                return value
        return value

    def normalize_output(self, output):
        """Normalizuje výstup - odstraní bílé znaky a normalizuje čísla"""
        # Odstraň všechny bílé znaky
        normalized = output.strip()

        # Rozdělí podle řádků/mezer a normalizuj čísla
        tokens = []
        for token in re.split(r'[\s\n]+', normalized):
            if token:
                # Pokus se normalizovat číslo
                try:
                    if '0x' in token and 'p' in token:
                        # Hex float
                        tokens.append(str(self.normalize_hex(token)))
                    elif '.' in token:
                        # Decimal float
                        tokens.append(str(float(token)))
                    else:
                        # Int
                        tokens.append(str(int(token)))
                except:
                    tokens.append(token)

        return ''.join(tokens)

    def run_compiler(self, test_name, input_file):
        """Spustí compiler přes valgrind"""
        output_code = self.test_dir / f"{test_name}.ifjcode"
        valgrind_log = self.test_dir / f"{test_name}_valgrind.log"

        try:
            result = subprocess.run(
                f"valgrind --leak-check=full --log-file={valgrind_log} {self.compiler}",
                stdin=open(input_file, 'r'),
                stdout=open(output_code, 'w'),
                stderr=subprocess.PIPE,
                shell=True,
                timeout=10
            )

            return result.returncode == 0, str(output_code), valgrind_log
        except subprocess.TimeoutExpired:
            return False, None, None
        except Exception as e:
            print(f"Chyba při spuštění kompileru: {e}")
            return False, None, None

    def run_interpreter(self, ifjcode_file):
        """Spustí interpret na generovaném kódu"""
        try:
            result = subprocess.run(
                f"{self.interpreter} {ifjcode_file}",
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                shell=True,
                timeout=5,
                text=True
            )
            return result.stdout, result.stderr, result.returncode
        except subprocess.TimeoutExpired:
            return None, "TIMEOUT", 1
        except Exception as e:
            return None, str(e), 1

    def check_memory_leaks(self, valgrind_log):
        """Kontroluje memory leaky v valgrind logu"""
        try:
            with open(valgrind_log, 'r') as f:
                content = f.read()
                if 'definitely lost' in content or 'indirectly lost' in content:
                    return True  # Memory leak nalezen
        except:
            pass
        return False

    def run_test(self, test_name, input_code, expected_output):
        """Spustí jeden test"""
        input_file = self.test_dir / f"{test_name}.in"
        expected_file = self.test_dir / f"{test_name}.expected"

        # Uložení vstupního kódu
        with open(input_file, 'w') as f:
            f.write(input_code)

        # Uložení očekávaného výstupu
        with open(expected_file, 'w') as f:
            f.write(expected_output)

        # Spuštění kompileru
        success, ifjcode_file, valgrind_log = self.run_compiler(test_name, input_file)

        if not success or not ifjcode_file:
            self.results.append({
                'test': test_name,
                'status': 'FAIL',
                'reason': 'Kompilace selhala',
                'memory_leak': False
            })
            print(f"✗ {test_name}: Kompilace selhala")
            return False

        # Spuštění interpretu
        output, stderr, exit_code = self.run_interpreter(ifjcode_file)

        if output is None:
            self.results.append({
                'test': test_name,
                'status': 'FAIL',
                'reason': f'Interpret selhał: {stderr}',
                'memory_leak': False
            })
            print(f"✗ {test_name}: Interpret selhał")
            return False

        # Porovnání výstupů
        actual_norm = self.normalize_output(output)
        expected_norm = self.normalize_output(expected_output)

        memory_leak = self.check_memory_leaks(valgrind_log) if valgrind_log else False

        if actual_norm == expected_norm:
            self.results.append({
                'test': test_name,
                'status': 'PASS',
                'memory_leak': memory_leak
            })
            leak_warning = " ⚠ MEMORY LEAK" if memory_leak else ""
            print(f"✓ {test_name}{leak_warning}")
            return True
        else:
            self.results.append({
                'test': test_name,
                'status': 'FAIL',
                'reason': f'Výstup se liší',
                'expected': expected_norm,
                'actual': actual_norm,
                'memory_leak': memory_leak
            })
            print(f"✗ {test_name}: Výstup se liší")
            print(f"  Očekáváno:  {expected_norm[:60]}")
            print(f"  Skutečně:   {actual_norm[:60]}")
            return False

    def print_summary(self):
        """Vypíše souhrn výsledků"""
        passed = sum(1 for r in self.results if r['status'] == 'PASS')
        failed = sum(1 for r in self.results if r['status'] == 'FAIL')
        memory_leaks = sum(1 for r in self.results if r.get('memory_leak', False))

        print("\n" + "="*50)
        print("SOUHRN TESTŮ")
        print("="*50)
        print(f"Celkem testů:      {len(self.results)}")
        print(f"Úspěšné:           {passed}")
        print(f"Neúspěšné:         {failed}")
        print(f"Memory leaky:      {memory_leaks}")
        print("="*50 + "\n")

        # Detailní výstup
        with open('test_results_python.json', 'w') as f:
            json.dump(self.results, f, indent=2, ensure_ascii=False)

        print("Detailní výsledky uloženy v: test_results_python.json")

        return failed == 0

def main():
    tester = IFJ25Tester()

    print("╔══════════════════════════════════════╗")
    print("║  IFJ25 Python Testovací Suite        ║")
    print("╚══════════════════════════════════════╝\n")

    # Test 1: Jednoduchý výstup
    tester.run_test(
        "test_01_simple",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        Ifj.write(42)
    }
}''',
        "42\n"
    )

    # Test 2: Sčítání
    tester.run_test(
        "test_02_addition",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        var a
        a = 10
        var b
        b = 5
        Ifj.write(a + b)
    }
}''',
        "15\n"
    )

    # Test 3: String concatenace
    tester.run_test(
        "test_03_string_concat",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        Ifj.write("Hello" + " " + "World")
    }
}''',
        "HelloWorld\n"
    )

    # Test 4: If podmínka
    tester.run_test(
        "test_04_if",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        var x
        x = 10
        if (x > 5) {
            Ifj.write("yes")
        } else {
            Ifj.write("no")
        }
    }
}''',
        "yes\n"
    )

    # Test 5: While cyklus
    tester.run_test(
        "test_05_while",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        var i
        i = 3
        while (i > 0) {
            Ifj.write(i)
            i = i - 1
        }
    }
}''',
        "3\n2\n1\n"
    )

    # Test 6: Funkce
    tester.run_test(
        "test_06_function",
        '''import "ifj25" for Ifj
class Program {
    static add(x, y) {
        return x + y
    }

    static main() {
        Ifj.write(add(5, 3))
    }
}''',
        "8\n"
    )

    # Test 7: Float operace (hex výstup)
    tester.run_test(
        "test_07_float",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        var x
        x = 2.5
        var y
        y = 2.0
        Ifj.write(x + y)
    }
}''',
        "0x1.2p+2\n"  # 4.5 v hex
    )

    # Test 8: Type checking
    tester.run_test(
        "test_08_type_check",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        var x
        x = 42
        if (x is Num) {
            Ifj.write("num")
        }
    }
}''',
        "num\n"
    )

    # Test 9: Globální proměnné
    tester.run_test(
        "test_09_global",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        __g = 99
        Ifj.write(__g)
    }
}''',
        "99\n"
    )

    # Test 10: Logické operátory
    tester.run_test(
        "test_10_logical",
        '''import "ifj25" for Ifj
class Program {
    static main() {
        if (true && true) {
            Ifj.write("and")
        }
        if (false || true) {
            Ifj.write("or")
        }
    }
}''',
        "and\nor\n"
    )

    # Vypíšení souhrnu
    success = tester.print_summary()

    return 0 if success else 1

if __name__ == '__main__':
    sys.exit(main())
