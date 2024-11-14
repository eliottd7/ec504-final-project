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
	- `store -file="path/to/filename" -locker="path/to/locker" -new-name="file01`
- Output to command line:
	- `file01 saved`
	- `Locker contents: N files, XX.X KB/MB/GB used`
- 7 flags:
	- -locker="path/to/locker"
	- -add="path/to/filename"
 	- -rename="name"
 	- -new-name="name"
 	- -delete="name"
 	- -retrieve="name"
 	- -write-to="path/to/filename"
- 7 commands can be built from these flags (flags can be in any order):
	- locker status
 		- store -locker
	- add file
 		- store -locker, -add
	- add file and change its name
 		- store -locker, -add, -new-name
	- rename file
 		- store -locker, -rename, -new-name
	- delete file
 		- store -locker, -delete
	- print file to console
 		- store -locker, -retrieve
	- write file to path
 		- store -locker, -retrieve, -new-name

## References

