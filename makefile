CC = g++
CXX = g++

COMMON_CFLAGS = -std=c++20 -Wall -Werror
DEBUG_FLAGS = $(COMMON_CFLAGS) -g -Og
RELEASE_FLAGS = $(COMMON_CFLAGS) -O3

SOURCE_DIR = src
BUILD_DIR = build

SRCFILES := $(wildcard $(SOURCE_DIR)/*.cpp)

# Default Target: Debug via all
all: debug

# Rules
debug: $(SRCFILES)
	$(CC) $(DEBUG_FLAGS) -o $(BUILD_DIR)/$@ $^

release: $(SRCFILES)
	$(CC) $(RELEASE_FLAGS) -o $(BUILD_DIR)/$@ $^

$(BUILD_DIR):
	@mkdir -p $@


# Utility

clean:
	rm -rdf $(BUILD_DIR)

rebuild:
	make clean
	make all
