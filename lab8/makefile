CC := gcc
CFLAGS := -g -fPIC
LDFLAGS := -L. -lcalc

all: main

libcalc.so: add.o sub.o mul.o div.o
	$(CC) -shared -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $^ -c

main: main.c libcalc.so
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o *.a main