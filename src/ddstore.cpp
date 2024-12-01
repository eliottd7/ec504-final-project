using namespace std;

#include "ddstore.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define MODE (S_IRWXU|S_IRWXG|S_IROTH)

// de-duplicator store
int new_ddstore(const char *path) {
	// creates a new ddstore at path
	
	struct stat st;
	int dirfd, fd, err;

	if (stat(path,&st) == 0) {
		dprintf(2,"cannot create ddstore at %s: path already exists\n",path);
		return 1;
	}

	if (mkdir(path, MODE)) {
		perror("could not create new ddstore");	
		return 1;
	}

	dirfd = open(path, O_DIRECTORY | O_RDONLY);
	if (mkdirat(dirfd, "basedocs", MODE)) {
		perror("could not create basedocs directory");
		return 1;
	}
	
	dirfd = openat(dirfd, "basedocs", O_DIRECTORY | O_RDONLY);
	fd = openat(dirfd, "basetab", O_CREAT|O_TRUNC|O_WRONLY, MODE);
	if (fd == -1) {
		perror("could not create basetab");
		return 1;
	}
	ftruncate(fd, sizeof(int) * 2);
	close(fd);

	return 0;
}
	
int add_document(const char *docpath, const char *storepath) {
	// adds the file at path as a storefile to our ddstore
	// returns 0 on success, -1 otherwise
	struct stat docst, basetabst, basest;
	char *doc, *base, basepath[2], newbase; 
	int fd, dirfd, basedirfd, basetabfd, basefd, docfd;
	int i;
	bool match;
	edit_t edits[EDITS_MAX + 1];
	int *basetab, *nextfreebase;

	for (int i = 0; i < EDITS_MAX; i++) {
		edits[i].type = EDIT_NOOP;
	}

	if (stat(docpath, &docst)){
		perror("could not access document");
		return 1;
	}
	fd = open(docpath, O_RDONLY);
	doc = (char*) mmap(NULL, docst.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	if ((dirfd = open(storepath, O_DIRECTORY | O_RDONLY)) == -1 ) {
		perror("could not open store");
		return 1;
	}

	if ((basedirfd = openat(dirfd, "basedocs", O_DIRECTORY | O_RDONLY)) == -1) {
		perror("could not open basedocs");
		return 1;
	}
	
	if ((basetabfd = openat(basedirfd, "basetab", O_RDWR)) == -1) {
		perror("could not open basetab");
		return 1;
	}
	
	if (faccessat(dirfd, docpath, F_OK, 0) == 0) {
		dprintf(2, "cannot add document to store. Path already exists\n");
		return 1;
	}

	fstat(basetabfd, &basetabst);
	nextfreebase = (int*) mmap(NULL, basetabst.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, basetabfd, 0);
	basetab = &nextfreebase[1];

	match = false;
	for (i = 0; i < *nextfreebase; i++) {
		sprintf(basepath,"%d",i);
		if (faccessat(basedirfd, basepath, F_OK, 0) != 0)
			continue;
		
		basefd = open(basepath, O_RDONLY);
		fstat(basefd, &basest);
		base = (char*) mmap(NULL, basest.st_size, PROT_READ, MAP_SHARED, basefd, 0);
		
		if (match = generate_edit_list(base, basest.st_size, doc, docst.st_size, edits)) {
			break;
		}
		
		close(basefd);
		munmap(base, basest.st_size);
	}

	if (!match) {
		// create a new basetab entry with d1
		sprintf(basepath, "%d", *nextfreebase);
		basefd = openat(basedirfd, basepath, O_RDWR | O_CREAT | O_TRUNC, MODE);
		
		ftruncate(basefd, docst.st_size);
		base = (char*) mmap(NULL, docst.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, basefd, 0);
		memcpy(base, doc, docst.st_size);

		close(basefd);
		munmap(base, docst.st_size);
		
		i = *nextfreebase;
		*nextfreebase++;
		if ((*nextfreebase + 1) * sizeof(int) > basetabst.st_size)
			ftruncate(basetabfd, basetabst.st_size + sizeof(int) * 16);
	}

	basetab[i] += 1;

	docfd = openat(dirfd, docpath, O_CREAT | O_RDWR | O_TRUNC);
	stfile_t s;
	s.base = i;
	s.n = docst.st_size;
	
	memcpy(s.edits, edits, sizeof(edit_t) * EDITS_MAX);
	write(docfd, &s, sizeof(stfile_t));
	close(docfd);
	
	munmap(doc, docst.st_size);

	close(dirfd);
	close(basedirfd);
	close(basetabfd);

	return 0;
}

void *get_document(int *n, const char *docpath,const char *storepath) {
	// retrieves file stored at path and places it into malloc'd buffer
	// stores size of buffer in n
	// 
	// returns address of buffer
	char *base, basepath[16], *buf;
	int dirfd, basedirfd, basefd, docfd;
	long i1, i2;
	edit_t *e;
	stfile_t *doc;
	struct stat docst, basest;

	if ((dirfd = open(storepath, O_DIRECTORY | O_RDONLY)) == -1 ) {
		perror("could not open store");
		return (void*) -1;
	}

	if ((basedirfd = openat(dirfd, "basedocs", O_DIRECTORY | O_RDONLY)) == -1) {
		perror("could not open basedocs");
		return (void*) -1;
	}

	if ((docfd = openat(dirfd, docpath, O_RDONLY)) == -1) {
		perror("could not open document");
		return (void*) -1;
	}
	
	fstat(docfd, &docst);
	doc = (stfile_t*) mmap(NULL, docst.st_size, PROT_READ, MAP_SHARED, docfd, 0);
	
	sprintf(basepath,"%d",doc->base);
	if ((basefd = openat(basedirfd, basepath, O_RDONLY)) == -1) {
		perror("could not open basetab");
		return (void*) -1;
	}
	fstat(basefd, &basest);
	base = (char*) mmap(NULL, basest.st_size, PROT_READ, MAP_SHARED, basefd, 0);
	
	buf = (char*) malloc(doc->n);

	e = &doc->edits[0];
	i1 = i2 = 0;
	while (i1 < doc->n) {
		if (i1 == e->offset) {
			if (e->type == EDIT_MODIFY) {
				buf[i1] = e->c;
				i1++;
				i2++;
			}
			if (e->type == EDIT_DELETE) {
				i2++;
			}
			if (e->type == EDIT_INSERT) {
				buf[i1] = e->c;
				i1++;
			}

			if (e+1 < &doc->edits[EDITS_MAX])
				e++;
		} else {
			buf[i1] = base[i2];
			i1++;
			i2++;
		}
	}
	
	*n = doc->n;
	return (void*) buf;

}

int generate_edit_list(char *s1, long n1, char *s2, long n2, edit_t *edits) {
	// given two potentially very long strings, produces a list of edits
	// to transform d1 to d2
	// 
	// returns 0 if it would take more than EDITS_MAX edits to 
	
	long i1, i2;
	int e, j1, j2, d;

	if (EDITS_MAX < (n1 > n2 ? n1 - n2 : n2 - n1)) {
		return false;
	}
	
	e = EDITS_MAX;
	i1 = 0;
	i2 = 0;
	while (i1 < n1 && i2 < n2) {
		if (s1[i1] == s2[i2]) { // XXX this could be optimized with vectorized comparisons
			i1++;
			i2++;
		} else {
			if (e == 0) {
				return false;
			}
			j1 = n1 - i1 < e + 1 ? n1 - i1 : e + 1;
			j2 = n2 - i2 < e + 1? n2 - i2 : e + 1;
			d = edit_list(&edits[EDITS_MAX-e], &s1[i1], j1, &s2[i2], j2);
			if (d > e) {
				return false;
			}
			for (int i = EDITS_MAX-e; i < EDITS_MAX-e+d; i++) {
				edits[i].offset += i1;
			}
			i1 = j1;
			i2 = j2;
		}
	}
	return true;
}
