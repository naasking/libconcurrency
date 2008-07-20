LIBNAME=libconcurrency

# library name and version
SONAME=$(LIBNAME).so.0.8

# compiler and linker flags
#CFLAGS=-W -Wall -c -g -O2 -fPIC -fomit-frame-pointer -I./
CFLAGS=-W -fno-stack-protector -ansi -pedantic -O2 -fPIC -I./
LFLAGS=-shared -Wl,-soname,$(SONAME) -o $(SONAME) $<

# compiler
CC=gcc

default: libconcurrency/coro.o tstcnry.o
	$(CC) $(LFLAGS)
	$(CC) $(LIBNAME)/coro.o tstcnry.c -I./ -ansi -pedantic -o test

clean:
	rm $(LIBNAME)/*.o *.so.*

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
