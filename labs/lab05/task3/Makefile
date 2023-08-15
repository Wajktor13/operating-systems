CC=gcc
CFLAGS=-Wall
SOURCE=main

all: compile raport

compile:
	$(CC) $(CFLAGS) $(SOURCE).c -o $(SOURCE)
	$(CC) $(CFLAGS) sgc.c -o sgc

raport:
	echo "dx: 0.000000001, n: 5" > raport.txt
	./$(SOURCE) 0.000000001 10 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.000000001, n: 10" >> raport.txt
	./$(SOURCE) 0.000000001 100 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.000000001, n: 100" >> raport.txt
	./$(SOURCE) 0.000000001 1000 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.000000001, n: 500" >> raport.txt
	./$(SOURCE) 0.000000001 500 >> raport.txt
	echo "" >> raport.txt
	
	echo "dx: 0.0000000008, n: 5" >> raport.txt
	./$(SOURCE) 0.0000000008 10 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.0000000008, n: 10" >> raport.txt
	./$(SOURCE) 0.0000000008 100 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.0000000008, n: 100" >> raport.txt
	./$(SOURCE) 0.0000000008 1000 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.0000000008, n: 500" >> raport.txt
	./$(SOURCE) 0.0000000008 500 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.0000000002, n: 5" >> raport.txt
	./$(SOURCE) 0.0000000002 10 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.0000000002, n: 10" >> raport.txt
	./$(SOURCE) 0.0000000002 100 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.0000000002, n: 100" >> raport.txt
	./$(SOURCE) 0.0000000002 1000 >> raport.txt
	echo "" >> raport.txt

	echo "dx: 0.0000000002, n: 500" >> raport.txt
	./$(SOURCE) 0.0000000002 500 >> raport.txt
	echo "" >> raport.txt

clean:
	rm -rf *.exe *.in main *.txt sgc
