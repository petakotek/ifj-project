import subprocess
import glob
import os
import difflib
import re

EXEC = "./ifj25"   # tvoje binárka
TESTDIR = "testy/tokenazir"

def normalize_output(text):
    lines = [line.strip() for line in text.split('\n') if line.strip()]
    normalized_text = "\n".join(lines)
    return normalized_text.strip()

def run_test(input_file, expected_file):
    with open(input_file, "r") as f:
        result = subprocess.run([EXEC], stdin=f, capture_output=True, text=True)

    # načíst očekávaný výstup
    with open(expected_file, "r") as f:
        expected = f.read()

    # skutečný výstup
    actual = result.stdout
    normalized_expected = normalize_output(expected)
    normalized_actual = normalize_output(actual)
    if normalized_actual == normalized_expected:
        print(f"[ OK ] {os.path.basename(input_file)}")
    else:
        print(f"[FAIL] {os.path.basename(input_file)}")
        print("Diff (expected vs actual):")
        diff = difflib.unified_diff(
            expected.splitlines(),
            actual.splitlines(),
            fromfile="expected",
            tofile="actual",
            lineterm=""
        )
        for line in diff:
            print(line)

def main():
    print("start ")
    inputs = sorted(glob.glob(f"{TESTDIR}/*.in"))
    print(f"Found {len(inputs)} input files in {TESTDIR}.")
    for infile in inputs:
        outfile = infile.replace(".in", ".out")
        if not os.path.exists(outfile):
            print(f"⚠️  Missing expected output file: {outfile}")
            continue
        run_test(infile, outfile)

if __name__ == "__main__":
    main()
