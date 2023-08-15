CC=gcc
CFLAGS=-Wall
SOURCE=reverse_lib

all: run_test

b1:
	$(CC) $(CFLAGS) $(SOURCE).c -o $(SOURCE)_1.exe -D BUFF_BLOCK_SIZE=1

b1024:
	$(CC) $(CFLAGS) $(SOURCE).c -o $(SOURCE)_1024.exe -D BUFF_BLOCK_SIZE=1024

run_test: b1 b1024 test

test:
	echo "BUFF BLOCK SIZE = 1:" > pomiar_zad_2.txt
	./reverse_lib_1.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null
	./reverse_lib_1.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null
	./reverse_lib_1.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null
	./reverse_lib_1.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null
	./reverse_lib_1.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null

	echo "" >> pomiar_zad_2.txt
	echo "BUFF BLOCK SIZE = 1024:" >> pomiar_zad_2.txt
	./reverse_lib_1024.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null
	./reverse_lib_1024.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null
	./reverse_lib_1024.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null
	./reverse_lib_1024.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null
	./reverse_lib_1024.exe /etc/passwd result.in >> pomiar_zad_2.txt 2>/dev/null


compare: lib sys
	./replace_lib.exe / ? /etc/passwd result_replace.in ; tr / ? < /etc/passwd > result_tr.in ; diff -s result_tr.in result_replace.in


clean:
	rm -rf *.exe *.in result.txt