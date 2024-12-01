CC = g++
CXX = g++

COMMON_CFLAGS = -std=c++20
DEBUG_FLAGS = $(COMMON_CFLAGS) -g -Og
RELEASE_FLAGS = $(COMMON_CFLAGS) -O3 -Wall -Werror

SOURCE_DIR = src
BUILD_DIR = build
TEST_DIR = testing

SRCFILES := $(wildcard $(SOURCE_DIR)/*.cpp)
TESTFILES := $(wildcard $(TEST_DIR)/*.cpp)
TESTBINARIES := $(TESTFILES:.cpp=.test)

# Default Target: Debug via all
all: debug

# Rules
debug: $(SRCFILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(DEBUG_FLAGS) -o $(BUILD_DIR)/$@ $^

release: $(SRCFILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(RELEASE_FLAGS) -o $(BUILD_DIR)/$@ $^

%.test: %.cpp
	@mkdir -p $(TEST_DIR)
	$(CC) $(DEBUG_FLAGS) -I src/ -o $@ $<

# Utility

.PHONY: clean rebuild test testclean
clean:
	rm -rdf $(BUILD_DIR)

rebuild:
	make clean
	make all

testclean:
	@echo Caution: DO NOT remove .run files.
	@echo Caution: Generated files have the .test extension
	rm -rd $(TESTBINARIES)

test:
	@echo "Building tests"
	@make --no-print-directory $(TESTBINARIES)
	@echo "Running tests"
	$(TEST_DIR)/run_tests.sh
