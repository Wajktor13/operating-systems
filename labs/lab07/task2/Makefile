CC=gcc
CFLAGS=-Wall

all: compile

compile:
	$(CC) $(CFLAGS) main.c -o main
	$(CC) $(CFLAGS) barber.c -o barber
	$(CC) $(CFLAGS) customer.c -o customer

clean:
	rm -f main barber customer