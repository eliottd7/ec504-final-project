using namespace std;

#include "ddstore.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define DIR_MODE (S_IRWXU | S_IRWXG | S_IROTH)
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)

// de-duplicator store

int DDStore::add_base(char* doc, long n) {
    int i;
    int basefd;
    char basepath[16], *base;

    i = *this->nextfreebase;
    (*this->nextfreebase)++;

    if ( ((*this->nextfreebase) + 1) * sizeof(int) > (unsigned long)this->basetabst.st_size ) {
        ftruncate(basetabfd, basetabst.st_size + sizeof(int) * 16);
        munmap(this->nextfreebase, this->basetabst.st_size);

        fstat(this->basetabfd, &this->basetabst);
        this->nextfreebase = (int*)mmap(NULL, this->basetabst.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, this->basetabfd, 0);
        basetab = &this->nextfreebase[1];
    }

    sprintf(basepath, "%d", i);
    basefd = openat(this->basedirfd, basepath, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
    ftruncate(basefd, n);
    base = (char*)mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, basefd, 0);
    memcpy(base, doc, n);

    close(basefd);
    munmap(base, n);

    return i;
}

int DDStore::delete_base(int i) {
    char basepath[16];

    if ( basetab[i] != 0 ) {
        dprintf(2, "cannot delete: base %d refcount is not 0\n", i);
        return 1;
    }

    sprintf(basepath, "%d", i);
    unlinkat(basedirfd, basepath, 0);
    return 0;
}

DDStore::DDStore(const char* storepath) {
    int exists = !access(storepath, F_OK);

    if ( !exists ) {
        if ( mkdir(storepath, DIR_MODE) ) {
            perror("could not create new ddstore");
            return;
        } else {
            dprintf(2, "ddstore created\n");
        }
    }
    this->dirfd = open(storepath, O_DIRECTORY | O_RDONLY);
    if ( !exists ) {
        if ( mkdirat(this->dirfd, ".ddstore", DIR_MODE) ) {
            perror("could not create .ddstore directory");
            return;
        }
    }
    this->basedirfd = openat(dirfd, ".ddstore", O_DIRECTORY | O_RDONLY);
    if ( !exists ) {
        this->basetabfd = openat(basedirfd, "basetab", O_CREAT | O_TRUNC | O_RDWR, FILE_MODE);
        ftruncate(this->basetabfd, sizeof(int) * 2);
    } else {
        this->basetabfd = openat(basedirfd, "basetab", O_RDWR, FILE_MODE);
    }

    fstat(this->basetabfd, &this->basetabst);
    this->nextfreebase = (int*)mmap(NULL, this->basetabst.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, this->basetabfd, 0);
    this->basetab = &this->nextfreebase[1];
}

int DDStore::add_document(const char* diffpath, const char* docpath) {
    // adds the file at path path to the store at path docpath
    // returns 0 on success, -1 otherwise
    struct stat docst, basest;
    char *doc, *base;
    char basepath[16];
    int docfd, basefd, difffd;
    int i;
    bool match;
    edit_t edits[EDITS_MAX + 1];
    diff_t diff;

    if ( stat(docpath, &docst) ) {
        perror("Could not access document");
        return 1;
    }

    if ( faccessat(this->dirfd, diffpath, F_OK, 0) == 0 ) {
        dprintf(2, "Cannot add document %s to store. Path already exists\n", diffpath);
        return 1;
    }

    for ( int i = 0; i < EDITS_MAX; i++ ) {
        edits[i].type = EDIT_NOOP;
        edits[i].offset = -1;
    }

    docfd = open(docpath, O_RDONLY);
    doc = (char*)mmap(NULL, docst.st_size, PROT_READ, MAP_SHARED, docfd, 0);
    close(docfd);

    match = false;
    for ( i = 0; i < nextfreebase[1]; i++ ) {
        sprintf(basepath, "%d", i);
        if ( faccessat(basedirfd, basepath, F_OK, 0) != 0 )
            continue;

        basefd = openat(basedirfd, basepath, O_RDONLY);
        fstat(basefd, &basest);
        base = (char*)mmap(NULL, basest.st_size, PROT_READ, MAP_SHARED, basefd, 0);
        if ( base == NULL ) {
            perror("couldn't read base");
            exit(1);
        }

        if ( (match = generate_edit_list(base, basest.st_size, doc, docst.st_size, edits)) ) {
            break;
        }

        close(basefd);
        munmap(base, basest.st_size);
    }

    if ( !match ) {
        i = this->add_base(doc, docst.st_size);
    }

    basetab[i] += 1;

    diff.base = i;
    diff.n = docst.st_size;
    memcpy(diff.edits, edits, sizeof(edit_t) * EDITS_MAX);
    difffd = openat(dirfd, diffpath, O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
    write(difffd, &diff, sizeof(diff_t));
    close(difffd);

    munmap(doc, docst.st_size);

    return 0;
}

int DDStore::get_document_size(const char* diffpath) {
    int difffd;
    diff_t* diff;
    if ( (difffd = openat(this->dirfd, diffpath, O_RDONLY)) == -1 ) {
        perror("could not open diff");
        return -1;
    }
    diff = (diff_t*)mmap(NULL, sizeof(diff_t), PROT_READ, MAP_SHARED, difffd, 0);
    return diff[0].n;
}

void* DDStore::get_document(int* n, const char* diffpath) {
    // retrieves file stored at path and places it into malloc'd buffer
    // stores size of buffer in n
    //
    // returns address of buffer
    char *base, basepath[16], *doc;
    long i1, i2;
    struct stat basest;
    int basefd, difffd;
    edit_t* e;
    diff_t* diff;

    if ( (difffd = openat(this->dirfd, diffpath, O_RDONLY)) == -1 ) {
        perror("could not open diff");
        return (void*)-1;
    }

    diff = (diff_t*)mmap(NULL, sizeof(diff_t), PROT_READ, MAP_SHARED, difffd, 0);

    sprintf(basepath, "%d", diff->base);
    if ( (basefd = openat(this->basedirfd, basepath, O_RDONLY)) == -1 ) {
        perror("could not open basetab");
        return (void*)-1;
    }

    fstat(basefd, &basest);
    base = (char*)mmap(NULL, basest.st_size, PROT_READ, MAP_SHARED, basefd, 0);

    doc = (char*)malloc(diff->n);

    e = &diff->edits[0];
    i1 = i2 = 0;
    while ( i1 < diff->n ) {
        if ( i2 == e->offset ) {
            if ( e->type == EDIT_MODIFY ) {
                doc[i1] = e->c;
                i1++;
                i2++;
            }
            if ( e->type == EDIT_DELETE ) {
                i2++;
            }
            if ( e->type == EDIT_INSERT ) {
                doc[i1] = e->c;
                i1++;
            }

            if ( e + 1 < &diff->edits[EDITS_MAX] )
                e++;
        } else {
            doc[i1] = base[i2];
            i1++;
            i2++;
        }
    }

    *n = diff->n;

    close(basefd);
    close(difffd);
    munmap(diff, sizeof(diff_t));
    munmap(base, basest.st_size);

    return (void*)doc;
}

int DDStore::delete_document(const char* diffpath) {
    diff_t* diff;
    int difffd;

    if ( faccessat(this->dirfd, diffpath, F_OK, 0) ) {
        dprintf(2, "can't remove diff: file does not exist");
        return 1;
    }

    difffd = openat(this->dirfd, diffpath, O_RDWR);
    diff = (diff_t*)mmap(NULL, sizeof(diff_t), PROT_READ | PROT_WRITE, MAP_SHARED, difffd, 0);

    basetab[diff->base] -= 1;
    if ( basetab[diff->base] == 0 ) {
        this->delete_base(diff->base);
    }

    munmap(diff, sizeof(diff_t));
    close(difffd);

    unlinkat(this->dirfd, diffpath, 0);

    return 0;
}

int DDStore::generate_edit_list(char* s1, long n1, char* s2, long n2, edit_t* edits) {
    // given two potentially very long strings, produces a list of edits
    // to transform d1 to d2
    //
    // returns 0 if it would take more than EDITS_MAX edits
    // to perform transformation

    long i1, i2;
    int e, r, j1, j2, d, b, e1, d1, d2;

    if ( EDITS_MAX < (n1 > n2 ? n1 - n2 : n2 - n1) ) {
        return false;
    }

    e = 0;
    r = EDITS_MAX; // remaining edits available
    i1 = 0;
    i2 = 0;
    while ( i1 < n1 || i2 < n2 ) {
        if ( s1[i1] == s2[i2] ) { // XXX this could be optimized with vectorized comparisons
            i1++;
            i2++;
        } else {
            b = 2 * r + 1;
            j1 = b > n1 - i1 ? n1 - i1 : b;
            j2 = b > n2 - i2 ? n2 - i2 : b;
            d = edit_list(&edits[e], &s1[i1], j1, &s2[i2], j2);

            // remove edits that are not contiguous with the first character
            e1 = e + d;
            d1 = d2 = edits[e].offset;
            for ( edit_t* edit = &edits[e]; edit < &edits[e1]; edit++ ) {
                if ( d1 == edit->offset ) {
                    edit->offset += i1;
                    if ( edit->type != EDIT_INSERT ) {
                        d1 += 1;
                    }
                    if ( edit->type != EDIT_DELETE ) {
                        d2 += 1;
                    }
                } else {
                    d--;
                    edit->type = EDIT_NOOP;
                }
            }

            e += d;
            r -= d;

            if ( r < 0 )
                return false;

            i1 += d1;
            i2 += d2;
        }
    }

    return true;
}

int DDStore::dist(char* s1, int i1, char* s2, int i2, dedit_t** edits) {
    int opts[3];
    int cost;
    dedit_t *e, *next;
    EditType type;

    if ( edits[i1][i2].cost != -1 ) {
        return edits[i1][i2].cost;
    }

    opts[0] = (s1[i1] != s2[i2]) + dist(s1, i1 + 1, s2, i2 + 1, edits); // modify character. Do not apply cost if characters are same
    opts[1] = 1 + dist(s1, i1 + 1, s2, i2, edits);                      // delete character
    opts[2] = 1 + dist(s1, i1, s2, i2 + 1, edits);                      // insert character

    if ( opts[0] < opts[1] && opts[0] < opts[2] ) {
        cost = opts[0];
        type = s1[i1] == s2[i2] ? EDIT_NOOP : EDIT_MODIFY;
        next = &edits[i1 + 1][i2 + 1];
    } else if ( opts[1] < opts[2] ) {
        cost = opts[1];
        type = EDIT_DELETE;
        next = &edits[i1 + 1][i2];
    } else {
        cost = opts[2];
        type = EDIT_INSERT;
        next = &edits[i1][i2 + 1];
    }

    e = &edits[i1][i2];
    e->i = i1;
    e->type = type;
    e->c = s2[i2];
    e->next = next;
    e->cost = cost;

    return e->cost;
}

int DDStore::edit_list(edit_t* edits, char* s1, int n1, char* s2, int n2) {
    // produces list of character edits to transform s1 to s2

    dedit_t** dedits = (dedit_t**)malloc(sizeof(dedit_t*) * (n1 + 1));
    for ( int i = 0; i < n1 + 1; i++ ) {
        dedits[i] = (dedit_t*)malloc(sizeof(dedit_t) * (n2 + 1));
    }

    dedits[n1][n2].cost = 0;
    dedits[n1][n2].type = EDIT_NOOP;
    dedits[n1][n2].next = (dedit_t*)0x0;
    dedits[n1][n2].i = -1;

    for ( int i1 = 0; i1 < n1; i1++ ) {
        dedits[i1][n2].cost = n1 - i1;
        dedits[i1][n2].type = EDIT_DELETE;
        dedits[i1][n2].next = &dedits[i1 + 1][n2];
        dedits[i1][n2].i = i1;
    }

    for ( int i2 = 0; i2 < n2; i2++ ) {
        dedits[n1][i2].cost = n2 - i2;
        dedits[n1][i2].c = s2[i2];
        dedits[n1][i2].type = EDIT_INSERT;
        dedits[n1][i2].next = &dedits[n1][i2 + 1];
        dedits[n1][i2].i = n1;
    }

    for ( int i1 = 0; i1 < n1; i1++ ) {
        for ( int i2 = 0; i2 < n2; i2++ ) {
            dedits[i1][i2].cost = -1;
        }
    }

    int cost = dist(s1, 0, s2, 0, dedits);

    dedit_t* e = &dedits[0][0];
    int i = 0;
    while ( i < cost ) {
        if ( e->type != EDIT_NOOP ) {
            edits[i].type = e->type;
            edits[i].offset = e->i;
            edits[i].c = e->c;
            i++;
        }
        e = e->next;
    }
    return cost;
}

void print_edit_list(edit_t* edits, int n) {
    char* ident = {};
    char modify[] = "modify";
    char del[] = "delete";
    char insert[] = "insert";

    for ( edit_t* e = edits; e < &edits[n]; e++ ) {
        switch ( e->type ) {
        case EDIT_NOOP:
            continue;
        case EDIT_MODIFY:
            ident = modify;
            break;
        case EDIT_DELETE:
            ident = del;
            break;
        case EDIT_INSERT:
            ident = insert;
            break;
        }
        printf("(%s, %c, %ld)\n", ident, e->c, e->offset);
    }
}
