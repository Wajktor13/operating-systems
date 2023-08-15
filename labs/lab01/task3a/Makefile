SHELL:=/bin/bash
CC=gcc
CFLAGS=-Wall
LIB=libbm
REPL=repl.c

all:
	make exe_static
	make exe_shared
	make exe_dynamic

run_tests: 
	make all 
	make test
	make clean

test:
	make write_test
	./repl_static.exe < test.in > results/results_static.txt 2>/dev/null
	./repl_shared.exe < test.in > results/results_shared.txt 2>/dev/null
	./repl_dynamic.exe < test.in > results/results_dynamic.txt 2>/dev/null

write_test:
	echo "init 1024" > test.in
	(ls -p | grep -v / | sed 's/^/count /') >> test.in
	echo "show 0" >> test.in
	echo "show 1" >> test.in
	echo "show 2" >> test.in
	echo "show 3" >> test.in
	echo "show 4" >> test.in
	echo "show 5" >> test.in
	echo "delete 1" >> test.in
	echo "delete 2" >> test.in
	echo "delete 3" >> test.in
	echo "delete 4" >> test.in
	echo "delete 5" >> test.in
	echo "destroy" >> test.in 
	echo "exit" >> test.in

clean_test:
	rm results/*

lib_static:
	$(CC) $(CFLAGS) -c $(LIB).c
	ar rcs $(LIB).a $(LIB).o

lib_shared: $(LIB).c
	$(CC) $(CFLAGS) -c -fPIC $(LIB).c
	$(CC) $(CFLAGS) -shared -o $(LIB).so $(LIB).o

exe_static: lib_static
	$(CC) $(CFLAGS) $(REPL) -o repl_static.exe -L . $(LIB).a

exe_shared: lib_shared
	$(CC) $(CFLAGS) $(REPL) -o repl_shared.exe -L . $(LIB).so
	make move_shared_lib

exe_dynamic: lib_shared
	$(CC) $(CFLAGS) $(REPL) -o repl_dynamic.exe -D USE_DLL

move_shared_lib:
	sudo mv -f $(LIB).so /usr/lib

remove_shared_lib:
	sudo rm -f /usr/lib/$(LIB).so

clean:
	rm -rf *.o *.a *.so *.exe test.in .in
	make remove_shared_lib
