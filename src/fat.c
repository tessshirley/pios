#include "fat.h"
#include "sd.h"
#include "msec.h"
#include "uart.h"
#include "rprintf.h"
#include "serial.h"
#include "mmu.h"
#include "gpio.h"
#include "malloc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SECTOR_SIZE 512

// global boot sector
struct boot_sector *bs;
char bootSector[512];
char fat_table[8 * SECTOR_SIZE];
unsigned int root_sector;


// FAT initialization function
int fatInit() {
    sd_init();

    uint8_t buffer[SECTOR_SIZE];
    bs = (struct boot_sector *)bootSector;

    // read the boot sector
    if(sd_readblock(0, buffer, 1) != SECTOR_SIZE) {
        return -1; // error reading boot sector
    }

    // cast the buffer to the boot_sector struct
    memcpy(bs, buffer, sizeof(struct boot_sector));

    // validate the boot signature and FAT type
    const char *bs_type = bs->fs_type;
    if(bs->boot_signature != 0xAA55) {
         return -1; // invalid filesystem
    }

    if(strncmp(bs_type, "FAT12", 5) != 0) {
        return -1; // unsupported
    }
    
    // read FAT table from SD card int fat_table 
    int fat_start = bs->num_reserved_sectors;
    for(int i =0; i < bs->num_sectors_per_fat; ++i) {
         sd_readblock(fat_start + i, &fat_table[i * SECTOR_SIZE], 1);
    }

    // compute root sector
    root_sector = bs->num_fat_tables + bs->num_sectors_per_fat + bs->num_reserved_sectors + bs->num_hidden_sectors;
     
    return 0; // success!
}

// function to open a file
struct file *fatOpen(const char *filename) {
    uint32_t root_dir_sector = bs->num_reserved_sectors + (bs->num_fat_tables * bs->num_sectors_per_fat);
    uint8_t buffer[SECTOR_SIZE];

    // iterate through root directory
    for(uint32_t sector = 0; sector < bs->num_root_dir_entries * sizeof(struct root_directory_entry) / SECTOR_SIZE; ++sector) {
        if(sd_readblock(root_dir_sector + sector, buffer, 1) != SD_OK) {
            return NULL; // error reading root directory
	}
        struct root_directory_entry *rde = (struct root_directory_entry *) buffer;

	// loop through each entry
	for(int i = 0; i < SECTOR_SIZE / sizeof(struct root_directory_entry); ++i) {
	    if(strncmp(rde[i].file_name, filename, 8) == 0) {

		// create a new file structure to hold file information
                struct file *my_file = (struct file *)malloc(sizeof(struct file));
                if(my_file == NULL) {
                    return NULL;
		}

		// found the file, store it in my_file structure
		my_file->rde = rde[i];
		my_file->start_cluster = rde[i].cluster;
		return my_file; // file found
	     }
	}
    }
    return NULL; // file not found
}

// fuunction to read data from an open file
int fatRead(struct file *file, void *buffer, uint32_t bytes_to_read){
    uint32_t current_cluster = file->start_cluster;
    uint8_t sector_buffer[SECTOR_SIZE];
    uint32_t bytes_read = 0;

    while(bytes_read < bytes_to_read) {
        // calculate the first sector of the current cluster
	uint32_t first_sector_of_cluster = root_sector + ((current_cluster - 2) * bs->num_sectors_per_cluster);

	for(int i = 0; i < bs->num_sectors_per_cluster && bytes_read < bytes_to_read; ++i) {
	    if(sd_readblock(first_sector_of_cluster + i, sector_buffer, 1) != SD_OK) {
	       return -1; // error reading sector
	     }

	    uint32_t remaining_bytes = bytes_to_read - bytes_read;
	    uint32_t bytes_to_copy = (remaining_bytes < SECTOR_SIZE) ? remaining_bytes : SECTOR_SIZE;

	    memcpy((char *)buffer + bytes_read, sector_buffer, bytes_to_copy);
	    bytes_read += bytes_to_copy;
	}

	// move to the next cluster in the FAT chain
	uint32_t fat_sector = bs->num_reserved_sectors + (current_cluster *3 / 2) / SECTOR_SIZE;
        uint32_t fat_offset = (current_cluster * 3 / 2) % SECTOR_SIZE;
	uint8_t fat_buffer[SECTOR_SIZE];

        if(sd_readblock(fat_sector, fat_buffer, 1) != SD_OK) {
            return -1; // error reading FAT
	}
	if(current_cluster & 0x0001) {
	    current_cluster = ((fat_buffer[fat_offset] >> 4) | (fat_buffer[fat_offset + 1] << 4)) & 0x0FF;
	} else {
	    current_cluster = ((fat_buffer[fat_offset] | (fat_buffer[fat_offset + 1] & 0x0F) << 8)) & 0x0FFF;
	}	
    }
    return bytes_read;
}

