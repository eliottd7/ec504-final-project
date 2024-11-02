#include "binfmt.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

class ddstore {
        ddstore(const char *path) {
 		this.path = malloc(strlen(path));
		stcpy(this.path, path);
		
		struct stat st;
		if (stat(path,&st) != 0){
			new_store();
		} else {
			fd = open(path, O_RDWR);
			hdr = mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		}
		
		if (hdr->s_ident != S_IDENT){
			printf("WARNING: s_ident does not match 0x%x",S_IDENT);
		}

		stfiletab = (void*)hdr + hdr->stfiletab;
		basetab = (void*)hdr + hdr->basetab;
		strtab = (void*)hdr + hdr->strtab;
	}
	
	int add_document(const char *path) {
		struct stat new_st;
		void *ptr;
		int fd;
		edit_t edits[EDITS_MAX];
		bool match;

		if (stat(path, &new_st)){
			perror("add_document: ");
			return -errno;
		}
		fd = open(path, O_RDONLY);
		ptr = (char*) mmap(NULL, new_st.st_size, PROT_READ, MAP_SHARED, 0);
		
		match = false;
		for (int i = 0; i<n_base; i++) {
			if (match = generate_edit_list(ptr, i, edits)) {
				break;
			}
		}
	}

	bool generate_edit_list(void *d1, int i0, edit_t *edits) {
		// edit distance!!!
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
	int sz_strtab;
	char *strtab;


	int new_store() {
		hdr = (ddstore_t*) malloc(sizeof(ddstore_t));
		
		hdr->s_ident = S_IDENT;
		
		int n = sizeof(ddstore_t);
		
		hdr->stfiletab = n;
		hdr->n_stfile = INIT_STFILE;
		n += sizeof(stfile_t) * INIT_STFILE;

		hdr->basetab = n;
		hdr->n_base = INIT_BASE;
		n += sizeof(base_t) * INIT_BASE;

		hdr->strtab = n;
		hdr->n_str = INIT_STRTAB;
		n += sizeof(char) * INIT_STRTAB;

		fd = open(path, O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
		ftruncate(fd,n);
		write(fd, hdr, sizeof(ddstore_t));
	}
}
