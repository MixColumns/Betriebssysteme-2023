#include "halde.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// Magic value for occupied memory chunks.
#define MAGIC ((void*)0xbaadf00d)

/// Size of the heap (in bytes).
#define SIZE (1024*1024*1)

/// Memory-chunk structure.
struct mblock {
	struct mblock *next;
	size_t size;
	char memory[];
};

/// Heap-memory area.
static char memory[SIZE];

/// Pointer to the first element of the free-memory list.
static struct mblock *head;

/// Helper function to visualise the current state of the free-memory list.
void printList(void) {
	struct mblock *lauf = head;

	// Empty list
	if (head == NULL) {
		char empty[] = "(empty)\n";
		write(STDERR_FILENO, empty, sizeof(empty));
		return;
	}

	// Print each element in the list
	const char fmt_init[] = "(off: %7zu, size:: %7zu)";
	const char fmt_next[] = " --> (off: %7zu, size:: %7zu)";
	const char * fmt = fmt_init;
	char buffer[sizeof(fmt_next) + 2 * 7];

	while (lauf) {
		size_t n = snprintf(buffer, sizeof(buffer), fmt
			, (uintptr_t) lauf - (uintptr_t)memory, lauf->size);
		if (n) {
			write(STDERR_FILENO, buffer, n);
		}

		lauf = lauf->next;
		fmt = fmt_next;
	}
	write(STDERR_FILENO, "\n", 1);
}

// Value that indicates if the list was initialized yet so we know when
// we are actually out of memory.
static int init = 0;
void *malloc (size_t size) {
	
	// Edge Case size 0 should return NULL
	if(size <= 0){return NULL;}
	
	// If list is not initialized yet create the first list element
	if(!init){
		head = (struct mblock*) memory;
		head->size = sizeof(memory) - sizeof(struct mblock);
		head->next = NULL;
		init=1;
	}
	

	// The currently selected element and the element before that
	struct mblock* selected = head;
	struct mblock* previous = NULL;
	//fprintf(stderr, "head %p\n", (void*)head);
	//fprintf(stderr, "selected %p\n", (void*)selected);
	//fprintf(stderr, "previous %p\n", (void*)previous);
	
	// While no element is selected and as long as the size does not match
	// we keep searchin for a new element that is suitable
	while((selected != NULL) && (selected->size < size)){
		previous = selected;
		selected = selected->next;
		//fprintf(stderr, "selected %p\n", (void*)selected);
		//fprintf(stderr, "previous %p\n", (void*)previous);
	}
	
	//fprintf(stderr, "FINAL selected %p\n", (void*)selected);
	//fprintf(stderr, "FINAL previous %p\n", (void*)previous);
	
	// If there is no Element selected we are out of memory. There HAS to
	// be an element elsewise as we create a head that can store something
	// in the beginning and new List Elements as long as there is enough 
	// space left for a new element
	if(selected == NULL){
		errno = ENOMEM;
		return NULL;
	}
	
	// Check if remaining storage is big enough to create a new element
	if(selected->size - size <= sizeof(struct mblock)){
		// There is not enough space so now we check if there was an element
		// before the one that accessed the current one
		if(previous == NULL){
			// There was no previous element so we can set the head to the
			// elemen pointed to by the one we remove. If it was the only
			// one and points to NULL this is still correct as the list now
			// has no available elements
			head = selected->next;
		}else{
			// There is an element before the current element so we can now
			// set the value next for the previous element to the one of the
			// one we remove points to. This works as above and is correct for NULL.
			previous->next = selected->next;
		}
	}else{
		// Theres enough space to create a new Block and insert it
		struct mblock* nB = (struct mblock*) (selected->memory+size);
		nB->size = selected->size - size - sizeof(struct mblock);
		nB->next = selected->next;
		
		//fprintf(stderr, "new block at: nB %p\n", (void*)nB);
		//fprintf(stderr, "with nB->next %p\n", (void*)nB->next);
		
		// There is no previous element so the head now points to the
		// only available element (we came from the head so if its NULL
		// the head was our Element) which is the newly generated one
		if(previous == NULL){
			head = nB;
			//fprintf(stderr, "prev was null head now %p\n", (void*)head);	
		}else{
			// If there is a previous element it now points to the newly
			// generated block instead of the element we removed
			previous->next = nB;
			//fprintf(stderr, "prev now links to %p", (void*)previous->next);
		}
	}
	
	// The size of our memory Block is the requested size
	selected->size = size;
	// The next value becomes the magic value
	selected->next = MAGIC;
	// We return the requested memory
	return selected->memory;
		
}

void free (void *ptr) {
	// Nullpointer edge case where we just return
	if(ptr == NULL){
		return;
	}
	
	// Cast the given pointer to our structure
	struct mblock *oB = (struct mblock*) ptr - 1;
	//fprintf(stderr, "given oB at %p\n",(void*) oB);

	// If we cant find the magic value abort the pointer is not valid
	//fprintf(stderr, "oB->next %p\n", (void*) oB->next);
	if(oB->next != MAGIC){
		abort();
	}
	
	//fprintf(stderr, "head %p\n", (void*) head);
	
	// Insert the freed element at the beginning by pointing to the 
	// element that used to be the first one
	oB->next = head;
	//fprintf(stderr, "oB->next %p\n", (void*) oB->next);
	// and now pointing to that element instead
	head = oB;
	//fprintf(stderr, "head new %p\n", (void*) head);
}

void *realloc (void *ptr, size_t size) {
	// Edge case Nullpointer
	if(ptr == NULL){
		return malloc(size);
	}
	
	// Check if size is 0 if so act like free
	if(size == 0){
		free(ptr);
		return NULL;
	}
	
	// Malloc new space
	void* new_ptr = malloc(size);
	if(new_ptr == NULL){
		// Didnt work most likely ENOMEM
		errno = ENOMEM;
		return NULL;
	}
	
	// Cast the memory back to a struct to get the size
	struct mblock* tmp = (struct mblock*) ptr-1;
	// Memcopy from old to new with the size of old the rest is 
	// not initialized.
	new_ptr = memcpy(new_ptr, ptr, tmp->size);
	// Free the old pointer
	free(ptr);
	// Return the new pointer
	return new_ptr;
}

void *calloc (size_t nmemb, size_t size) {
	// According to manpage if either is null return null
	if(nmemb == 0 || size == 0){return NULL;}
	
	// Calculate total size
	size_t s = nmemb * size;
	
	// Overflow check according to manpage
	if(s / size != nmemb){
		errno = ENOMEM;
		return NULL;
	}
	
	// Malloc and Initialize
	void* ptr = malloc(s);
	
	// Check if it worked and either return the memset pointer or the
	// nullpointer with the error
	if(ptr == NULL){
		errno = ENOMEM;
		return NULL;
	}else{
		// Sets 0 to ptr for length s
		return memset(ptr, 0, s);
	}
}
