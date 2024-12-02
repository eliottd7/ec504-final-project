#include "ddstore.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
	int n;
	char *doc, *str;

	DDStore store = DDStore("test-store");
	store.add_document("hello.txt","hello.txt");
	store.add_document("hi.txt","hi.txt");	
	doc = (char*) store.get_document(&n, "hi.txt");
	str = (char*) malloc(n+1);
	strncpy(str, doc, n);
	str[n] = '\0';
	printf("%s",str);

	store.delete_document("hello.txt");
	store.delete_document("hi.txt");
	return 0;
}
