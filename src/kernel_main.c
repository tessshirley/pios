#include "list.h"
//#include <stdef.h>
#include "rprintf.h"
#include "serial.h"
#include "page.h"

char glbl[128];


unsigned long get_timer_count() {
     unsigned long *timer_count_register = 0x3f003004;
     return *timer_count_register;
}

void wait_1ms() {
    unsigned long start_time = get_timer_count();
    unsigned long target = 100;
    // compute the end time
    unsigned long end_time = start_time + target;
    // wait until the current time is past the end time
    while(get_timer_count() < end_time) {
    }
}

int getEL() {
   unsigned int el;
   asm("mrs %0,CurrentEL"
    : "=r"(el)
    :
    :);
   return el>>2;
}

int count_allocated_pages(struct ppage *list){
    int count = 0;
    struct ppage *current = list;
    while(current){
        count++;
	current = current->next;
    }
    return count;
}


void kernel_main() {
    get_timer_count();
    wait_1ms();
    extern int __bss_start, __bss_end;
    char *bssstart, *bssend;
    esp_printf( my_putc, "Current Execution Level is %d\r\n", getEL());

    // initialize the page fram allocator
    init_pfa_list();
    esp_printf(my_putc, "Page frame allocator initialized.\r\n");

    // allocate n pages for testing
    struct ppage *allocated_pages= allocate_physical_pages(128);
    if(allocated_pages) {
        esp_printf(my_putc, "Allocated %d physical pages.\r\n", count_allocated_pages(allocated_pages));
    } else {
	esp_printf(my_putc, "Failed to allocate pages.\r\n");
    }

    // free allocated pages
    free_physical_pages(allocated_pages);
    esp_printf(my_putc, "Freed allocated pages.\r\n");
   
    // allocate all available pages
    struct ppage *allocate_all = allocate_physical_pages(128);
    if(allocate_all){
        esp_printf(my_putc,"Allocated all 128 physical pages.\r\n");
    } else {
	esp_printf(my_putc,"Failed to allocate all pages.\r\n");
    }

    // free all
    free_physical_pages(allocate_all);
    esp_printf(my_putc, "Freed allocated pages.\r\n");

    // allocate more pages than available
    struct ppage *allocate_more = allocate_physical_pages(130);
    if(allocate_more){
        esp_printf(my_putc, "Unexpectedly allocated 130 pages.\r\n");
    } else {
	esp_printf(my_putc, "Correctly failed to allocate more pages than available.\r\n");
    }

    // zero out the bss segment
    bssstart = &__bss_start;
    bssend = &__bss_end;

    while(bssstart < bssend) {
	*bssstart++ = 0;
    }

    while(1){
    }

}
