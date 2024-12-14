/*
    This file will test the library functions in cli_funcs.h
    For the actual command line interface testing, see test_cli.run

    locker_status()
    - tested from command line because we're trying to compare stdout

*/

#include "cli_funcs.h"
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#define RED_TEXT ("\e[31m")
#define GREEN_TEXT ("\e[32m")
#define YELLOW_TEXT ("\e[33m")
#define END_FMT ("\e[0m\n")

static int failures = 0;

#define ERRDUMP(STRX)                                                                                                                      \
    do {                                                                                                                                   \
        std::cout << RED_TEXT << (STRX) << END_FMT;                                                                                        \
        failures++;                                                                                                                        \
    } while ( 0 );

int main(int argc, char* argv[]) {

    if ( argc != 6 ) {
        ERRDUMP("Invalid arguments to test_cli_lib.cpp");
        return failures;
    }

    string valid_file = argv[1];
    string valid_dir = argv[2];
    valid_dir += "/";
    string lockername = argv[3];
    string valid_empty_dir = argv[4];
    string alt_dir = argv[5];
    string invalid_file = "invalid.txt";
    string invalid_dir = "invaliddir";

    // CLI_error()
    bool caught = false;
    try {
        CLI_error();
    } catch ( string err ) {
        caught = true;
    }
    if ( !caught ) {
        ERRDUMP("Failed to throw error when explicitly invoked");
        failures++;
    }

    // delete before locker even created
    caught = false;
    try {
        delete_file(lockername, valid_dir + valid_file);
    } catch ( string err ) {
        caught = true;
    }
    if ( !caught ) {
        ERRDUMP("Failed to throw an error calling --delete on nonexistent locker");
    }

    // add_file()
    try {
        add_file(lockername, valid_dir + valid_file);
    } catch ( string err ) {
        string ln = std::to_string(__LINE__);
        ERRDUMP(ln + ": Failed to add a valid file: " + err);
    }

    caught = false;
    try {
        add_file(lockername, invalid_file);
    } catch ( string err ) {
        caught = true;
    }
    if ( !caught ) {
        ERRDUMP("Failed to throw an error when adding invalid file");
    }

    caught = false;
    try {
        add_file(lockername, valid_dir + valid_file);
    } catch ( string err ) {
        caught = true;
    }
    if ( !caught ) {
        ERRDUMP("Failed to throw an error when adding exact same file with same contents");
    }

    try {
        add_file(lockername, valid_empty_dir);
    } catch ( string err ) {
        string ln = std::to_string(__LINE__);
        ERRDUMP(ln + ": Failed to add an empty directory: " + err);
    }

    caught = false;
    try {
        add_file(lockername, invalid_dir);
    } catch ( string err ) {
        caught = true;
    }
    if ( !caught ) {
        ERRDUMP("Failed to throw error when adding invalid directory");
    }

    // fetch_to_file()
    try {
        fetch_to_file(lockername, alt_dir + ".txt", valid_file);
    } catch ( string err ) {
        string ln = std::to_string(__LINE__);
        ERRDUMP(ln + ": Failed to fetch a valid file: " + err);
    }

    caught = false;
    try {
        fetch_to_file(lockername, "invalid_" + alt_dir + ".txt", invalid_file);
    } catch ( string err ) {
        caught = true;
    }
    if ( !caught ) {
        ERRDUMP("Failed to throw error when fetching a nonexistent file");
    }

    // delete_file()
    try {
        delete_file(lockername, valid_dir + valid_file);
    } catch ( string err ) {
        ERRDUMP("Failed to delete a stored file, err: " + err);
    }

    try {
        delete_file(lockername, valid_dir);
    } catch ( string err ) {
        ERRDUMP("Failed to delete a validly stored directory");
    }

	caught = false;
	try {
		delete_file("nonexistent_locker", valid_dir);
	}
	catch (string err) {
		caught = true;
	}
	if (!caught) {
		ERRDUMP("Failed to notify user they tried to delete from an invalid locker");
	}

	caught = false;
	try {
		delete_file(lockername, invalid_file);
	}
	catch (string err) {
		caught = true;
	}
	if (!caught) {
		ERRDUMP("Failed to notify user they tried to delete a file that was not yet added");
	}

    // fetch after delete
    caught = false;
    try {
        fetch_to_file(lockername, alt_dir + ".txt", valid_dir + valid_file);
    } catch ( string err ) {
        caught = true;
    }
    if ( !caught ) {
        ERRDUMP("Failed to throw an error deleting a previously deleted file");
    }

    // add_file_save_as()
    try {
        add_file_save_as(lockername, valid_dir, alt_dir);
    } catch ( string err ) {
        string ln = std::to_string(__LINE__);
        ERRDUMP(ln + ": Failed to add a directory (save as variant): " + err);
    }

    caught = false;
    try {
        add_file_save_as(lockername, invalid_file, alt_dir);
    } catch ( string err ) {
        caught = true;
    }
    if ( !caught ) {
        ERRDUMP("Failed to throw an error when adding invalid file (save as variant)");
    }

    // fetch from save-as file
    try {
        fetch_to_file(lockername, alt_dir + "/result.txt", alt_dir + "/" + valid_file);
    } catch ( string err ) {
        string ln = std::to_string(__LINE__);
        ERRDUMP(ln + ": Failed to fetch a valid file (save as variant)" + err);
    }
    try {
        fetch_to_file(lockername, alt_dir + "-b/", alt_dir + "/");
    } catch ( string err ) {
        ERRDUMP("Failed to retrieve an entire directory");
    }

    if ( failures > 0 ) {
        std::cout << RED_TEXT << "CLI: Failed " << failures << " subtests." << END_FMT << std::endl;
        return failures;
    }

    std::cout << GREEN_TEXT << "All subtests pass." << END_FMT << std::endl;
    return 0;
}
