CC = gcc
CLIENT_LIBS = -pthread -lncurses

all: serv client

serv: serv.c
	$(CC) $(CFLAGS) -o $@ $<

client: client.c
	$(CC) $(CFLAGS) -o $@ $< $(CLIENT_LIBS)

clean:
	rm -f serv client

.PHONY: all clean