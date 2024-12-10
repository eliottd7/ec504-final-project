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
#define GREEN_TEXT ("\e[32m")
#define YELLOW_TEXT ("\e[33m")
#define END_FMT ("\e[0m\n")


static int failures = 0;

#define ERRDUMP(STRX) do { \
	std::cout << RED_TEXT << (STRX) << END_FMT; \
	failures++; \
} while(0);

int main(int argc, char* argv[]) {

	if (argc != 5) {
		ERRDUMP("Invalid arguments to test_cli_lib.cpp");
		return failures;
	}

	string valid_file = argv[1];
	string valid_dir = argv[2];
	string lockername = argv[3];
	string valid_empty_dir = argv[4];
	string invalid_file = "invalid.txt";
	string invalid_dir = "invaliddir";

	// CLI_error()
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

	// test_path()
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
		ERRDUMP("Failed test on a valid file");
	}

	// test_dir()
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

	// string into_dd()
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

	// locker_status()
	// todo

	// add_file()
	try {
		add_file(lockername, valid_file);
	}
	catch (string err) {
		ERRDUMP(err);
	}
	caught = false;
	try {
		add_file(lockername, invalid_file);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw an error when adding invalid file");
	}

	try {
		add_file(lockername, valid_empty_dir);
	}
	catch (string err) {
		ERRDUMP(err);
	}
	try {
		add_file(lockername, valid_dir);
	}
	catch (string err) {
		ERRDUMP(err);
	}
	caught = false;
	try {
		add_file(lockername, invalid_dir);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw error when adding invalid directory");
	}

	caught = false;
	try {
		add_file(lockername, valid_file);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw an error when adding exact same file with same contents");
	}

	// add_file_change_name
	// todo

	// rename_file()
	// todo

	// retreive_to_console
	try {
		retrieve_to_console(lockername, valid_file);
	}
	catch (string err) {
		ERRDUMP("Failed to delete a validly stored file");
	}
	caught = false;
	try {
		retrieve_to_console(lockername, invalid_file);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw an error when retrieving an invalid file");
	}
	caught = false;
	try {
		retrieve_to_console(lockername, valid_dir);
		retrieve_to_console(lockername, invalid_dir);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw an error when retrieving a directory");
	}

	// retrieve_to_file()
	// todo

	// delete_file(lockername, )
	try {
		delete_file(lockername, valid_file);
	}
	catch (string err) {
		ERRDUMP("Failed to delete a validly stored file");
	}
	caught = false;
	try {
		delete_file(lockername, invalid_file);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw an error when deleting an invalid file");
	}

	try {
		delete_file(lockername, valid_dir);
	}
	catch (string err) {
		ERRDUMP("Failed to delete a validly stored directory");
	}
	caught = false;
	try {
		delete_file(lockername, invalid_dir);
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		ERRDUMP("Failed to throw an error when deleting an invalid directory");
	}

	if (failures > 0) {
		std::cout << RED_TEXT << "Failed " << failures << " subtests." << END_FMT << std::endl;
		return 1;
	}

	std::cout << GREEN_TEXT << "All subtests pass." << END_FMT << std::endl;
	return 0;
}
