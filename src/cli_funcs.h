#pragma once

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
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
namespace fs = filesystem;

/*
Accepted flags:
-locker "path/to/locker"
    prints to the command_binary line: Contents: N files, XX.X KB/MB/GB used, {list of names of files}
-file "path/to/filename" (in addition to -locker)
    adds file to locker, also prints to the command_binary line locker data
-save-as "/path/in/locker" (in addition to -file)
    changes the name of the file to that of how it will be stored in the locker
-delete "/path/in/locker" (in addition to -locker)
    removes the file from the locker, also prints to the command_binary line locker data
-fetch "/path/in/locker" (in addition to -locker)
    prints to the console the contents of the file (for uses such as piping)
save-as "path/to/filename" (in addition to -fetch)
    writes the contents of the file to filename, instead of printing to the console
*/

// throws a custom error
void CLI_error() {
    string error = "ERROR: Invalid request";
    throw error;
}

// prints the status of the locker
void locker_status(string locker_path) {
    DDStore dd(locker_path.c_str());
    const fs::path locker(locker_path);
    vector<string> prefixes = {"B", "KB", "MB", "GB"};
    string name;
    int count = 0;
    float size;
    for (auto const& path:fs::recursive_directory_iterator{locker}) {
        string name(path.path());
        if(!path.is_directory() && (name.find(".ddstore") == string::npos)) {
            name = name.substr(locker_path.size() + 1);
            size = (float)dd.get_document_size(name.data());
            for ( int i = 0; i < 3; i++ ) {
                if ( size > 1024 ) {
                    count++;
                    size /= 1024.;
                }
            }
            cout << name << ": ";
            if ( count ) {
                printf("%4.1f", size);
            } else {
                cout << size;
            }
            cout << " " << prefixes[count] << endl;
        }
    }
}

// inserts a file/files into the locker, no locker subfolders
void add_file(string locker_path, string out_locker_path) {
    DDStore dd(locker_path.c_str());
    const fs::directory_entry path_out_locker_path(out_locker_path);
    if(path_out_locker_path.is_directory()) {
        for (auto const& sub_path:fs::recursive_directory_iterator{path_out_locker_path}) {
            if(!sub_path.is_directory()) {
                string name(sub_path.path());
                string in_locker_path = name.substr(name.find_last_of("/") + 1);
                dd.add_document(in_locker_path.c_str(), name.c_str());
            }
        }
    }
    else {
        string in_locker_path = out_locker_path.substr(out_locker_path.find_last_of("/") + 1);
        dd.add_document(in_locker_path.c_str(), out_locker_path.c_str());
    }
}

//
void add_file_save_as(string locker_path, string out_locker_path, string in_locker_path) {
    DDStore dd(locker_path.c_str());
    const fs::directory_entry path_out_locker_path(out_locker_path);
    if(path_out_locker_path.is_directory()) {
        for (auto const& sub_path:fs::recursive_directory_iterator{path_out_locker_path}) {
            if(!sub_path.is_directory()) { // this is string purgatory
                string name(sub_path.path());
                string in_locker_name = name;
                in_locker_name.erase(0, out_locker_path.size());
                in_locker_name = in_locker_path + "/" + in_locker_name;
                const fs::directory_entry path_locker_path(locker_path + "/" + 
                    in_locker_name.substr(0, in_locker_name.find_last_of("/")));
                if(!path_locker_path.exists()) {
                    fs::create_directories(path_locker_path.path());
                }
                dd.add_document(in_locker_name.c_str(), name.c_str());
            }
        }
    }
    else {
        const fs::directory_entry path_locker_path(locker_path + "/" + 
            in_locker_path.substr(0, in_locker_path.find_last_of("/")));
        if(!path_locker_path.exists()) {
            fs::create_directories(path_locker_path.path());
        }
        dd.add_document(in_locker_path.c_str(), out_locker_path.c_str());
    }
}

void delete_file(string locker_path, string in_locker_path) {
    if(in_locker_path == ".") {
        const fs::directory_entry path_in_locker_path(locker_path);
        fs::remove_all(path_in_locker_path);
        return;
    }
    DDStore dd(locker_path.c_str());
    const fs::directory_entry path_in_locker_path(locker_path + '/' + in_locker_path);
    if(path_in_locker_path.is_directory()) {
        for (auto const& sub_path:fs::recursive_directory_iterator{path_in_locker_path}) {
            if(!sub_path.is_directory()) {
                string name(sub_path.path());
                dd.delete_document(name.substr(locker_path.size() + 1).c_str());
            }
        }
        fs::remove_all(path_in_locker_path);
    }
    else {
        dd.delete_document(in_locker_path.c_str());
    }
}

void fetch_helper(DDStore dd, string in, string out) {
    int length;
    char* output = (char*)dd.get_document(&length, in.c_str());
    ofstream open_file(out);
    for ( int i = 0; i < length; i++ ) {
        open_file << output[i];
    }
    open_file.close();
}

void fetch_to_file(string locker_path, string out_locker_path, string in_locker_path) {
    if(out_locker_path == ".") {
        out_locker_path = fs::current_path();
    }
    if(in_locker_path == ".") {
        in_locker_path = "";
    }
    DDStore dd(locker_path.c_str());
    const fs::directory_entry path_in_locker_path(locker_path + '/' + in_locker_path);
    const fs::directory_entry path_out_locker_path(out_locker_path);
    if(path_in_locker_path.is_directory()) {
        for (auto const& sub_path:fs::recursive_directory_iterator{path_in_locker_path}) {
            string temp(sub_path.path());
            string name(out_locker_path + "/" + temp.erase(0, locker_path.size() + 1));
            if(!sub_path.is_directory() && (name.find(".ddstore") == string::npos)) {
                const fs::directory_entry running_out_of_names_here(name.substr(0, name.find_last_of("/")));
                if(!running_out_of_names_here.exists()) {
                    fs::create_directories(running_out_of_names_here.path());
                }
                fetch_helper(dd, temp, name);
            }
        }
    }
    else {
        if(path_out_locker_path.is_directory()) {
            string name(out_locker_path + "/" + in_locker_path);
            const fs::directory_entry running_out_of_names_here(name.substr(0, name.find_last_of("/")));
            if(!running_out_of_names_here.exists()) {
                fs::create_directories(running_out_of_names_here.path());
            }
            cout << running_out_of_names_here << ", " << name << endl;
            fetch_helper(dd, in_locker_path, name);
        }
        else {
            cout << "here!\n";
            fetch_helper(dd, in_locker_path, out_locker_path);
        }
    }
}

void CLI_parser(vector<string> in) {
    string locker_path, out_locker_path, in_locker_path, save_path;
    string command_binary = "00000";
    string arg, flag;
    vector<string> flags = {"-locker", "-file", "-delete", "-fetch", "-save-as"};

    for(int j = 0; j < in.size(); j++) {
        if(j + 1 == in.size()) {
            continue;
        }
        arg = in[j];
        for(int i = 0; i < flags.size(); i++) {
            flag = flags[i];
            size_t found = arg.find(flag);
            if(found == 0) { // string matches and starts at index 0
                if(command_binary[i] == '1') {
                    CLI_error(); // that flag was already used
                }
                command_binary[i] = '1';
                if(flag == "-locker") {
                    locker_path = in[j + 1];
                }
                else if(flag == "-file") {
                    out_locker_path = in[j + 1];
                }
                else if((flag == "-delete") || (flag == "-fetch")) {
                    in_locker_path = in[j + 1];
                }
                else if(flag == "-save-as") {
                    save_path = in[j + 1];
                }
            }
        }
    }
    if(command_binary[0] == '0') {
        string error = "ERROR: Path to locker must be provided";
        throw error;
    }
    switch(stoi(command_binary)) {
    case 10000: // -locker
        locker_status(locker_path);
        break;
    case 11001: // -locker, -file, -save-as
        if(save_path == ".") {
            add_file(locker_path, out_locker_path);
        }
        else {
            add_file_save_as(locker_path, out_locker_path, save_path);
        }
        break;
    case 10100: // -locker, -delete
        delete_file(locker_path, in_locker_path);
        break;
    case 10011: // -locker, -fetch, -save-as
        fetch_to_file(locker_path, save_path, in_locker_path);
        break;
    default:
        CLI_error(); // flags used don't correspond to a command
    }
}

int call_dd(int argc, char** argv) {
    vector<string> CLI_input;
    for ( int i = 1; i < argc; i++ ) {
        string s(argv[i]);
        CLI_input.push_back(s);
    }
    try {
        CLI_parser(CLI_input);
    } catch ( string error ) {
        dprintf(2, "%s\n", error.data());
        return -1;
    }
    return 0;
}
