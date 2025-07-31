CC := gcc
CFLAGS := -g
LDFLAGS := -L. -lcalc

all: main

libcalc.a: add.o sub.o mul.o div.o
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $^ -c

main: main.c libcalc.a
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o *.a main