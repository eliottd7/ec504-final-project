#include "binfmt.h"

int new_ddstore(const char*);
int add_document(const char*, const char*);
void *get_document(int*, const char*, const char*);
int generate_edit_list(char*, long, char*, long, edit_t*);

int edit_list(edit_t*, char*, int, char*, int);
