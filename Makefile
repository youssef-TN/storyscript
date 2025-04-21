CC = gcc
CFLAGS = -Wall -g
LEX = flex
YACC = bison

# Main target
all: storyscript

# Generate parser (creates parser.c and parser.h)
parser.c parser.h: storyscript.y
	$(YACC) -d -o parser.c storyscript.y

# Generate lexer (creates lexer.c)
lexer.c: storyscript.l parser.h
	$(LEX) -o lexer.c storyscript.l

# Compile the program
storyscript: lexer.c parser.c
	$(CC) $(CFLAGS) -o storyscript lexer.c parser.c -lfl

# Clean up generated files
clean:
	rm -f storyscript lexer.c parser.c parser.h *.o

# Test the program with a sample file
test: storyscript
	@echo "Running test with simple_adventure.story..."
	./storyscript simple_adventure.story