CFLAGS = -g
FLEX_FLAGS = -DLEX_DEBUG
OBJ = main scanner.c

main: scanner.c
	gcc $(CFLAGS) -o $@ $<

scanner.c: scanner.l
	flex -o $@ $(FLEX_FLAGS) $<

clean:
	rm -rf $(OBJ) main.dSYM