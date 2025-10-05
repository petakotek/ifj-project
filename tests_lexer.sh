#!/bin/bash

EXEC=./ifj
TESTDIR=tests

for file in "$TESTDIR"/*.txt; do
    echo ">>> Running test: $file"
    $EXEC < "$file"
    echo "-------------------------------"
done
