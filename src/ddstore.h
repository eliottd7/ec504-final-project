#pragma once

#include <sys/stat.h>

#ifndef EDITS_MAX
#define EDITS_MAX 5
#endif

// For any given structure, an offset of 0 indicates that it is not present (since this will always be the offset of the header)

typedef enum {
    EDIT_DELETE,
    EDIT_INSERT,
    EDIT_MODIFY,
    EDIT_NOOP
} EditType;
typedef enum {
    FILE_DOCUMENT,
    FILE_DIRECTORY
} FileType;

// A a single character edit
struct edit_t {
    EditType type;
    char c;      // The character that is inserted. If type == EDIT_DELETE, this is meaningless.
    long offset; // Offset within document where this edit occurs. XXX we will need to decide if offset refers to the offset from the
                 // beginning of the original document, or if it refers to the offset after all previous edits have been applied
};

// a single character edit as it appears in dist.cpp
struct dedit_t {
    int i;
    EditType type;
    char c;
    dedit_t* next;
    int cost;
};

// A diff. Specifies a base document and the edits to be performed on it
// to get a file stored in this database
typedef struct {
    long n;                  // length of file in bytes
    int base;                // index of base file in base table
    edit_t edits[EDITS_MAX]; // edits to apply from base. If type == FILE_DIRECTORY. This field is meaningless
} diff_t;

// creates new ddstore at storepath. Fails if storepath exists
class DDStore {
private:
    int dirfd;
    int basedirfd;
    int basetabfd;

    int* nextfreebase;
    int* basetab;
    struct stat basetabst;

    int add_base(char* doc, long n);
    int delete_base(int i);

private:
	// ddstore-internal functions
	static int generate_edit_list(char*, long, char*, long, edit_t*);
	static int edit_list(edit_t*, char*, int, char*, int);
	static int dist(char* s1, int i1, char* s2, int i2, dedit_t** edits);

public:
    // opens the DDStore located at storepath. Creates one if it
    // does not already exists.
    DDStore(const char* storepath);

    // adds a document located at docpath to the store as a diff
    // with the path diffpath. If none of the current base documents
    // match the new document, adds it as a base
    int add_document(const char* diffpath, const char* docpath);

    // returns a pointer to a malloc'd buffer of the document specified
    // by the diff at diffpath. Writes the size of the buffer, in bytes
    // to n
    void* get_document(int* n, const char* diffpath);

    // remvoes the specified document from the store
    int delete_document(const char* diffpath);
};
