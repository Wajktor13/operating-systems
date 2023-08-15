CC=gcc
CFLAGS=-Wall

all: catcher sender

catcher:
	$(CC) $(CFLAGS) catcher.c -o catcher.exe

sender:
	$(CC) $(CFLAGS) sender.c -o sender.exe

clean:
	rm -rf *.exe *.in
