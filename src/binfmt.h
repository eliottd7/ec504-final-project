using namespace std;

#define S_IDENT 0xdededebe

#define EDITS_MAX 10

#define INIT_STRTAB 256
#define INIT_BASE 4
#define INIT_STFILE 20

// For any given structure, an offset of 0 indicates that it is not present (since this will always be the offset of the header)

typedef enum { EDIT_DELETE, EDIT_INSERT, EDIT_MODIFY } EditType;
typedef enum { FILE_DOCUMENT, FILE_DIRECTORY } FileType;

// A a single character edit
typedef struct {
	EditType type;
	char c; // The character that is inserted. If type == EDIT_DELETE, this is meaningless.
	long offset; // Offset within document where this edit occurs. XXX we will need to decide if offset refers to the offset from the beginning of the original document, or if it refers to the offset after all previous edits have been applied
} edit_t;

// A storefile. Can either be a document or a directory based on type field
typedef struct {
	char name[16]; // name of file or directory
	FileType type; // whether this is a document or a directory
	int idx; // if type == FILE_DOCUMENT, is index into basetab. if type == FILE_DIRECTORY, it's an index into stfiletab
	edit_t edits[EDITS_MAX]; // edits to apply from base. If type == FILE_DIRECTORY. This field is meaningless
} stfile_t;


// A base (document) to which edits are applied to get documents stored in our database
typedef struct {
	int refs; // number of references to this base. When this is 0, should be deleted
	long size; // size of document
	long offset; // offset at which this base is stored
} base_t;


// store header
typedef struct {
	long s_ident; // 0xdededebe	
	int n_stfile; 
	long stfiletab; // offset of stfiletab
	int n_base; // length of basetab
	long basetab; // offset of base table
} sthdr_t;



