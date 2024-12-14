CC = g++
CXX = g++

COMMON_CFLAGS = -std=c++20
DEBUG_FLAGS = $(COMMON_CFLAGS) -g -Og -Wall
RELEASE_FLAGS = $(COMMON_CFLAGS) -O3 -Wall #-Werror

SOURCE_DIR = src
BUILD_DIR = build
TEST_DIR = testing

SRCFILES := src/ddstore.cpp
TESTFILES := $(wildcard $(TEST_DIR)/*.cpp)
TESTBINARIES := $(TESTFILES:.cpp=.test)

# Adding separate source files for De Duplicator server implementation

# Server files
SERVER_SRC = $(SOURCE_DIR)/server_main.cpp $(SRCFILES)

# Default Target: Debug via all
all: debug

# Rules
debug: $(SRCFILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(DEBUG_FLAGS) -o $(BUILD_DIR)/$@ src/debug_main.cpp $^

cli_debug: $(SRCFILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(DEBUG_FLAGS) -o $(BUILD_DIR)/$@ src/cli_main.cpp $^

%.test: %.cpp
	@mkdir -p $(TEST_DIR)
	$(CC) $(DEBUG_FLAGS) -I src/ -o $@ $(SRCFILES) $<

.PHONY: network release

network: $(CLIENT_SRC) $(SERVER_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(DEBUG_FLAGS) -o $(BUILD_DIR)/server_debug $(SERVER_SRC)

release: $(SRCFILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(RELEASE_FLAGS) -o $(BUILD_DIR)/ddstore src/cli_main.cpp $^
	$(CC) $(RELEASE_FLAGS) -o $(BUILD_DIR)/server $(SERVER_SRC)


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
	-@rm $(TESTBINARIES)
	@make --no-print-directory $(TESTBINARIES)
	@$(TEST_DIR)/run_tests.sh
