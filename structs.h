#ifndef STRUCTS_H
#define STRUCTS_H
#include"structs.c"
/*
 *
 * Define page/sector structures here as well as utility structures
 * such as directory entries.
 *
 * Sectors/Pages are 512 bytes
 * The filesystem is 4 megabytes in size.
 * You will have 8K pages total.
 *
 */
  struct root_sector;
  struct free_memory_page;
  struct file_location;
  struct loaded_pages;
  struct directory_page;
  struct data_page;

#endif
