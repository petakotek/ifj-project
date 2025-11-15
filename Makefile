# IFJ25
CC = cc
CFLAGS = -std=c11 -Wall -Wextra -O2 -pedantic -Werror=implicit-function-declaration -g -D_POSIX_C_SOURCE=200809L

SRC = main.c lexer.c  parser.c support_functions.c symtable.c dynamic_stack.c ast_strom.c expr.c codegen.c builtin_functions.c semantic_analysis.c
OBJ = $(SRC:.c=.o)
EXEC = ifj25

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^  -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean

# TODO zabalení dle zadání
# pack