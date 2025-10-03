CC = gcc

SERVER_TARGET = serv
CLIENT_TARGETS = client1 client2 client3
ALL_TARGETS = $(SERVER_TARGET) $(CLIENT_TARGETS)

.PHONY: all clean

all: $(ALL_TARGETS)

%: %.c
	$(CC) -o $@ $<

clean:
	rm -f $(ALL_TARGETS)