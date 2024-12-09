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

#define DIR_MODE (S_IRWXU|S_IRWXG|S_IROTH)
#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)

// de-duplicator store

int DDStore::add_base(char *doc, long n) {
	int i;
	int basefd;
	char basepath[16], *base;

	i = *this->nextfreebase;
	*this->nextfreebase++;
	
	if ((*this->nextfreebase + 1) * sizeof(int) > this->basetabst.st_size) {
		ftruncate(basetabfd, basetabst.st_size + sizeof(int) * 16);
		munmap(this->nextfreebase,this->basetabst.st_size);
		
		fstat(this->basetabfd, &this->basetabst);
		this->nextfreebase = (int*) mmap(NULL, this->basetabst.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, this->basetabfd, 0);
		basetab = &this->nextfreebase[1];
	}

	sprintf(basepath, "%d", i);
	basefd = openat(this->basedirfd, basepath, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
	ftruncate(basefd, n);
	base = (char*) mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, basefd, 0);
	memcpy(base, doc, n);
	
	close(basefd);
	munmap(base,n);
	
	return i;
}

int DDStore::delete_base(int i) {
	char basepath[16];
	
	if (basetab[i] != 0) {
		dprintf(2, "cannot delete: base %d refcount is not 0\n", i);
		return 1;
	}

	sprintf(basepath,"%d",i);
	unlinkat(basedirfd,basepath,0);
	return 0;
}

DDStore::DDStore(const char *storepath) {
	int exists = !access(storepath, F_OK);

	if (!exists) {
		if (mkdir(storepath, DIR_MODE)) {
			perror("could not create new ddstore");
			return;
		}
		else {
			dprintf(2, "ddstore created\n");
		}
	}
	this->dirfd = open(storepath, O_DIRECTORY | O_RDONLY);
	if (!exists) {
		if (mkdirat(this->dirfd, "basedocs", DIR_MODE)) {
			perror("could not create basedocs directory");
			return;
		}
	}
	this->basedirfd = openat(dirfd, "basedocs", O_DIRECTORY | O_RDONLY);
	if (!exists) {
		this->basetabfd = openat(basedirfd, "basetab", O_CREAT|O_TRUNC|O_RDWR, FILE_MODE);			
		ftruncate(this->basetabfd, sizeof(int) * 2);
	} else {
		this->basetabfd = openat(basedirfd, "basetab", O_RDWR, FILE_MODE);
	}

	fstat(this->basetabfd, &this->basetabst);
	this->nextfreebase = (int*) mmap(NULL, this->basetabst.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, this->basetabfd, 0);
	this->basetab = &this->nextfreebase[1];
}

int DDStore::add_document(const char *diffpath, const char *docpath) {
	// adds the file at path path to the store at path docpath
	// returns 0 on success, -1 otherwise
	struct stat docst, diffst, basest;
	char *doc, *base;
	char basepath[16];
	int docfd, basefd, difffd;
	int i;
	bool match;
	edit_t edits[EDITS_MAX + 1];
	diff_t diff;

	if (stat(docpath, &docst)){
		perror("could not access document");
		return 1;
	}

	if (faccessat(this->dirfd, diffpath, F_OK, 0) == 0) {
		dprintf(2, "cannot add document to store. Path already exists\n");
		return 1;
	}
	
	for (int i = 0; i < EDITS_MAX; i++) {
		edits[i].type = EDIT_NOOP;
		edits[i].offset = -1;
	}

	docfd = open(docpath, O_RDONLY);
	doc = (char*) mmap(NULL, docst.st_size, PROT_READ, MAP_SHARED, docfd, 0);
	close(docfd);

	match = false;
	for (i = 0; i < nextfreebase[1]; i++) {
		sprintf(basepath,"%d",i);
		if (faccessat(basedirfd, basepath, F_OK, 0) != 0)
			continue;
		
		basefd = openat(basedirfd, basepath, O_RDONLY);
		fstat(basefd, &basest);
		base = (char*) mmap(NULL, basest.st_size, PROT_READ, MAP_SHARED, basefd, 0);
		
		if (match = generate_edit_list(base, basest.st_size, doc, docst.st_size, edits)) {
			break;
		}
		
		close(basefd);
		munmap(base, basest.st_size);
	}

	if (!match) {
		i = this->add_base(doc, docst.st_size);
	}

	basetab[i] += 1;

	diff.base = i;
	diff.n = docst.st_size;
	memcpy(diff.edits, edits, sizeof(edit_t) * EDITS_MAX);
	
	difffd = openat(dirfd, docpath, O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
	write(difffd, &diff, sizeof(diff_t));
	close(difffd);
	
	munmap(doc, docst.st_size);

	return 0;
}

void *DDStore::get_document(int *n, const char *diffpath) {
	// retrieves file stored at path and places it into malloc'd buffer
	// stores size of buffer in n
	// 
	// returns address of buffer
	char *base, basepath[16], *doc;
	long i1, i2;
	struct stat basest;
	int basefd, difffd;
	edit_t *e;
	diff_t *diff;

	if ((difffd = openat(this->dirfd, diffpath, O_RDONLY)) == -1) {
		perror("could not open diff");
		return (void*) -1;
	}
	
	diff = (diff_t*) mmap(NULL, sizeof(diff_t), PROT_READ, MAP_SHARED, difffd, 0);
	
	sprintf(basepath, "%d", diff->base);
	if ((basefd = openat(this->basedirfd, basepath, O_RDONLY)) == -1) {
		perror("could not open basetab");
		return (void*) -1;
	}

	fstat(basefd, &basest);
	base = (char*) mmap(NULL, basest.st_size, PROT_READ, MAP_SHARED, basefd, 0);
	
	doc = (char*) malloc(diff->n);

	e = &diff->edits[0];
	i1 = i2 = 0;
	while (i1 < diff->n) {
		if (i2 == e->offset) {
			if (e->type == EDIT_MODIFY) {
				doc[i1] = e->c;
				i1++;
				i2++;
			}
			if (e->type == EDIT_DELETE) {
				i2++;
			}
			if (e->type == EDIT_INSERT) {
				doc[i1] = e->c;
				i1++;
			}

			if (e+1 < &diff->edits[EDITS_MAX])
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

	return (void*) doc;
}

int DDStore::delete_document(const char *diffpath) {
	diff_t *diff;
	int difffd;
	
	if (faccessat(this->dirfd, diffpath, F_OK, 0)) {
		dprintf(2, "can't remove diff: file does not exist");
		return 1;
	}

	difffd = openat(this->dirfd, diffpath, O_RDWR);
	diff = (diff_t*) mmap(NULL, sizeof(diff_t), PROT_READ | PROT_WRITE, MAP_SHARED, difffd, 0);
	
	basetab[diff->base] -= 1;
	if (basetab[diff->base] == 0) {
		this->delete_base(diff->base);		
	}
	
	munmap(diff,sizeof(diff_t));
	close(difffd);

	unlinkat(this->dirfd, diffpath, 0);

	return 0;
}
