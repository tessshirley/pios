#include "fat.h"
#include "sd.h"
#include "msec.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>

struct boot_sector bs;
struct file my_file;

// FAT initialization function
int fatInit() {
    uint8_t buffer[SECTOR_SIZE];

    // read the boot sector
    if(sd_readblock(0, buffer, 1) != SD_OK) {
        return -1; // error reading boot sector
    }

    // cast the buffer to the boot_sector struct
    memcpy(&bs, buffer, sizeof(struct boot_sector));

    // validate the boot signature and FAT type
    const char *bs_type = bs.fs_type;
    if(bs.boot_signature != 0xAA55 || strncmp(bs_type, "FAT12", 5) != 0) {
	    return -1; // invalid filesystem
    }

    return 0; // success!
}

// function to open a file
int fatOpen(const char *filename) {
    uint32_t root_dir_sector = bs.num_reserved_sectors + (bs.num_fat_tables + bs.num_sectors_per_fat);
    uint8_t buffer[SECTOR_SIZE];

    // iterate through root directory
    for(uint32_t sector = 0; sector < bs.num_root_dir_entries * sizeof(struct root_directory_entry) / SECTOR_SIZE; ++sector) {
        if(sd_readblock(root_dir_sector + sector, buffer, 1) != SD_OK) {
            return -1; // error reading root directory
	}
        struct root_directory_entry *rde = (struct root_directory_entry *) buffer;

	// loop through each entry
	for(int i = 0; i < SECTOR_SIZE / sizeof(struct root_directory_entry); ++i) {
	    if(strncmp(rde[i].file_name, filename, 8) == 0) {
		// found the file, store it in my_file structure
		my_file.rde = rde[i];
		my_file.start_cluster = rde[i].cluster;
		return 0; // file found
	     }
	}
    }
    return -1; // file not found
}

// fuunction to read data from an open file
int fatRead(struct file *file, void *buffer, uint32_t bytes_to_read){
    uint32_t current_cluster = file->start_cluster;
    uint8_t sector_buffer[SECTOR_SIZE];
    uint32_t bytes_read = 0;

    while(bytes_read < bytes_to_read) {
        // calculate the first sector of the current cluster
	uint32_t first_sector_of_cluster = bs.num_reserved_sectors + 
		(bs.num_fat_tables * bs.num_sectors_per_fat) + 
		((current_cluster - 2) * bs.num_sectors_per_cluster);

	for(int i = 0; i < SECTORS_PER_CLUSTER && bytes_read < bytes_to_read; ++i) {
	    if(sd_readblock(first_sector_of_cluster + i, sector_buffer, 1) != SD_OK) {
	       return -1; // error reading sector
	     }
	    memcpy(buffer +  bytes_read, sector_buffer, SECTOR_SIZE);
	    bytes_read += SECTOR_SIZE;
	}

	// move to the next cluster in the FAT chain
	uint32_t fat_sector = bs.num_reserved_sectors + (current_cluster / (SECTOR_SIZE / 2));
        uint32_t fat_offset = (current_cluster % (SECTOR_SIZE / 2)) * 2;
        uint8_t fat_buffer[SECTOR_SIZE];

        if(sd_readblock(fat_sector, fat_buffer, 1) != SD_OK) {
            return -1; // error reading FAT
	}
	current_cluster = (fat_buffer[fat_offset] | (fat_buffer[fat_offset + 1] << 8)) & 0x0FFF; // Adjust for FAT12
	
    }
    return bytes_read;
}
int main() {
    uint8_t buffer[SECTOR_SIZE]; // buffer to hold file data
    const char *filename = "TESS   ";

    // initialize the sd card
    if(sd_init() != SD_OK) {
        printf("SD card initialization failed\n");
	return -1;
    }

    // initialize the FAT filesystem
    if(fatInit() != 0) {
        printf("Failed to initialize FAT filesystem\n");
	return -1;
    }

    // open the specified file
    if(fatOpen(filename) != 0) {
	printf("Failed to open file: %s\n", filename);
	return -1;
    }

    // read the contents of the file into the buffer
    int bytes_to_read = SECTOR_SIZE;
    int bytes_read = fatRead(&my_file, buffer, bytes_to_read);
    if(bytes_read < 0) {
	printf("Failed to read file data\n");
	return -1;
    }

    // output the read data 
    printf("Read %d bytes from %s:\n", bytes_read, filename);
    for(int i = 0; i < bytes_read; i++) {
	 printf("%02x ", buffer[i]);
    }
    printf("\n");

    return 0;
}

