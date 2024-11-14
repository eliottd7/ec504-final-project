using namespace std;

#include "binfmt.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>


// de-duplicator store
class ddstore {
	public:
	ddstore(const char *path) {
		// either loads a ddstore from path
		// or creates a new one if path does not exist
 		this->path = (char*) malloc(strlen(path));
		strcpy(this->path, path);
		
		struct stat st;
		if (stat(path,&st) != 0){
			new_store();
		} else {
			fd = open(path, O_RDWR);
			hdr = (sthdr_t*) mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		}
		
		// the first 8 bytes of a ddstore are an identifier that must match our 
		// hard-coded constant
		if (hdr->s_ident != S_IDENT){
			printf("WARNING: s_ident does not match 0x%x",S_IDENT);
		}

		stfiletab = (stfile_t*) addrof(hdr->stfiletab);
		n_stfile = hdr->n_stfile;
		basetab = (base_t*) addrof(hdr->basetab);
		n_base = hdr->n_base;
	}
	
	int add_document(const char *path) {
		// adds the file at path as a storefile to our ddstore
		// returns 0 on success, -1 otherwise
		// 
		// incomplete atm
		struct stat new_st;
		char *d1;
		int fd;
		edit_t edits[EDITS_MAX];
		bool match;

		if (stat(path, &new_st)){
			printf("Could not access file at %s\n",path);
			return -1
		}
		fd = open(path, O_RDONLY);
		d1 = (char*) mmap(NULL, new_st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		
		match = false;
		for (int i = 0; i<n_base; i++) {
			if (match = generate_edit_list(d1, new_st.st_size, i, edits)) {
				break;
			}
		}
	}

	bool generate_edit_list(char *d1, long n1, int i_base, edit_t *edits) {
		// given a potentially very long string, generates a list of edits
		// between it and the base document specified by index i_base
		char *d0;
		long n0, i0, i1;
		int e, j0, j1;
		edit_t min_edits[MAX_EDIT];

		if (basetab[i_base].offset == 0) {
			return false;
		}

		d0 = (char*) addrof(basetab[i_base].offset);
		n0 = basetab[i_base].size;

		if  (EDITS_MAX < (n1 > n0 ? n1 - n0 : n0 - n1)) {
			return false;
		}
		
		e = EDITS_MAX;
		i0 = 0;
		i1 = 0;
		while (i0 < n0 && i1 < n1) {
			if (s0[i0] == s1[i1]) { // XXX I think this can be optimized if we cast s0 and s1 to longs and compare those
				i0++;
				i1++;
				continue;
			}

			j0 = i0 + e >= n0 ? n0 - i0 : e; 
			j1 = i1 + e >= n1 ? n1 - i1 : e; 
			min_edit_list(&min_edits, &s0[i0], &s1[i1], e);
		}
	}


	private:
	int fd; // file descriptor of the opened ddstore file
	sthdr_t *hdr; // store header. Points to the memory-mapped ddstore header
	char *path; // path to the ddstore file
	struct stat st; // stat of the ddstore file

	int n_stfile; // length of the storefile table
	stfile_t *stfiletab; // storefile table
	int n_base; // length of the basedoc table
	base_t *basetab; // base document table

	char *addrof(long offset) { // given an offset into the ddstore file, gives the address of it in the memory-mapped region
		return (char*)hdr + offset; 
	}

	int new_store() { // creates a new ddstore. Called when this->path does not exist
		hdr = (sthdr_t*) malloc(sizeof(sthdr_t));
		
		hdr->s_ident = S_IDENT;
		
		int n = sizeof(sthdr_t);
		
		hdr->stfiletab = n;
		hdr->n_stfile = INIT_STFILE;
		n += sizeof(stfile_t) * INIT_STFILE;

		hdr->basetab = n;
		hdr->n_base = INIT_BASE;
		n += sizeof(base_t) * INIT_BASE;

		fd = open(path, O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
		ftruncate(fd,n);
		write(fd, hdr, sizeof(sthdr_t));
	}

	
};
