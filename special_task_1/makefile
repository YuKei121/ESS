CC = gcc

SERVER_TARGET = echo-serv
CLIENT_TARGETS = echo-client
ALL_TARGETS = $(SERVER_TARGET) $(CLIENT_TARGETS)

.PHONY: all clean

all: $(ALL_TARGETS)

%: %.c
	$(CC) -o $@ $<

clean:
	rm -f $(ALL_TARGETS)