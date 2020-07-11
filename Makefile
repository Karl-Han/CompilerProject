CFLAGS = -g -m64
# FLEX_FLAGS = -DLEX_DEBUG
# OBJ = main scanner.c
# 
# main: scanner.c
# 	gcc $(CFLAGS) -o $@ $<
# 
# scanner.c: scanner.l
# 	flex -o $@ $(FLEX_FLAGS) $<
# 
# clean:
# 	rm -rf $(OBJ) main.dSYM
OUT      = main
TESTFILE = gcd.c
SCANNER  = scanner.l
PARSER   = parser.y

CC       = gcc
OBJ      = lex.yy.o y.tab.o test.o gen_dot.o
# TESTOUT  = $(basename $(TESTFILE)).asm
OUTFILES = lex.yy.c y.tab.c y.tab.h y.output $(OUT)

.PHONY: build test simulate clean

build: $(OUT)

test-all: test test1 test2 test3

test1: ./test_cases/test_case1.c
	./$(OUT) < $<

test2: ./test_cases/test_case2.c
	./$(OUT) < $<

test3: ./test_cases/sort.c
	./$(OUT) < $<

test: $(TESTOUT)

clean:
	rm -f *.o $(OUTFILES)

$(TESTOUT): $(TESTFILE) $(OUT)
	./$(OUT) < $< > $@

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ)

lex.yy.c: $(SCANNER) y.tab.c y.tab.h
	flex $<

y.tab.c: $(PARSER)
	bison -vdty $<