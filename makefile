CC = g++
CXX = g++

COMMON_CFLAGS = -std=c++20
DEBUG_FLAGS = $(COMMON_CFLAGS) -g -Og
RELEASE_FLAGS = $(COMMON_CFLAGS) -O3 -Wall -Werror

SOURCE_DIR = src
BUILD_DIR = build

SRCFILES := $(wildcard $(SOURCE_DIR)/*.cpp)

# Default Target: Debug via all
all: debug

# Rules
debug: $(SRCFILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(DEBUG_FLAGS) -o $(BUILD_DIR)/$@ $^

release: $(SRCFILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(RELEASE_FLAGS) -o $(BUILD_DIR)/$@ $^

# Utility

.PHONY: clean rebuild
clean:
	rm -rdf $(BUILD_DIR)

rebuild:
	make clean
	make all
