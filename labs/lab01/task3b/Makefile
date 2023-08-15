SHELL:=/bin/bash
CC=gcc
CFLAGS=-Wall
LIB=libbm
REPL=repl.c

all:
	make exe_static
	make exe_shared
	make exe_dynamic

opt_o0:
	make all OPT_FLAG="-O0"

opt_o2:
	make all OPT_FLAG="-O2"

opt_os:
	make all OPT_FLAG="-Os"

run_tests: 
	make clean
	make write_test
	
	make opt_o0
	make test NAME="o0"
	make clean

	make opt_o2
	make test NAME="o2"
	make clean

	make opt_os
	make test NAME="os"
	make clean

test:
	./repl_static.exe < test.in > results/results_static_$(NAME).txt 2>/dev/null
	./repl_shared.exe < test.in > results/results_shared_$(NAME).txt 2>/dev/null
	./repl_dynamic.exe < test.in > results/results_dynamic_$(NAME).txt 2>/dev/null

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

lib_static: $(LIB).c
	$(CC) $(CFLAGS) $(OPT_FLAG) -c $(LIB).c
	ar rcs $(LIB).a $(LIB).o

lib_shared: $(LIB).c
	$(CC) $(CFLAGS) $(OPT_FLAG) -c -fPIC $(LIB).c
	$(CC) $(CFLAGS) $(OPT_FLAG) -shared -o $(LIB).so $(LIB).o

exe_static: lib_static
	$(CC) $(CFLAGS) $(OPT_FLAG) $(REPL) -o repl_static.exe -L . $(LIB).a

exe_shared: lib_shared
	$(CC) $(CFLAGS) $(OPT_FLAG) $(REPL) -o repl_shared.exe -L . $(LIB).so
	make move_shared_lib

exe_dynamic: lib_shared
	$(CC) $(CFLAGS) $(OPT_FLAG) $(REPL) -o repl_dynamic.exe -D USE_DLL

move_shared_lib:
	sudo mv -f $(LIB).so /usr/lib

remove_shared_lib:
	sudo rm -f /usr/lib/$(LIB).so

clean:
	rm -rf *.o *.a *.so *.exe
	make remove_shared_lib
