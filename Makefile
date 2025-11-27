# IFJ25
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -pedantic -Werror=implicit-function-declaration -g -lm -D_POSIX_C_SOURCE=200809L
DEV_CFLAGS = -DENABLE_DEVELOPMENT_LOGGING

SRC = main.c lexer.c  parser.c support_functions.c symtable.c dynamic_stack.c ast.c expressions.c semantic_analysis.c code_generation.c builtin_functions.c
OBJ = $(SRC:.c=.o)
EXEC = ifj25

all: clean $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^  -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
	rm -rf xvoldrv00.zip ./tmpIsItOk ready.zip testy.zip


dev: clean
	$(MAKE) CFLAGS="$(CFLAGS) $(DEV_CFLAGS)"


.PHONY: all clean pack dev pack_merlin_test

pack:
	rm -rf xvoldrv00.zip ./tmpIsItOk
	clang-format -i *.c *.h
	zip xvoldrv00.zip *.c *.h rozdeleni rozsireni dokumentace.pdf Makefile
	mkdir ./tmpIsItOk
	./is_it_ok.sh xvoldrv00.zip ./tmpIsItOk

pack_merlin_test: pack
	rm -rf testy.zip ready.zip
	zip -r testy.zip ./testy *.sh ic25int-linux-x86_64 *.py
	zip ready.zip *.zip