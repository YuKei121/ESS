CC = gcc

TARGETS = taxi

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS)