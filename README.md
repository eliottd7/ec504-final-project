# EC 504 Final Project
Vance Raiti, Pranet Sharma, Linden Adamson, Matthew Guacho, Eliott Dinfotan

## Requirements
*Chosen Project: DeDuplicator File Locker*

Features
The fundamental project is described as "an efficient data storage locker that utilizes deduplication." We will be developing this project in C++ and providing a makefile to enable cross-platform builds.

Minimum requirements:
- Store (at least) 10, 10MB files
- Retrieve any stored file at any time in any access order
- Persist between program runtimes and be operational on other computers given the same locker
- CLI Interface

Chosen Extensions (up to 35% of the final grade):
- File Deletion- 20%
- Allowed arbitrary binary files- 5%
- Networked access to the locker- 10%
- Store directory of files as a single entity- 15%

## Command Line Interface
- Format example:
	- `store -file <path/to/filename> -locker <path/to/locker> -save-as .`
- 5 flags:
	- -locker
	- -add
 	- -delete
 	- -fetch
 	- -save-as
- 4 commands can be built from these flags (flags can be in any order):
	- locker status
 		- store -locker
	- add file
 		- store -locker, -add, -save-as
   			- "-save-as ." will keep the file's name
	- delete file
 		- store -locker, -delete
	- reconstruct file
 		- store -locker, -fetch, -save-as
   			- "-fetch ." will reconstruct everything in storage
      		- "-save-as ." will place the reconstructed files in the current directory

## References

