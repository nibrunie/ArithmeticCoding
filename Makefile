
CFLAGS += -Wall -Werror --pedantic -O3 -g -Ilib

test_basic: lib/arith_coding.o test/test_basic.o
	$(CC) $(CFLAGS) -o $@ $^

lib: lib/arith_coding.o
	$(AR) rcs libarithcoding.a lib/arith_coding.o

test: test_basic
	./test_basic

encoder: lib/arith_coding.o util/encoder.o
	$(CC) $(CFLAGS) -o $@ $^

doc:
	doxygen

clean:
	rm -f lib/*.o util/*.o ./test_basic ./encoder

.PHONY: test lib doc
