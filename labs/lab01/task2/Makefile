SHELL:=/bin/bash
CC=gcc
CFLAGS=-Wall
LIB=libbm
REPL=repl.c

all:
	make exe_static
	make exe_shared
	make exe_dynamic

lib_static: $(LIB).c
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
	rm -rf *.o *.a *.so *.exe test.in *.txt
	make remove_shared_lib
