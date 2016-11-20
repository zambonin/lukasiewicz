CC = g++
CFLAGS = -O2 -Wall -Wextra -Wno-sign-compare \
		 -Wno-unused-function -Wno-unused-parameter -std=c++11 -I include
LIBS = -lreadline
OUTPUT = lukacompiler

default: grammar
	@$(CC) $(LIBS) $(CFLAGS) $(wildcard **/*.cpp) -o $(OUTPUT)

debug: grammar
	@$(CC) -g $(LIBS) $(CFLAGS) $(wildcard **/*.cpp) -o $(OUTPUT)

grammar:
	@flex -o src/scanner.cpp src/scanner.l
	@bison src/parser.y

test: default
	@cd test/ && ./batch_test.sh ../lukacompiler
	@$(MAKE) -s clean

clean:
	@rm src/parser.cpp src/parser.output \
		include/parser.h src/scanner.cpp $(OUTPUT)
