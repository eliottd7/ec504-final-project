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



class ddstore {
	public:
	ddstore(const char *path) {
 		this->path = (char*) malloc(strlen(path));
		strcpy(this->path, path);
		
		struct stat st;
		if (stat(path,&st) != 0){
			new_store();
		} else {
			fd = open(path, O_RDWR);
			hdr = (sthdr_t*) mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		}
		
		if (hdr->s_ident != S_IDENT){
			printf("WARNING: s_ident does not match 0x%x",S_IDENT);
		}

		stfiletab = (stfile_t*) addrof(hdr->stfiletab);
		basetab = (base_t*) addrof(hdr->basetab);
	}
	
	int add_document(const char *path) {
		struct stat new_st;
		char *d1;
		int fd;
		edit_t edits[EDITS_MAX];
		bool match;

		if (stat(path, &new_st)){
			perror("add_document: ");
			return -errno;
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
	int fd;
	sthdr_t *hdr;
	char *path;
	struct stat st;

	int n_stfile;
	stfile_t *stfiletab;
	int n_base;
	base_t *basetab;

	char *addrof(long offset) {
		return (char*)hdr + offset;
	}

	int new_store() {
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
