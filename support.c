#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "support.h"

/*
 * Make sure that the student name and email fields are not empty.
 */
void check_student(char * progname)
{
	if((strcmp("", student.name) == 0) || (strcmp("", student.email) == 0))
	{
		printf("%s: Please fill in the student struct in student.c\n", progname);
		exit(1);
	}
	printf("Student: %s\n", student.name);
	printf("Email  : %s\n", student.email);
	printf("\n");
}

void writeIntToCharArr(char * charPointer, int integer){
	charPointer[0] = integer & 0x000000ff;
	charPointer[1] = (integer >> 8) & 0x000000ff;
	charPointer[2] = (integer >> 16) & 0x000000ff;
	charPointer[3] = (integer >> 24) & 0x000000ff;
}

int getIntFromCharArr(char *charPointer){
	int i = 0;
	i = ((charPointer[3] & 0xff) << 24) | i ;
	i = ((charPointer[2] & 0xff) << 16) | i;
	i = ((charPointer[1] & 0xff) << 8) | i;
	i = (charPointer[0] & 0xff) | i;
	return i;
}

int findNewPage(struct free_memory_page *bitMap, int *lastAllocatedPage){
	int newPage = -1;
	while(newPage < 0){
		int byteLocation = *lastAllocatedPage / 8;
		int bitPosition = *lastAllocatedPage % 8 + 1;
		if (*lastAllocatedPage < 4096){
			if(bitPosition == 8){
				bitPosition = 0;
				byteLocation++;
			}
			int available = bitMap[0].freePages[byteLocation] ^ (0x01 << bitPosition);
			if(available){
				newPage = (8 * byteLocation) + bitPosition;
				*lastAllocatedPage = newPage;
			}
			else{
				bitPosition++;
			}
		}
		else{
			if(bitPosition == 8){
				bitPosition = 0;
				byteLocation++;
			}
			int available = bitMap[1].freePages[byteLocation - 512] ^ (0x01 << bitPosition);
			if(available){
				newPage = (8 * byteLocation) + bitPosition;
				*lastAllocatedPage = newPage;
			}
			else{
				bitPosition++;
			}
		}
	}
	return newPage;
}

int verify(char *filename){
	FILE *fp = fopen(filename, "r");
	if(fp == NULL){
		return -1;
	}
	fclose(fp);

	int fileData = open(filename, O_RDWR);
	char *map = (char *) mmap(NULL, 4096, PROT_READ, MAP_SHARED, fileData, 0);
	if(map == MAP_FAILED){
	  perror("mmap failed");
		close(fileData);
		return -1;
	}
	if (getIntFromCharArr(&map[0]) == 1){
		if (getIntFromCharArr(&map[0]) == 2){
			if (getIntFromCharArr(&map[0]) == 3){
				close(fileData);
				return 1;
			}
		}
	}
	close(fileData);
	return -1;
}

char * loadPage(struct loaded_pages *loadedPages, int pageOffset){
	char * map;
	int index = -1;
	for (int i = 0; i < loadedPages->numberOfLoadedPages; i++){
		if(loadedPages->loadedPagesList[i] == pageOffset){
			index = i;
			i = loadedPages->numberOfLoadedPages;
		}
	}
	if(index != -1){
		map = loadedPages->pages[index];
	}
	else{
		map = (char *) mmap(NULL, 4096, PROT_WRITE, MAP_SHARED, loadedPages->fileData, 4096 *pageOffset);
		loadedPages->numberOfLoadedPages++;
		loadedPages->loadedPagesList = realloc(loadedPages->loadedPagesList, loadedPages->numberOfLoadedPages * sizeof(int));
		loadedPages->pages = realloc(loadedPages->pages, loadedPages->numberOfLoadedPages * sizeof(char *));
		loadedPages->loadedPagesList[loadedPages->numberOfLoadedPages -1] = pageOffset;
		loadedPages->pages[loadedPages->numberOfLoadedPages -1] = map;
	}

	return map;
}

int updatePage(struct loaded_pages *loadedPages, int pageOffset){
	int index = -1;
	for (int i = 0; i < loadedPages->numberOfLoadedPages; i++){
		if(loadedPages->loadedPagesList[i] == pageOffset){
			index = i;
			i = loadedPages->numberOfLoadedPages;
		}
	}
	if(index != -1){
		char *map = loadedPages->pages[index];
		index = msync(map, 4096, MS_SYNC);
	}

	return index;
}
