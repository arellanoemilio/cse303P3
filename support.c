#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

int findNewPage(struct free_memory_page *bitMap, int *lastAllocatedPage){
	int newPage = -1;
	while(newPage < 0){
		int byteLocation = lastAllocatedPage / 8;
		int bitPosition = lastAllocatedPage % 8 + 1;
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

//TODO: finish this edge case
nextPageSetup(int pageNumber, int *spaceAvailable, int fd){
	int offset = pagenumber / 8;
	char *map = (char *) mmap(NULL, 4096, PROT_WRITE, MAP_SHARED, fd, offset * 4096);
}

void writeDirectory(int * spaceAvailable, struct directory current, char *map,
struct free_memory_page *bitMap, int *lastAllocatedPage, int nextPage, int fd){
	int strLength = strlen(current.name);
	if(*spaceAvailable > strLength + 9){
		strcpy(map[0], current.name);
		*spaceAvailable  -= strLength+9;
		writeIntToCharArr(&map[strLength+1], current.isFile);
		writeIntToCharArr(&map[strLength+5], current.contents);
		if(current.isFile == 0 && current.contents > 0 ){
			for(int i = 0; i <  current.contents; i++){
				writeDirectory(spaceAvailable, current.children[i], map[strLength+9],
					bitMap, lastAllocatedPage, nextPage,fd);
			}
		}
	}
	else{
		int pageNumber;
		if(nextPage == -1){
			pageNumber = findNewPage(bitMap, lastAllocatedPage)
		}
		else{
			pageNumber = nextPage;
		}
		map = nextPageSetup(pageNumber, spaceAvailable, fd);
		strcpy(map[0], current.name);
		*spaceAvailable  -= strLength+9;
		writeIntToCharArr(&map[strLength+1], current.isFile);
		writeIntToCharArr(&map[strLength+5], current.contents);
		if(current.isFile == 0 && current.contents > 0 ){
			for(int i = 0; i <  current.contents; i++){
				writeDirectory(spaceAvailable, current.children[i], map[strLength+9],
					bitMap, lastAllocatedPage, nextPage,fd);
			}
		}
	}
}

void writeDirectoriesToMap(char * map, struct directory_page *rootDirectoryPage,
struct directory *rootDirectory, struct free_memory_page *bitMap,
int *lastAllocatedPage, int fd){
	int spaceAvailable = 496;
	struct directory current = *rootDirectory;
	rootDirectoryPage->numElements = 0;
	writeIntToCharArr(&map[0], rootDirectoryPage->emty);
	writeIntToCharArr(&map[4], rootDirectoryPage->pageType);
	writeIntToCharArr(&map[12], rootDirectoryPage->nextDirectoryPage);
	//order to save info
	//save the name
	//save the is file
	//if not is file save number of children
	//if is file save contents
	//total size = size of name + 4(is file) + 4 (contents/ number of children)
	writeDirectory(&spaceAvailable, current, &map[16], bitMap, lastAllocatedPage,
								rootDirectoryPage->nextDirectoryPage,fd);

}


int verify(char *filename){
	FILE *fp = fopen(filename, "r");
	if(fp ==NULL){
		return -1;
	}
	fclose(fp);

	int fileData = open(file, O_RDWR);
	char *map = (char *) mmap(NULL, 4096, PROT_READ, MAP_SHARED, fileData, 0);
	if(map == MAP_FAILED){
	  perror("mmap failed");
		return -1;
	}
	int good = map[0] & 0x01;
	if (!good){
		return -1;
	}
	good = map[4] & 0x02;
	if (!good){
		return -1;
	}
	good = map[8] & 0x03;
	if (!good){
		return -1;
	}
	return 1;
}
