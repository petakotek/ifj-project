# IFJ25
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2

SRC = main.c lexer.c  parser.c support_functions.c symtable.c dynamic_stack.c ast_strom.c
OBJ = $(SRC:.c=.o)
EXEC = ifj25

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
