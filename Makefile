INCLUDE_DIR=include
LIB_INCLUDE=lib/include
BUILD_DIR=build
SRC_DIR=src

CC?=gcc
AR?=ar

SO_FILE=europa.so
A_FILE=europa.a
EXECUTABLE=europa

CFLAGS=-I$(INCLUDE_DIR) -I$(LIB_INCLUDE) -fpic -pedantic

OBJECTS=$(patsubst src/%.c,%.o,$(wildcard src/*.c))

all: clean $(BUILD_DIR)/$(SO_FILE) $(BUILD_DIR)/$(A_FILE)# $(BUILD_DIR)/$(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR)/$(SO_FILE) $(BUILD_DIR)/$(A_FILE) $(OBJECTS:%=$(BUILD_DIR)/%)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $^ $(CFLAGS)

$(BUILD_DIR)/$(A_FILE): $(OBJECTS:%=$(BUILD_DIR)/%)
	$(AR) rvs $@ $^

$(BUILD_DIR)/$(SO_FILE): $(OBJECTS:%=$(BUILD_DIR)/%)
	$(CC) -shared -o $@ $^

debug: CFLAGS+=-g
debug: all