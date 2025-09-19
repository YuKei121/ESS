CC = gcc
TARGETS = local-degram/serv local-degram/client \
          local-stream/serv local-stream/client \
          inet-degram/serv inet-degram/client \
          inet-stream/serv inet-stream/client

.PHONY: all clean

all: $(TARGETS)

local-degram/serv: local-degram/serv.c
	$(CC) -o $@ $<

local-degram/client: local-degram/client.c
	$(CC) -o $@ $<

local-stream/serv: local-stream/serv.c
	$(CC) -o $@ $<

local-stream/client: local-stream/client.c
	$(CC) -o $@ $<

inet-degram/serv: inet-degram/serv.c
	$(CC) -o $@ $<

inet-degram/client: inet-degram/client.c
	$(CC) -o $@ $<

inet-stream/serv: inet-stream/serv.c
	$(CC) -o $@ $<

inet-stream/client: inet-stream/client.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS)