CC=gcc
CFLAGS=-Wall
SOURCE=replace

all: run_test

lib:
	$(CC) $(CFLAGS) $(SOURCE)_lib.c -o $(SOURCE)_lib.exe

sys:
	$(CC) $(CFLAGS) $(SOURCE)_sys.c -o $(SOURCE)_sys.exe

run_test: lib sys test

test:
	echo "LIB:" > pomiar_zad_1.txt
	./replace_lib.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null
	./replace_lib.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null
	./replace_lib.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null
	./replace_lib.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null
	./replace_lib.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null

	echo "" >> pomiar_zad_1.txt
	echo "SYS:" >> pomiar_zad_1.txt
	./replace_sys.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null
	./replace_sys.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null
	./replace_sys.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null
	./replace_sys.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null
	./replace_sys.exe / ? /etc/passwd result.in >> pomiar_zad_1.txt 2>/dev/null

compare: lib sys
	./replace_lib.exe / ? /etc/passwd result_replace.in ; tr / ? < /etc/passwd > result_tr.in ; diff -s result_tr.in result_replace.in


clean:
	rm -rf *.exe *.in result.txt