CC=gcc
LIB=libbm
CFLAGS=-Wall

all:
	make lib_static
	make lib_shared

lib_static: $(LIB).c
	$(CC) $(CFLAGS) -c $(LIB).c
	ar rcs $(LIB).a $(LIB).o

lib_shared: $(LIB).c
	$(CC) $(CFLAGS) -c -fPIC $(LIB).c
	$(CC) $(CFLAGS) -shared -o $(LIB).so $(LIB).o

clean:
	rm -f *.o *.a *.so
	