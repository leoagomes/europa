MUNIT_SOURCE=../lib/munit.c
LIB_INCLUDE=../lib/include
PROJ_INCLUDE=../include

EXECUTABLE=test
LIBRARY=../build/europa.a
SOURCES=main.c
OBJECTS=gc.o read.o helpers.o table.o

CC?=gcc

CFLAGS=-I$(LIB_INCLUDE) -I$(PROJ_INCLUDE)

all: clean $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(OBJECTS) $(LIBRARY)
	$(CC) -o $@ $^ $(MUNIT_SOURCE) $(LIBRARY) $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS)

clean:
	(cd .. && make clean)
	rm -rf $(EXECUTABLE) $(OBJECTS)

debug: CFLAGS+=-g
debug: all

$(LIBRARY): # this is library
	(cd .. && make debug)