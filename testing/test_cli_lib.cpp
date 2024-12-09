/*
	This file will test the library functions in cli_funcs.h
	For the actual command line interface testing, see test_cli.run
*/

#include "cli_funcs.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#define RED_TEXT ("\e[31m")
#define END_FMT ("\e[0m\n")

int failures = 0;

#define ERRDUMP(STRX) do { \
	std::cout << RED_TEXT << (STRX) << END_FMT; \
	failures++; \
} while(0);

int main(int argc, char* argv[]) {

	if (argc != 5) {
		ERRDUMP("Invalid arguments to test_cli_lib.cpp");
		return failures;
	}

	// 1. CLI_error()
	bool caught = false;
	try {
		CLI_error();
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw error when explicitly invoked");
		failures++;
	}

	// 2. test_path()
	string invalid_file = "invalid.txt";
	string valid_file = argv[1];

	caught = false;
	try {
		test_path(invalid_file);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw error for invalid file");
	}
	try {
		test_path(valid_file);
	}
	catch (string err) {
		ERRDUMP("Failed to open a valid file");
	}

	// 3. test_dir()
	string invalid_dir = "invaliddir";
	string valid_dir = argv[2];

	caught = false;
	try {
		test_dir(invalid_dir);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw error for invalid directory");
	}
	try {
		test_dir(valid_dir);
	}
	catch (string err) {
		ERRDUMP("Failed to open a valid directory");
	}

	// 4. string into_dd()
	try {
		string into_dd_result = into_dd("alpha", "beta");
		if (into_dd_result != "alpha/beta") {
			ERRDUMP("Incorrectly formed locker name");
		}
	}
	catch (string err) {
		ERRDUMP(err);
	}
	caught = false;
	try {
		string empty = "";
		string whitespace = "\n\t\r";
		string into_dd_result = into_dd(empty, whitespace);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("into_dd() doesn't check for whitespace or empty strings");
	}

	// 5. locker_status()
	// todo

	// 6. add_file()
	string lockername = argv[3];
	try {
		DDStore store(lockername);
		store.add_file(valid_file);
	}
	catch (string err) {
		ERRDUMP(err);
	}
	caught = false;
	try {
		DDStore store(lockername);
		store.add_file(invalid_file);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw an error when adding invalid file");
	}

	string valid_empty_dir = argv[4];
	try {
		DDStore store(lockername);
		store.add(valid_empty_dir);
	}
	catch (string err) {
		ERRDUMP(err);
	}
	try {
		DDStore store(lockername);
		store.add(valid_dir);
	}
	catch (string err) {
		ERRDUMP(err);
	}
	caught = false;
	try {
		DDStore store(lcokername);
		store.add(invalid_dir);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw error when adding invalid directory");
	}

	// same file added a second time is handled silently?
	try {
		DDStore store(lockername);
		store.add(valid_file);
	}
	catch (string err) {
		ERRDUMP(err);
	}

	// 7. add_file_change_name
	// todo

	// 8. rename_file()
	

	return failures;
}
