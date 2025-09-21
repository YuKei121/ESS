CC = gcc
CLIENT_LIBS = -pthread -lncurses

all: serv client

serv: serv.c
	$(CC) -o $@ $<

client: client.c
	$(CC) -o $@ $< $(CLIENT_LIBS)

clean:
	rm -f serv client

.PHONY: all clean