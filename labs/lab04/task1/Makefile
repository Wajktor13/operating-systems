CC=gcc
CFLAGS=-Wall
SOURCE=main
RAPORT=raport.2

all: compile test

compile:
	$(CC) $(CFLAGS) $(SOURCE)_fork.c -o $(SOURCE)_fork.exe
	$(CC) $(CFLAGS) $(SOURCE)_exec.c -o $(SOURCE)_exec.exe

test:
	echo "fork:" > $(RAPORT).txt
	echo "\tignore:" >> $(RAPORT).txt
	./$(SOURCE)_fork.exe ignore >> $(RAPORT).txt
	echo "" >> $(RAPORT).txt
	echo "\thandler:" >> $(RAPORT).txt
	./$(SOURCE)_fork.exe handler >> $(RAPORT).txt
	echo "" >> $(RAPORT).txt
	echo "\tmask:" >> $(RAPORT).txt
	./$(SOURCE)_fork.exe mask >> $(RAPORT).txt
	echo "" >> $(RAPORT).txt
	echo "\tpending:" >> $(RAPORT).txt
	./$(SOURCE)_fork.exe pending >> $(RAPORT).txt
	echo "" >> $(RAPORT).txt

	echo "exec:" >> $(RAPORT).txt
	echo "\tignore:" >> $(RAPORT).txt
	./$(SOURCE)_exec.exe ignore p >> $(RAPORT).txt
	echo "" >> $(RAPORT).txt
	echo "\tmask:" >> $(RAPORT).txt
	./$(SOURCE)_exec.exe mask p >> $(RAPORT).txt
	echo "" >> $(RAPORT).txt
	echo "\tpending:" >> $(RAPORT).txt
	./$(SOURCE)_exec.exe pending p >> $(RAPORT).txt
	echo "" >> $(RAPORT).txt
	echo "Wnioski: Dla fork dziedziczone jest ignorowanie, przypisany handler i maska. Sygnal oczekujacy nie jest widoczny w dziecku.\n\
	Dla exec dziedziczone jest ignorowanie, maska oraz oczekujacy sygnal." >> $(RAPORT).txt

clean:
	rm -rf *.exe *.in $(RAPORT).txt