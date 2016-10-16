CC = g++
CFLAGS = -Wall -Wextra -fexceptions -Wno-sign-compare -Wno-unused-function \
		 -Wno-unused-parameter -std=c++11 -I include

OUTPUT = lukacompiler

all: flex bison cfiles

flex:
	@flex -o src/scanner.cpp src/scanner.l

bison:
	@bison -v --defines=include/parser.h --output=src/parser.cpp src/parser.y

cfiles:
	@$(CC) $(CFLAGS) $(wildcard **/*.cpp) -o $(OUTPUT)

clean:
	@rm src/parser.{cpp,output} include/parser.h src/scanner.cpp $(OUTPUT)
