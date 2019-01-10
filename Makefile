INC_DIR = include
SRC_DIR = src

SCANNER_L = $(SRC_DIR)/scanner.l
SCANNER_CPP = $(SCANNER_L:.l=.cpp)

PARSER_Y = $(SRC_DIR)/parser.y
PARSER_CPP = $(PARSER_Y:.y=.cpp)
PARSER_H = $(INC_DIR)/parser.h

SRC_FILES = $(PARSER_CPP) $(SCANNER_CPP) $(wildcard src/*.cpp)
OBJ_FILES = $(SRC_FILES:.cpp=.o)

ENTRY = $(PARSER_Y:.y=)
OUTPUT = lukacompiler

CXXFLAGS = -O2 -Wall -Wextra -std=c++11 -I$(INC_DIR)
LDFLAGS = -lstdc++

all: $(ENTRY)
	mv $^ $(OUTPUT)

$(PARSER_H) $(PARSER_CPP): $(PARSER_Y) /usr/bin/bison
	bison $<

$(SCANNER_CPP): $(SCANNER_L) $(PARSER_H) /usr/bin/flex
	flex -o $@ $<

$(ENTRY): $(OBJ_FILES)

debug: CXXFLAGS += -g
debug: all

test: vtest ptest itest mtest

vtest: $(addsuffix .vtest, $(basename $(wildcard test/valid/**/*.in)))
%.vtest: %.in %.out /usr/bin/cmp all
	@./$(OUTPUT) < $< | cmp -s $(word 2, $?) -

ptest: $(addsuffix .ptest, $(basename $(wildcard test/valid/**/*.in)))
%.ptest: %.in %.out /usr/bin/cmp all
	@./$(OUTPUT) -p < $< | python

itest: $(addsuffix .itest, $(basename $(wildcard test/invalid/**/*.in)))
%.itest: %.in %.out /usr/bin/cmp all
	@./$(OUTPUT) < $< 2>&1 >/dev/null | cmp -s $(word 2, $?) -

# TODO: some invalid inputs are leaking memory on functions
mtest: $(addsuffix .mtest, $(basename $(wildcard test/valid/**/*.in)))
%.mtest: %.in %.out /usr/bin/valgrind all
	@valgrind --leak-check=full --errors-for-leak-kinds=all --error-exitcode=1 \
		--quiet ./$(OUTPUT) < $< >/dev/null 2>/dev/null

clean:
	rm -f $(PARSER_H) $(PARSER_CPP) $(SCANNER_CPP) $(OBJ_FILES) $(OUTPUT)
