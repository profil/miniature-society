CC = gcc
CFLAGS=-Wall -pedantic
LDFLAGS=
SRC=util.c
OBJ=$(SRC:.c=.o)

all: client master-server

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

client: $(OBJ) client.o
	$(CC) -o $@ $? $(LDFLAGS)

master-server: $(OBJ) master-server.o
	$(CC) -o $@ $? $(LDFLAGS)

clean:
	rm -f $(OBJ) master-server.o client.o master-server client
