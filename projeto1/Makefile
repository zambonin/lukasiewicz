CC = g++
CFLAGS = -Wall -Wextra -fexceptions -Wno-sign-compare -Wno-unused-function

OUTPUT = lukacompiler

$(OUTPUT): parser.h scanner.cpp main.cpp
	@$(CC) $(CFLAGS) *.cpp -o $(OUTPUT)

scanner.cpp: scanner.l parser.h
	@flex -o scanner.cpp scanner.l

parser.h: parser.y
	@bison -v --defines=parser.h --output=parser.cpp parser.y

clean:
	@rm parser.cpp parser.h parser.output scanner.cpp $(OUTPUT)
