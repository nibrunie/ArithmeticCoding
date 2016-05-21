
CFLAGS += -Ilib

test_basic: lib/arith_coding.o test/test_basic.o
	$(CC) $(CFLAGS) -o $@ $^

test: test_basic
	./test_basic

.PHONY: test
