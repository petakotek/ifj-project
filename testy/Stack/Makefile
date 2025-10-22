# IFJ25 Stack testy
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -pedantic

SRC = dynamic_stack.c stack_test.c support_functions.c lexer.c
OBJ = $(SRC:.c=.o)
EXEC = dynamicStackTest

all: dataGet
	make $(EXEC)

run:  all
	./dynamicStackTest

dataGet:
	cp ../../dynamic_stack.c ./
	cp ../../dynamic_stack.h ./
	cp ../../support_functions.c ./
	cp ../../support_functions.h ./

$(EXEC): $(OBJ)
	cp ../../dynamic_stack.c ./
	cp ../../dynamic_stack.h ./
	$(CC) $(CFLAGS) -o $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean dataGet
