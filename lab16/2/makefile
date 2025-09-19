CC = gcc
LDFLAGS = -lpthread

SERVER_TARGETS = serv1-multiproc serv2-forkedPool serv3-factory serv4-multiprotocol
CLIENT_TARGET = client
ALL_TARGETS = $(SERVER_TARGETS) $(CLIENT_TARGET)

.PHONY: all clean

all: $(ALL_TARGETS)

serv1-multiproc: serv1-multiproc.c
	$(CC) -o $@ $<

serv2-forkedPool: serv2-forkedPool.c
	$(CC) -o $@ $<

serv3-factory: serv3-factory.c
	$(CC) -o $@ $< $(LDFLAGS)

serv4-multiprotocol: serv4-multiprotocol.c
	$(CC) -o $@ $<

client: client.c
	$(CC) -o $@ $<

clean:
	rm -f $(ALL_TARGETS)