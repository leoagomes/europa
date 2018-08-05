MUNIT_SOURCE=../lib/munit.c
LIB_INCLUDE=../lib/include
PROJ_INCLUDE=../include

EXECUTABLE=test
LIBRARY=../lib/europa.a
SOURCES=main.c
OBJECTS=gc.o

CC?=gcc

CFLAGS=-I$(LIB_INCLUDE) -I$(PROJ_INCLUDE)

all: clean $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(OBJECTS)
	$(CC) -o $@ $(MUNIT_SOURCE) $(LIBRARY) $^ $(CFLAGS)

gc.o: gc.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(EXECUTABLE)