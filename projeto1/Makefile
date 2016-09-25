CC = g++
CFLAGS = -Wall -Wextra -fexceptions -Wno-sign-compare -Wno-unused-function -std=c++11

OUTPUT = lukacompiler

all: flex bison cfiles

cfiles:
	@$(CC) $(CFLAGS) $(wildcard **/*.cpp) *.cpp -o $(OUTPUT)

flex: scanner.l
	@flex -o scanner.cpp scanner.l

bison: parser.y
	@bison -v --defines=parser.h --output=parser.cpp parser.y

clean:
	@rm parser.cpp parser.h parser.output scanner.cpp $(OUTPUT)
