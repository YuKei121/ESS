BIN := test
OBJ := main.o abonent_list.o
SRC := main.c abonent_list.c
HDR := abonent_list.h

CC := gcc
CFLAGS := -g
LDFLAGS := -lm

.PHONY : clean

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)
	
%.o: %.c
	$(CC) $(CFLAGS) $^ -c
	
clean:
	rm -rf $(BIN) $(OBJ)

$(BIN): $(HDR)
