CC = gcc
TARGETS = posix/serv posix/client \
          systemv/serv systemv/client

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS)