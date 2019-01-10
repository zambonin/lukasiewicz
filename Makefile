CXXFLAGS = -O2 -Wall -Wextra -std=c++11 -I include
OUTPUT = lukacompiler

all:
	flex -o src/scanner.cpp src/scanner.l
	bison src/parser.y
	$(CXX) $(CXXFLAGS) src/*.cpp -o $(OUTPUT)

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
	rm -f src/parser.cpp include/parser.h src/scanner.cpp $(OUTPUT)
