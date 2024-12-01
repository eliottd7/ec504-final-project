#include "ddstore.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int dist(char *s1, int n1, char *s2, int n2, int **dp, dedit_t** edits) {
    int opts[3];
    int d;
    dedit_t *e, *prev;
    EditType type;

    if (n1 == 0 && n2 == 0) {
        return 0;
    }

    if (n1 == 0) {
        for (int i = 0; i < n2; i++) {
            e = &edits[n1][i+1];
            e->i = n1;
            e->type = EDIT_INSERT;
            e->c = s2[i];
            e->prev = &edits[n1][i-1];
        }
        return n2;
    }

    if (n2 == 0) {
        for (int i = 0; i < n1; i++) {
            e = &edits[i+1][n2];
            e->i = i;
            e->type = EDIT_DELETE;
            e->c = '\0';
            e->prev = &edits[i-1][n2];
        }
        return n1; 
    }

    if (dp[n1][n2] != -1) {
        return dp[n1][n2];
    }

    opts[0] = (s1[n1-1] != s2[n2-1]) + dist(s1,n1-1,s2,n2-1,dp,edits); // modify character. Do not apply cost if characters are same
    opts[1] = 1 + dist(s1,n1-1,s2,n2,dp,edits); // delete character
    opts[2] = 1 + dist(s1,n1,s2,n2-1,dp,edits); // insert character

    if (opts[0] < opts[1] && opts[0] < opts[2]) {
        d = opts[0];
        type = s1[n1-1] == s2[n2-1] ? EDIT_NOOP : EDIT_MODIFY;
        prev = &edits[n1-1][n2-1];
    } else if (opts[1] < opts[2]){
        d = opts[1];
        type = EDIT_DELETE;
        prev = &edits[n1-1][n2];
    } else {
        d = opts[2];
        type = EDIT_INSERT;
        prev = &edits[n1][n2-1];
    }

    e = &edits[n1][n2];
    e->i = n1-1;
    e->type = type;
    e->c = s2[n2-1];
    e->prev = prev;

    dp[n1][n2] = d;
    return dp[n1][n2];
}

int edit_list(edit_t* edits, char *s1, int n1, char *s2, int n2) {
    // produces list of character edits to transform s1 to s2

    int **dp = (int**) malloc(sizeof(int*) * (n1 + 1));
    dedit_t **dedits = (dedit_t**) malloc(sizeof(dedit_t*) * (n1 + 1));
    for (int i = 0; i < n1 + 1; i++) {
        dp[i] = (int*) malloc(sizeof(int) * (n2 + 1));
        dedits[i] = (dedit_t*) malloc(sizeof(dedit_t) * (n2 + 1));
    }

    for (int i1 = 0; i1 < n1 + 1; i1++) {
        for (int i2 = 0; i2 < n2 + 1; i2++) {
            dp[i1][i2] = -1;
	    dedits[i1][i2].i = -1;
        }
    }

    int d = dist(s1,n1,s2,n2,dp,dedits);

    dedit_t *e = &dedits[n1][n2];
    int i = d-1;
    while (e->i != -1) {
	if (e->type != EDIT_NOOP) {
		edits[i].type = e->type;
		edits[i].offset = e->i;
		edits[i].c = e->c;
		i--;
	}
	e = e->prev;
    }
    return d;
}
