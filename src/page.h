#ifndef __PAGE_H__
#define __PAGE_H__
#include <stdarg.h>

struct ppage {
    struct ppage *next;
    struct ppage *prev;
    void *physical_addr;
};

// pointer to the head of the free physical page list
extern struct ppage *free_list;

// function declarations
void init_pfa_list(void);
struct ppage *allocate_physical_pages(unsigned int npages);
void free_physical_pages(struct ppage *ppage_list);

#endif
