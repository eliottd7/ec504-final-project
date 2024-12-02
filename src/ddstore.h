#include <sys/stat.h>

#define EDITS_MAX 3

// For any given structure, an offset of 0 indicates that it is not present (since this will always be the offset of the header)

typedef enum { EDIT_DELETE, EDIT_INSERT, EDIT_MODIFY, EDIT_NOOP } EditType;
typedef enum { FILE_DOCUMENT, FILE_DIRECTORY } FileType;

// A a single character edit
struct edit_t {
	EditType type;
	char c; // The character that is inserted. If type == EDIT_DELETE, this is meaningless.
	long offset; // Offset within document where this edit occurs. XXX we will need to decide if offset refers to the offset from the beginning of the original document, or if it refers to the offset after all previous edits have been applied
};

// a single character edit as it appears in dist.cpp
struct dedit_t {
	int i;
	EditType type;
	char c;
	dedit_t *prev;
	int cost;
};

// A diff. Specifies a base document and the edits to be performed on it
// to get a file stored in this database
typedef struct {
	long n; // length of file in bytes
	int base; // index of base file in base table
	edit_t edits[EDITS_MAX]; // edits to apply from base. If type == FILE_DIRECTORY. This field is meaningless
} diff_t;

// creates new ddstore at storepath. Fails if storepath exists
class DDStore {
private:
	int dirfd;
	int basedirfd;
	int basetabfd;

	int *nextfreebase;
	int *basetab;
	struct stat basetabst;

	int add_base(char* doc, long n);
	int delete_base(int i);

public:
	DDStore(const char* storepath);
	int add_document(const char *diffpath, const char *docpath);
	void *get_document(int *n, const char *docpath);
};

// ddstore-internal functions
int generate_edit_list(char*, long, char*, long, edit_t*);
int edit_list(edit_t*, char*, int, char*, int);
