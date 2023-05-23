#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "halde.h"

int main(int argc, char *argv[]) {
	
	// List at beginning
	printList();
	
	// First malloc
	char *m1 = malloc(200*1024);
	printList();

	// Second malloc
	char *m2 = malloc(100*1024);
	printList();

	// Third malloc
	char *m3 = malloc(250*1024);
	printList();

	// Fourth malloc
	char *m4 = malloc(100*1024);
	printList();
	
	// First free
	free(m1);
	printList();

	// Second free
	free(m2);
	printList();

	// Third free
	free(m3);
	printList();

	// Fourth free
	free(m4);
	printList();

	// Malloc one again
	m1 = malloc(100*1024);
	printList();
	
	// Malloc two again
	m2 = malloc(100*1024);
	printList();
	
	// Malloc three again
	m3 = malloc(100*1024);
	printList();
	
	// Malloc four again
	m4 = malloc(100*1024);
	printList();
	
	// Testing if malloc 0 returns NULL as edge case
	char* nullTest = malloc(0);
	if(nullTest != NULL){
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
