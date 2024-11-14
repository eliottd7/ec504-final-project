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
- CLI Interface: 
	- `store -file "path/to/filename" -locker "path/to/locker"`
	- Output: `Contents: N files, XX.X KB/MB/GB used`
 	- 7 commands:
  		- locker status
  		- add file
  		- add file and change its name
  		- rename file
  		- delete file
  		- print file to console
  		- write file to path

Chosen Extensions (up to 35% of the final grade):
- File Deletion- 20%
- Allowed arbitrary binary files- 5%
- Networked access to the locker- 10%
- Store directory of files as a single entity- 15%

## Command Line Interface
 todo: add instructions for command line interface here

## References

