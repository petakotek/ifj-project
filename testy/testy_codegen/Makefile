# Makefile pro IFJ25 Testovací Systém

.PHONY: help setup run-basic run-python run-construction run-all clean view-logs

COMPILER ?= ./ifj25
INTERPRETER ?= /home/tjoslef/Stažené/sys_package/.ic25int-linux-x86_64

# Barvy pro terminál
GREEN := \033[0;32m
RED := \033[0;31m
BLUE := \033[0;34m
NC := \033[0m # No Color

help:
	@echo "$(BLUE)═══════════════════════════════════════════════════════$(NC)"
	@echo "$(BLUE)     IFJ25 Testovací Systém - Makefile Reference$(NC)"
	@echo "$(BLUE)═══════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo "$(GREEN)Dostupné příkazy:$(NC)"
	@echo ""
	@echo "  $(GREEN)make help$(NC)              - Zobrazit tuto pomoc"
	@echo "  $(GREEN)make setup$(NC)             - Příprava (chmod +x na scripty)"
	@echo "  $(GREEN)make run-basic$(NC)         - Spustit základní 20 testů"
	@echo "  $(GREEN)make run-python$(NC)        - Spustit Python verzi testů"
	@echo "  $(GREEN)make run-construction$(NC)  - Spustit 25 stavebních testů"
	@echo "  $(GREEN)make run-all$(NC)           - Spustit všechny testy"
	@echo "  $(GREEN)make view-logs$(NC)         - Zobrazit výsledky"
	@echo "  $(GREEN)make clean$(NC)             - Smazat dočasné soubory"
	@echo "  $(GREEN)make stats$(NC)             - Zobrazit statistiku"
	@echo ""
	@echo "$(GREEN)Příklady:$(NC)"
	@echo ""
	@echo "  # Spustit základní testy"
	@echo "  make run-basic"
	@echo ""
	@echo "  # Spustit všechny testy (základní + stavební)"
	@echo "  make run-all"
	@echo ""
	@echo "  # Přizpůsobit cestu k compileru"
	@echo "  make run-basic COMPILER=/path/to/ifj25"
	@echo ""
	@echo "$(BLUE)═══════════════════════════════════════════════════════$(NC)"

setup:
	@echo "$(GREEN)⚙️  Příprava testovacího systému...$(NC)"
	@chmod +x run_tests.sh test_runner.py advanced_tester.sh generate_construction_tests.sh QUICKSTART.sh 2>/dev/null || true
	@echo "$(GREEN)✓ Hotovo!$(NC)"
	@echo ""
	@echo "Nyní můžeš spustit: $(GREEN)make run-all$(NC)"

run-basic: setup
	@echo ""
	@echo "$(BLUE)╔════════════════════════════════════════╗$(NC)"
	@echo "$(BLUE)║  Spouštění základních 20 testů         ║$(NC)"
	@echo "$(BLUE)╚════════════════════════════════════════╝$(NC)"
	@echo ""
	@export COMPILER=$(COMPILER) && export INTERPRETER=$(INTERPRETER) && ./run_tests.sh
	@echo ""
	@echo "$(GREEN)✓ Výsledky:$(NC)"
	@tail -20 test_results.txt

run-python: setup
	@echo ""
	@echo "$(BLUE)╔════════════════════════════════════════╗$(NC)"
	@echo "$(BLUE)║  Spouštění Python testů (10 testů)    ║$(NC)"
	@echo "$(BLUE)╚════════════════════════════════════════╝$(NC)"
	@echo ""
	@export COMPILER=$(COMPILER) && export INTERPRETER=$(INTERPRETER) && python3 test_runner.py

run-construction: setup
	@echo ""
	@echo "$(BLUE)╔════════════════════════════════════════╗$(NC)"
	@echo "$(BLUE)║  Generování a spuštění stavebních     ║$(NC)"
	@echo "$(BLUE)║  25 testů pro všechny konstrukce      ║$(NC)"
	@echo "$(BLUE)╚════════════════════════════════════════╝$(NC)"
	@echo ""
	@./generate_construction_tests.sh
	@export COMPILER=$(COMPILER) && export INTERPRETER=$(INTERPRETER) && ./advanced_tester.sh ./construction_tests

run-all: setup run-basic run-construction
	@echo ""
	@echo "$(GREEN)╔════════════════════════════════════════╗$(NC)"
	@echo "$(GREEN)║       Všechny testy byly spuštěny      ║$(NC)"
	@echo "$(GREEN)╚════════════════════════════════════════╝$(NC)"

clean:
	@echo "$(RED)🗑️  Čistění dočasných souborů...$(NC)"
	@rm -rf tests tests_python construction_tests
	@rm -f *.log test_results.txt test_results.json test_results_python.json
	@echo "$(GREEN)✓ Hotovo!$(NC)"

view-logs:
	@echo "$(BLUE)═════════════════════════════════════════$(NC)"
	@echo "$(BLUE)     Dostupné log soubory               $(NC)"
	@echo "$(BLUE)═════════════════════════════════════════$(NC)"
	@echo ""
	@ls -lh test_results* 2>/dev/null || echo "Zatím nejsou dostupné logové soubory"
	@echo ""
	@if [ -f test_results.txt ]; then \
		echo "$(GREEN)Poslední výsledky (posledních 30 řádků):$(NC)"; \
		tail -30 test_results.txt; \
	fi

stats:
	@echo "$(BLUE)═════════════════════════════════════════$(NC)"
	@echo "$(BLUE)     Statistika testů                    $(NC)"
	@echo "$(BLUE)═════════════════════════════════════════$(NC)"
	@echo ""
	@if [ -f test_results.txt ]; then \
		echo "Statistika z test_results.txt:"; \
		echo ""; \
		echo "Úspěšné testy: $$(grep -c "RESULT: PASS" test_results.txt || echo 0)"; \
		echo "Neúspěšné testy: $$(grep -c "RESULT: FAIL" test_results.txt || echo 0)"; \
		echo "Testy s memory leaky: $$(grep -c "MEMORY LEAK" test_results.txt || echo 0)"; \
	fi
	@echo ""
	@if [ -f test_results.json ]; then \
		echo "Statistika z test_results.json:"; \
		cat test_results.json; \
	fi

.DEFAULT_GOAL := help
