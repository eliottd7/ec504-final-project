#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ddstore.h"

using namespace std;

/*
Accepted flags:
-locker "path/to/locker"
  prints to the command_binary line: Contents: N files, XX.X KB/MB/GB used, {list of names of files}
-add "path/to/filename" (in addition to -locker)
  adds file to locker, also prints to the command_binary line locker data
-rename "name" (in addition to -locker)
  choses a file stored in the locker to be renamed
-new-name "name" (in addition to -add or -rename)
  changes the name of the file to that of how it will be stored in the locker
-delete "name" (in addition to -locker)
  removes the file from the locker, also prints to the command_binary line locker data
-fetch "name" (in addition to -locker)
  prints to the console the contents of the file (for uses such as piping)
-write-to "path/to/filename" (in addition to -fetch)
  writes the contents of the file to filename, instead of printing to the console
*/

// throws a custom error
void CLI_error() {
    string error = "ERROR: Invalid request";
    throw error;
}

// checks that the file path is valid
void test_path(string path) {
    fstream trying(path);
    if ( !trying ) {
        string error = "ERROR: Invalid file path";
        throw error;
    }
}

// checks that the directory is valid
void test_dir(string path) {
    const char* p = path.c_str();
    int exists = !access(p, F_OK);
    if ( !exists ) {
        string error = "ERROR: invalid directory";
        throw error;
    }
}

// converts a locker name and file name into the stored locker name
string into_dd(string locker_path, string file_name) {
	if (strlen(locker_path) < 1 ) {
		throw "Empty locker path";
	}
	if (strlen(file_name) < 1) {
		throw "Empty file name";
	}
	const string whitespace = "\a\b\f\n\r\t\v";
	if (strpbrk(locker_path, whitespace) != nullptr) {
		throw "Invalid locker path";
	}
	if (strpbrk(file_name, whitespace) != nullptr) {
		throw "Invalid file name";
	}
    string s = locker_path + "/" + file_name;
    return s;
}

// prints the status of the locker
void locker_status(string locker_path) {
    const char* lp = locker_path.c_str();
    // todo
}

// inserts a file into the locker
void add_file(string locker_path, string file_path) {
    test_path(file_path);
    const char* lp = locker_path.c_str();
    DDStore dd(lp);
    const char* fp = file_path.c_str();
    dd.add_document(fp, fp);
}

//
void add_file_change_name(string locker_path, string file_path, string file_name) {
    test_path(file_path);
    // todo
}

void rename_file(string locker_path, string file_name, string old_file_name) {
    test_dir(locker_path);
    // todo
}

void delete_file(string locker_path, string file_name) {
    test_dir(locker_path);
    test_path(into_dd(locker_path, file_name));
    const char* lp = locker_path.c_str();
    DDStore dd(lp);
    const char* fp = file_name.c_str();
    dd.delete_document(fp);
}

void retrieve_to_console(string locker_path, string file_name) {
    test_dir(locker_path);
    const char* lp = locker_path.c_str();
    DDStore dd(lp);
    const char* fp = file_name.c_str();
    int length;
    char* output = (char*)dd.get_document(&length, fp);
    for ( int i = 0; i < length; i++ ) {
        cout << output[i];
    }
    cout << endl;
}

void retrieve_to_file(string locker_path, string file_path, string file_name) {
    test_dir(locker_path);
    test_path(file_path);
    // todo
}

void CLI_parser(vector<string> in) {
  string locker_path, file_path, file_name, old_file_name;
  string command_binary = "0000000";
  string arg, flag;
  vector<string> flags = {"-locker", "-add", "-rename", "-new-name", "-delete", "-fetch", "-write-to"};
  //bool is_command;

  for(int j = 0; j < in.size(); j++) {
    if(j + 1 == in.size()) {
      continue;
    }
    arg = in[j];
    //is_command = false;
    for(int i = 0; i < flags.size(); i++) {
      flag = flags[i];
      size_t found = arg.find(flag);
      if(found == 0) { // string matches and starts at index 0
        if(command_binary[i] == '1') {
          CLI_error(); // that flag was already used
        }
        command_binary[i] = '1';
        //is_command = true;
        if(flag == "-locker") {
          locker_path = in[j + 1];
        }
        else if((flag == "-add") || (flag == "-write-to")) {
          file_path = in[j + 1];
        }
        else if((flag == "-new-name") || (flag == "-delete") || (flag == "-fetch")) {
          file_name = in[j + 1];
        }
        else if(flag == "-rename") {
          old_file_name = in[j + 1];
        }
      }
    }
    //if(!is_command) {
      //CLI_error(); // one of the argvs didn't correspond to a flag
    //}
  }
  if(command_binary[0] == '0') {
    string error = "ERROR: Path to locker must be provided";
    throw error;
  }
  switch(stoi(command_binary)) {
    case 1000000: // -locker
        locker_status(locker_path);
        break;
    case 1100000: // -locker, -add
        add_file(locker_path, file_path);
        break;
    case 1101000: // -locker, -add, -new-name
        add_file_change_name(locker_path, file_path, file_name);
        break;
    case 1011000: // -locker, -rename, -new-name
        rename_file(locker_path, file_name, old_file_name);
        break;
    case 1000100: // -locker, -delete
      delete_file(locker_path, file_name);
      break;
    case 1000010: // -locker, -fetch
      retreive_to_console(locker_path, file_name);
      break;
    case 1000011: // -locker, -fetch, -write-to
      retreive_to_file(locker_path, file_path, file_name);
      break;
    default:
        CLI_error(); // flags used don't correspond to a command
    }
}
