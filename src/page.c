#include <stdio.h>
#include <stdlib.h>
#include "page.h"
#include <stdint.h>

// statically allocate an array of 128 pages 
#define PAGE_SIZE 0x200000 // 2MB per page
#define NUM_PAGES 128

struct ppage physical_page_array[NUM_PAGES]; // creates array with 128 elements

// pointer to the head of the free phsyical page list
struct ppage *free_list = NULL;

// initialize the list of free physical pages
void init_pfa_list(void) {
    for(int i = 0; i < NUM_PAGES; i++) {
        physical_page_array[i].next = (i == NUM_PAGES - 1) ? NULL : &physical_page_array[i+1];
	physical_page_array[i].prev = (i == 0) ? NULL : &physical_page_array[i-1];
	physical_page_array[i].physical_addr = (void *)((uintptr_t)(i * PAGE_SIZE)); 
    }
    // set the head of the free list to the first page
    free_list = &physical_page_array[0];
}

// allocate npages from the free list
struct ppage *allocate_physical_pages(unsigned int npages) {
    struct ppage *allocd_list = NULL;
    struct ppage *current = free_list;

    // check if there are enough free pages
    unsigned int i;
    for(i = 0; i < npages; i++) {
        if(!current) {
            // not enough free pages
	    return NULL;
	}
	current = current->next;
    }

    // unlink the requested pages from the free list
    allocd_list = free_list;
    unsigned int j; 
    for(j = 0; j < npages; j++) {
        if(free_list) {
            free_list = free_list->next;
	    if(free_list) {
		free_list->prev = NULL;
	    }
	}
    }
    return allocd_list;
}

// free physical pages by adding them back to the free list
void free_physical_pages(struct ppage *ppage_list) {
    if(!ppage_list){
        return;
    }

    // find the last page in the list to be freed
    struct ppage *last = ppage_list;
    while(last->next) {
         last = last->next;
    }
    
    // attach the list back to the front of the free list
    last->next = free_list;
    if(free_list) {
        free_list->prev = last;
    }

    free_list = ppage_list;
    free_list->prev = NULL;
}
