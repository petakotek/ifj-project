#!/usr/bin/env bash
# Runs all semantic tests located in exampleCodesIFJ25/sem_tests.
# Expected exit code is encoded in the filename as the number after the last
# underscore (e.g., foo_4.txt -> expects exit 4). Each test file is fed to
# ./project via standard input so no temporary copies are needed.

set -u

TEST_DIR="./testy/semantic_automated_stolen/"
PROJECT_BIN="./ifj25"

# simple ANSI colors (disabled when stdout is not a TTY)
if [[ -t 1 ]]; then
	GREEN=$'\033[32m'
	RED=$'\033[31m'
	YELLOW=$'\033[33m'
	BOLD=$'\033[1m'
	RESET=$'\033[0m'
else
	GREEN=""
	RED=""
	YELLOW=""
	BOLD=""
	RESET=""
fi

if [[ ! -x "${PROJECT_BIN}" ]]; then
	echo "Binary ${PROJECT_BIN} not found or not executable. Run 'make' first." >&2
	exit 1
fi

if [[ ! -d "${TEST_DIR}" ]]; then
	echo "Test directory ${TEST_DIR} not found." >&2
	exit 1
fi

total=0
passed=0
failed=0
skipped=0

printf "${BOLD}Running semantic tests in %s${RESET}\n\n" "${TEST_DIR}"

shopt -s nullglob
for file in "${TEST_DIR}"/*.txt; do
	((total++))
	base="$(basename "${file}")"
	suffix="${base##*_}"
	expected="${suffix%.txt}"

	if ! [[ "${expected}" =~ ^-?[0-9]+$ ]]; then
		printf "${YELLOW}[SKIP]${RESET} %-30s reason: cannot parse expected code from filename\n" "${base}"
		((skipped++))
		continue
	fi

	# Run interpreter with file as stdin; suppress stdout for clean summary.
	if "${PROJECT_BIN}" < "${file}" > /dev/null 2>&1; then
		exit_code=0
	else
		exit_code=$?
	fi

	if [[ "${exit_code}" -eq "${expected}" ]]; then
		printf "${GREEN}[PASS]${RESET} %-30s expected=%s got=%s\n" "${base}" "${expected}" "${exit_code}"
		((passed++))
	else
		printf "${RED}[FAIL]${RESET} %-30s expected=%s got=%s\n" "${base}" "${expected}" "${exit_code}"
		((failed++))
	fi
done
shopt -u nullglob

summary_color="${GREEN}"
fail_color="${RED}"
(( failed > 0 )) && summary_color="${RED}"
(( failed == 0 )) && fail_color="${GREEN}"
skip_color="${YELLOW}"

printf "\n${summary_color}Summary:${RESET} %d total | ${GREEN}%d passed${RESET} | ${fail_color}%d failed${RESET} | ${skip_color}%d skipped${RESET}\n" \
	"${total}" "${passed}" "${failed}" "${skipped}"

(( failed == 0 )) || exit 1
exit 0
