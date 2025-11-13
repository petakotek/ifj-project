#!/usr/bin/env python3
import subprocess
import glob
import os
import sys
from typing import Optional

EXEC = "./ifj25"          # binárka z Makefile
TESTDIR = "testy/semantic_automated"  # kde máš .in/.rc (.out je voliteľné)


def normalize(text: str) -> str:
    """Odstráni trailing medzery a prázdne riadky, aby diff nebol citlivý na whitespace."""
    lines = [line.rstrip() for line in text.splitlines()]
    # vyhodí úplne prázdne riadky na začiatku/konci
    while lines and lines[0] == "":
        lines.pop(0)
    while lines and lines[-1] == "":
        lines.pop()
    return "\n".join(lines)


def run_single_test(infile: str) -> bool:
    base = os.path.splitext(infile)[0]
    rc_file = base + ".rc"
    out_file = base + ".out"

    expected_rc = 0
    if os.path.exists(rc_file):
        with open(rc_file, "r", encoding="utf-8") as f:
            expected_rc = int(f.read().strip())

    expected_out: Optional[str] = None
    if os.path.exists(out_file):
        with open(out_file, "r", encoding="utf-8") as f:
            expected_out = normalize(f.read())

    # spustíme prekladač, vstup je obsah .in
    with open(infile, "rb") as f:
        proc = subprocess.run(
            [EXEC],
            stdin=f,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    real_rc = proc.returncode
    real_out = normalize(proc.stdout.decode("utf-8", errors="ignore"))
    real_err = proc.stderr.decode("utf-8", errors="ignore")

    name = os.path.basename(base)

    ok = True

    if real_rc != expected_rc:
        print(f"[FAIL] {name}: očakávaný rc {expected_rc}, dostal som {real_rc}")
        ok = False

    if expected_out is not None and real_out != expected_out:
        print(f"[FAIL] {name}: stdout sa nezhoduje s očakávaným výstupom")
        print("----- očakávané -----")
        print(expected_out)
        print("----- skutočné  -----")
        print(real_out)
        ok = False

    if ok:
        print(f"[ OK ] {name}")
    else:
        # pri chybe aspoň ukážeme stderr, môže pomôcť pri debugovaní
        if real_err.strip():
            print("----- stderr -----")
            print(real_err.strip())

    return ok


def main() -> int:
    if not os.path.exists(EXEC):
        print(f"Binárka {EXEC} neexistuje. Najprv spusti `make`.")
        return 1

    pattern = os.path.join(TESTDIR, "*.in")
    inputs = sorted(glob.glob(pattern))
    if not inputs:
        print(f"V {TESTDIR} som nenašiel žiadne .in súbory.")
        return 1

    print(f"Našiel som {len(inputs)} testov v {TESTDIR}.")

    passed = 0
    for infile in inputs:
        if run_single_test(infile):
            passed += 1

    print(f"\nHotovo: {passed}/{len(inputs)} testov prešlo.")
    return 0 if passed == len(inputs) else 1


if __name__ == "__main__":
    sys.exit(main())
