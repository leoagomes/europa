LINENOISE_SOURCE=../lib/linenoise.c
LIB_INCLUDE=../lib/include
PROJ_INCLUDE=../include

EXECUTABLE=repl
LIBRARY=../build/europa.a
SOURCES=$(wildcard *.c)

CC?=gcc

CFLAGS=-I$(LIB_INCLUDE) -I$(PROJ_INCLUDE)

all: clean $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(LIBRARY)
	$(CC) -o $@ $^ $(LINENOISE_SOURCE) $(LIBRARY) $(CFLAGS) $(C_FLAGS)

%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS)

clean:
	(cd .. && make clean)
	rm -rf $(EXECUTABLE)

debug: CFLAGS+=-g
debug: all

$(LIBRARY): # this is library
	(cd .. && make debug)
