#include "ddstore.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int generate_edit_list(char *s1, long n1, char *s2, long n2, edit_t *edits) {
	// given two potentially very long strings, produces a list of edits
	// to transform d1 to d2
	// 
	// returns 0 if it would take more than EDITS_MAX edits
	// to perform transformation
	
	long i1, i2;
	int e, r, j1, j2, d, b, i0, e1, d1, d2;

	if (EDITS_MAX < (n1 > n2 ? n1 - n2 : n2 - n1)) {
		return false;
	}
	
	e = 0;
	r = EDITS_MAX; // remaining edits available
	i1 = 0;
	i2 = 0;
	while (i1 < n1 || i2 < n2) {
		if (s1[i1] == s2[i2]) { // XXX this could be optimized with vectorized comparisons
			i1++;
			i2++;
		} else {
			b = 2 * r + 1;
			j1 = b > n1 - i1 ? n1 - i1 : b;
			j2 = b > n2 - i2 ? n2 - i2 : b;
			d = edit_list(&edits[e], &s1[i1], j1, &s2[i2], j2);
			
			// remove edits that are not contiguous with the first character
			e1 = e+d;
			d1 = d2 = edits[e].offset;
			for (edit_t *edit = &edits[e]; edit < &edits[e1]; edit++) {
				if (d1 == edit->offset) {	
					edit->offset += i1;
					if (edit->type != EDIT_INSERT) {
						d1 += 1;
					}
					if (edit->type != EDIT_DELETE) {
						d2 += 1;
					}
				} else {
					d--;
					edit->type = EDIT_NOOP;
				}
			}
			
			e += d;
			r -= d;

			if (r < 0)
				return false;
			
			i1 += d1;
			i2 += d2;
		}
	}
	return true;
}

int dist(char *s1, int i1, char *s2, int i2, dedit_t** edits) {
    int opts[3];
    int cost;
    dedit_t *e, *next;
    EditType type;

    if (edits[i1][i2].cost != -1) {
        return edits[i1][i2].cost;
    }

    opts[0] = (s1[i1] != s2[i2]) + dist(s1,i1+1,s2,i2+1,edits); // modify character. Do not apply cost if characters are same
    opts[1] = 1 + dist(s1,i1+1,s2,i2,edits); // delete character
    opts[2] = 1 + dist(s1,i1,s2,i2+1,edits); // insert character

    if (opts[0] < opts[1] && opts[0] < opts[2]) {
        cost = opts[0];
        type = s1[i1] == s2[i2] ? EDIT_NOOP : EDIT_MODIFY;
        next = &edits[i1+1][i2+1];
    } else if (opts[1] < opts[2]){
        cost = opts[1];
        type = EDIT_DELETE;
        next = &edits[i1+1][i2];
    } else {
        cost = opts[2];
        type = EDIT_INSERT;
        next = &edits[i1][i2+1];
    }

    e = &edits[i1][i2];
    e->i = i1;
    e->type = type;
    e->c = s2[i2];
    e->next = next;
	e->cost = cost;

    return e->cost;
}

int edit_list(edit_t* edits, char *s1, int n1, char *s2, int n2) {
    // produces list of character edits to transform s1 to s2

    dedit_t **dedits = (dedit_t**) malloc(sizeof(dedit_t*) * (n1 + 1));
    for (int i = 0; i < n1 + 1; i++) {
        dedits[i] = (dedit_t*) malloc(sizeof(dedit_t) * (n2 + 1));
    }

	dedits[n1][n2].cost = 0;
	dedits[n2][n2].type = EDIT_NOOP;
	dedits[n1][n2].next = (dedit_t*) 0x0;
	dedits[n1][n2].i = -1;

	for (int i1 = 0; i1 < n1; i1++) {
		dedits[i1][n2].cost = n1-i1;
		dedits[i1][n2].type = EDIT_DELETE;
		dedits[i1][n2].next = &dedits[i1+1][n2];
		dedits[i1][n2].i = i1;
	}

	for (int i2 = 0; i2 < n2; i2++) {
		dedits[n1][i2].cost = n2-i2;
		dedits[n1][i2].c = s2[i2];
		dedits[n1][i2].type = EDIT_INSERT;
		dedits[n1][i2].next = &dedits[n1][i2+1];
		dedits[n1][i2].i = n1;
	}

    for (int i1 = 0; i1 < n1; i1++) {
   		for (int i2 = 0; i2 < n2; i2++) {
			dedits[i1][i2].cost = -1;
		}
    }

    int cost = dist(s1,0,s2,0,dedits);

    dedit_t *e = &dedits[0][0];
    int i = 0;
    while (i < cost) {
		if (e->type != EDIT_NOOP) {
			edits[i].type = e->type;
			edits[i].offset = e->i;
			edits[i].c = e->c;
			i++;
		}
		e = e->next;
    }
    return cost;
}

#ifdef CLI
char noop[] = "noop";
char modify[]="modify";
char del[] = "delete"; 
char insert[]="insert";

void print_edit_list(edit_t *edits, int n) {
	char *ident;

    for (edit_t *e = edits; e < &edits[n]; e++) {
		switch (e->type) {
			case EDIT_NOOP:
				continue;
			case EDIT_MODIFY:
				ident=modify;
				break;
			case EDIT_DELETE:
				ident=del;
				break;
			case EDIT_INSERT:
				ident=insert;
				break;
		}
		printf("(%s, %c, %d)\n",ident,e->c,e->offset);
	}
}

int main(int argc, char** argv) {
	char *s1, *s2;
	long n1, n2, n;

	if (argc != 3) {
		dprintf(2, "usage: %s [s1] [s2]\n", argv[0]);
		return 1;
	}

	s1 = argv[1];
	s2 = argv[2];
	
	n1 = strlen(s1);
	n2 = strlen(s2);
    
    n = n1 > n2 ? n1 : n2;

	edit_t edits[EDITS_MAX];
    edit_t dedits[n];

	for (edit_t *e=edits; e < &edits[EDITS_MAX]; e++) {
		e->type = EDIT_NOOP;
	}
    
    for (edit_t *e=dedits; e < &dedits[n]; e++) {
        e->type = EDIT_NOOP;
    }

    edit_list(dedits,s1,n1,s2,n2);
    printf("edits from base algorithm:\n");
    print_edit_list(dedits,n);
    printf("\n");

    printf("edits from optimized algorithm:\n");
	if (!generate_edit_list(s1,n1,s2,n2,edits)) {
		printf("transformation exceeded max edit distance\n");
		return 1;
	}
    print_edit_list(edits,EDITS_MAX);
}
#endif
