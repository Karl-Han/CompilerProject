CFLAGS = -g -m64 -c
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
CPP      = g++
OBJ      = lex.yy.o y.tab.o test.o gen_dot.o 
OBJCPP	 = analyze.o tables.o cgen.o code.o
SRC      = lex.yy.c y.tab.c gen_dot.c 
SRCPP	 = analyze.cpp tables.cpp test.cpp cgen.cpp code.cpp
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

$(OUT): $(OBJ) $(OBJCPP) lex.yy.c y.tab.c
	$(CPP) -o $(OUT) $(OBJ) $(OBJCPP)

$(OBJ): lex.yy.c y.tab.c
	$(CC) $(CFLAGS) $(SRC)

$(OBJCPP): lex.yy.c y.tab.c
	$(CPP) $(CFLAGS) $(SRCPP)

lex.yy.c: $(SCANNER) y.tab.c y.tab.h
	flex $<

y.tab.c: $(PARSER)
	bison -vdty $<
