CC=gcc
CFLAGS=-Wall

all: compile

compile:
	touch server_logs.txt
	echo "" > server_logs.txt
	$(CC) $(CFLAGS) client.c -o client
	$(CC) $(CFLAGS) server.c -o server

clean:
	rm -rf *.exe *.in client server server_logs.txt