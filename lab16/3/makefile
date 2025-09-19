CC = gcc
TARGETS = multicast/serv multicast/client \
          broadcast/serv broadcast/client

.PHONY: all clean

all: $(TARGETS)

multicast/serv: multicast/serv.c
	$(CC) -o $@ $<

multicast/client: multicast/client.c
	$(CC) -o $@ $<

broadcast/serv: broadcast/serv.c
	$(CC) -o $@ $<

broadcast/client: broadcast/client.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS)