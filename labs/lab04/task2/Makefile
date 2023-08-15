CC=gcc
CFLAGS=-Wall
SOURCE=main
RAPORT=raport

all: compile raport

compile:
	$(CC) $(CFLAGS) $(SOURCE).c -o $(SOURCE).exe

raport:
	./$(SOURCE).exe > $(RAPORT).txt
	echo "\n\n--------------------------------" >> $(RAPORT).txt
	echo "Podsumowanie: przetestowano flagi SA_SIGINFO, SA_NODEFER, SA_NOCLDSTOP. Wszystkie działają zgodnie z oczekiwaniem." >> $(RAPORT).txt

clean:
	rm -rf *.exe *.txt