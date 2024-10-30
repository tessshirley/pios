#include "list.h"
//#include <stddef.h>
#include "rprintf.h"
#include "serial.h"
#include "page.h"
#include "mmu.h"
#include "fat.h"
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "sd.h"
#include "msec.h"
#include "uart.h"
#include "gpio.h"
#include "malloc.h"

#define SYSTEM_CLOCK_HZ 1000000 // for wait_ms()

extern struct table_descriptor_stage1 L1table[]; // make L1table visible

char glbl[128];

char buffer[512];

struct file *my_file;

unsigned long get_timer_count() {
     unsigned long *timer_count_register = (unsigned long *)0x3f003004;
     return *timer_count_register;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n-- > 0) {
        if (*s1 != *s2) {
            return *(unsigned char *)s1 - *(unsigned char *)s2;
        }
        if (*s1 == '\0') {
            break;
        }
        s1++;
        s2++;
    }
    return 0;
}

// wait_msec() moved to msec.c

void kernel_main() {
    get_timer_count();
    wait_msec(1000);
    extern int __bss_start, __bss_end;
    char *bssstart, *bssend;
   // esp_printf( my_putc, "Current Execution Level is %d\r\n", getEL());

    // zero out the bss segment
    bssstart = (char *)&__bss_start;
    bssend = (char *)&__bss_end;

         // FAT Tests //
    // initialize the SD card
    if(sd_init() != SD_OK) {
       esp_printf(my_putc, "Failed to initialize SD card\n");
        return -1;
    }

    // initialize the FAT filesystem
    if(fatInit() != 0) {
        esp_printf(my_putc, "Failed to initialize FAT filesystem\n");
        return -1;
    } else {
	esp_printf(my_putc, "Successfully initialized FAT filesystem\n");
    }
 
    // open the specified file
    const char *file_handle = "/mnt/disk/test";
    if(fatOpen(file_handle) == NULL){
        esp_printf(my_putc, "file not found\n");
    } else {
	esp_printf(my_putc, "file found!");
    }

    // read the contents of the file into the buffer
    uint8_t buffer[CLUSTER_SIZE];
    int bytes_read = fatRead(file_handle, buffer, sizeof(buffer));
    if(bytes_read < 0) {
         esp_printf(my_putc, "Failed to read file data\n");
        return -1;
    }

    // output the read data 
    esp_printf(my_putc, "Read %d bytes from %s:\n", bytes_read, file_handle);
    for(int i = 0; i < bytes_read; i++) {
         esp_printf(my_putc, "%c ", buffer[i]);
    }
    esp_printf(my_putc,"\n");

    /*
    while(bssstart < bssend) {
        *bssstart++ = 0;
     } 
   */

    /*
    // testing page mapping
    mapPages((void*)0x0, (void*)0x0);
    // load the L1 page table and enable the MMU
   // int result = loadPageTable(L1table);
    loadPageTable(L1table);

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
  */
}
