# IFJ25
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -pedantic -Werror=implicit-function-declaration -g -lm -D_POSIX_C_SOURCE=200809L
DEV_CFLAGS = -DENABLE_DEVELOPMENT_LOGGING

SRC = main.c lexer.c  parser.c support_functions.c symtable.c dynamic_stack.c ast_strom.c expr.c semantic_analysis.c codegen.c builtin_functions.c
OBJ = $(SRC:.c=.o)
EXEC = ifj25

all: clean $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^  -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
	rm -rf xvoldrv00.zip ./tmpIsItOk


dev: clean
	$(MAKE) CFLAGS="$(CFLAGS) $(DEV_CFLAGS)"


.PHONY: all clean

# TODO zabalení dle zadání
pack:
	rm -rf xvoldrv00.zip ./tmpIsItOk
#	clang-format -i *.c *.h # TODO odkomentovat
	zip xvoldrv00.zip *.c *.h rozdeleni rozsireni dokumentace.pdf Makefile
	mkdir ./tmpIsItOk
	./is_it_ok.sh xvoldrv00.zip ./tmpIsItOk