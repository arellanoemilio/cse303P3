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
				bitMap[0].freePages[byteLocation] = bitMap[0].freePages[byteLocation] | (0x01 << bitPosition);
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
				bitMap[1].freePages[byteLocation] = bitMap[1].freePages[byteLocation] | (0x01 << bitPosition);
			}
			else{
				bitPosition++;
			}
		}
	}
	return newPage;
}

int freeMemoryPage(struct free_memory_page *bitMap, int *pageNumber){
	int byteLocation = *pageNumber / 8;
	int bitPosition = *pageNumber % 8 + 1;
	if (*pageNumber < 4096){
		bitMap[0].freePages[byteLocation] = bitMap[0].freePages[byteLocation] ^ (0x01 << bitPosition);
	}
	else{
		bitMap[1].freePages[byteLocation] = bitMap[1].freePages[byteLocation - 512] ^ (0x01 << bitPosition);
	}
	return 1;
}

int traverseToDirectory(struct directory_page *currentDirectory, char *directoryName, struct loaded_pages *loadedPages){
	int pageNumber = -1;
	char *token = strtok(directoryName, "/");
	while(token != NULL){
		pageNumber = pageContainsDirectory(currentDirectory, token);
		if(pageNumber > 2){
			char *map = loadPage(loadedPages, pageNumber/8);
			if(map == MAP_FAILED){
				perror("mmap failed");
				return -1;
			}
			else{
				if(getIntFromCharArr(&map[512 *(pageNumber % 8) + 4]) == 1){
					if(loadDirectoryFromMap(currentDirectory, &map[512 * (pageNumber % 8)], loadedPages) == -1){
						pageNumber = -1;
					}
				}
			}
		}
		token = strtok(NULL,"/");
	}
	pageNumber = currentDirectory->filesLocations[0].location;
	return pageNumber;
}

int traverseToFileDirectory(struct directory_page *currentDirectory, char *directoryName, struct loaded_pages *loadedPages, int *filePage){
	int previousPageNumber = -1;
	int pageNumber = -1;
	int isFile = -1;
	char *token = strtok(directoryName, "/");
	while(token != NULL && isFile < 0){
		previousPageNumber = pageNumber;
		pageNumber = pageContainsDirectory(currentDirectory, token);
		if(pageNumber > 2){
			char *map = loadPage(loadedPages, pageNumber/8);
			if(map == MAP_FAILED){
				perror("mmap failed");
				return -1;
			}
			else{
				if(getIntFromCharArr(&map[512 *(pageNumber % 8) + 4]) == 1){
					if(loadDirectoryFromMap(currentDirectory, &map[512 * (pageNumber % 8)], loadedPages) == -1){
						pageNumber = -1;
					}
				}
				else if (getIntFromCharArr(&map[512 *(pageNumber % 8) + 4]) == 2){
					isFile = 1;
					*filePage = pageNumber;
				}
			}
		}
		token = strtok(NULL,"/");
	}

	if(token != NULL){
		pageNumber = -1;
	}

	return previousPageNumber;
}

int pageContainsDirectory(struct directory_page *current, char *folderName){
	int pageNumber = -1;
	for(int i = 0; i < current->numElements; i++){
		if(strcmp(folderName, current->filesLocations[i].name) == 0){
			pageNumber = current->filesLocations[i].location;
			i = current->numElements;
		}
	}
	return pageNumber;
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
		if (getIntFromCharArr(&map[4]) == 2){
			if (getIntFromCharArr(&map[8]) == 3){
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

int removeDirectory(struct directory_page *directory, char *directoryName, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage){
	struct directory_page *currentDirectory = (struct directory_page *)malloc(sizeof(struct directory_page));
	struct directory_page *parentDirectory = (struct directory_page *)malloc(sizeof(struct directory_page));
	directoryCopy(currentDirectory, directory);
	int currentDirectoryPage = traverseToDirectory(currentDirectory, directoryName, loadedPages);
	if(currentDirectoryPage != -1 && currentDirectory->numElements == 2){
		if(strcmp(currentDirectory->filesLocations[1].name,"..") == 0){
			int parentLocation = currentDirectory->filesLocations[1].location;
			char *map = loadPage(loadedPages, parentLocation/8);
			loadDirectoryFromMap(parentDirectory,&map[parentLocation % 8], loadedPages);
			for(int i = 0; i < parentDirectory->numElements; i++){
				if(parentDirectory->filesLocations[i].location == currentDirectoryPage){
					for(int j = i; j < parentDirectory->numElements - 1; j++){
						parentDirectory->filesLocations[j] = parentDirectory->filesLocations[j + 1];
					}
					free(parentDirectory->filesLocations[parentDirectory->numElements - 1].name);
					free(&parentDirectory->filesLocations[parentDirectory->numElements - 1]);
					parentDirectory->numElements--;
					mapDirectoryToMap(&map[parentLocation % 8], parentDirectory, loadedPages, bitMap, lastAllocatedPage);
					freeMemoryPage(bitMap, &currentDirectoryPage);
					updatePage(loadedPages, parentLocation / 8);
					updatePage(loadedPages, 0);
					i = parentDirectory->numElements;
					currentDirectory->empty = 0;
					currentDirectory->pageType = 0;
					map = loadPage(loadedPages, currentDirectoryPage / 8);
					mapDirectoryToMap(&map[currentDirectoryPage % 8], currentDirectory, loadedPages, bitMap, lastAllocatedPage);
					updatePage(loadedPages, currentDirectoryPage / 8);
				}
			}
		}
	}
	else{
		return -1;
	}
	return 1;
}

int removeFile(struct directory_page *directory, char *directoryName, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage){
	struct directory_page *currentDirectory = (struct directory_page *)malloc(sizeof(struct directory_page));
	directoryCopy(currentDirectory, directory);
	int filePage = -1;
	int currentDirectoryPage = traverseToFileDirectory(currentDirectory, directoryName, loadedPages, &filePage);
	if(currentDirectoryPage != -1 && filePage != -1){
		char *map = loadPage(loadedPages, currentDirectoryPage / 8);
		for(int i = 0; i < currentDirectory->numElements; i++){
			if(currentDirectory->filesLocations[i].location == filePage){
				for(int j = i; j < currentDirectory->numElements - 1; j++){
					currentDirectory->filesLocations[j] = currentDirectory->filesLocations[j + 1];
				}
				free(currentDirectory->filesLocations[currentDirectory->numElements - 1].name);
				free(&currentDirectory->filesLocations[currentDirectory->numElements - 1]);
				currentDirectory->numElements--;
				mapDirectoryToMap(&map[currentDirectoryPage % 8], currentDirectory, loadedPages, bitMap, lastAllocatedPage);
				freeMemoryPage(bitMap, &filePage);
				updatePage(loadedPages, currentDirectoryPage / 8);
				updatePage(loadedPages, 0);
				i = currentDirectory->numElements;
				currentDirectory->pageType = 0;
				map = loadPage(loadedPages,filePage / 8);
				writeIntToCharArr(&map[512 * (filePage % 8)], 0);
				writeIntToCharArr(&map[(512 * (filePage % 8)) + 4], 0);
				updatePage(loadedPages, currentDirectoryPage / 8);
			}
		}
	}
	else{
		return -1;
	}
	return 1;
}

int removeRecursively(struct directory_page *directory, char *directoryName, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage){
	struct directory_page *currentDirectory = (struct directory_page *)malloc(sizeof(struct directory_page));
	struct directory_page *parentDirectory = (struct directory_page *)malloc(sizeof(struct directory_page));
	directoryCopy(currentDirectory, directory);
	int currentDirectoryPage = traverseToDirectory(currentDirectory, directoryName, loadedPages);
	if(currentDirectoryPage != -1){
		if(strcmp(currentDirectory->filesLocations[1].name,"..") == 0){
			for(int i = currentDirectory->numElements - 1; i > 1; i++){
				if(removeDirectory(currentDirectory, currentDirectory->filesLocations[i].name, loadedPages,bitMap, lastAllocatedPage) != 1){
					return -1;
				}
			}

			int parentLocation = currentDirectory->filesLocations[1].location;
			char *map = loadPage(loadedPages, parentLocation/8);
			loadDirectoryFromMap(parentDirectory,&map[parentLocation % 8], loadedPages);
			for(int i = 0; i < parentDirectory->numElements; i++){
				if(parentDirectory->filesLocations[i].location == currentDirectoryPage){
					for(int j = i; j < parentDirectory->numElements - 1; j++){
						parentDirectory->filesLocations[j] = parentDirectory->filesLocations[j + 1];
					}
					free(parentDirectory->filesLocations[parentDirectory->numElements - 1].name);
					free(&parentDirectory->filesLocations[parentDirectory->numElements - 1]);
					parentDirectory->numElements--;
					mapDirectoryToMap(&map[parentLocation % 8], parentDirectory, loadedPages, bitMap, lastAllocatedPage);
					freeMemoryPage(bitMap, &currentDirectoryPage);
					updatePage(loadedPages, parentLocation / 8);
					updatePage(loadedPages, 0);
					i = parentDirectory->numElements;
					currentDirectory->empty = 0;
					currentDirectory->pageType = 0;
					map = loadPage(loadedPages, currentDirectoryPage / 8);
					mapDirectoryToMap(&map[currentDirectoryPage % 8], currentDirectory, loadedPages, bitMap, lastAllocatedPage);
					updatePage(loadedPages, currentDirectoryPage / 8);
				}
			}
		}
	}
	else{
		return removeFile(directory, directoryName, loadedPages, bitMap, lastAllocatedPage);
	}
	return 1;
}

int makeDirectory(struct directory_page *directory, char *newDirectory, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage){
	struct directory_page *newDirectoryPage = (struct directory_page *)malloc(sizeof(struct directory_page));
	newDirectoryPage->empty = 1;
	newDirectoryPage->pageType = 1;
	newDirectoryPage->numElements = 2;
	newDirectoryPage->nextDirectoryPage = -1;
	newDirectoryPage->filesLocations = (struct file_location *)malloc(2 * sizeof (struct file_location));
	newDirectoryPage->filesLocations[1].name = (char *)malloc(3 * sizeof(char));
	newDirectoryPage->filesLocations[1].name = "..";
	newDirectoryPage->filesLocations[1].location = directory->filesLocations[0].location;
	newDirectoryPage->filesLocations[0].name = (char *)malloc(2 * sizeof(char));
	newDirectoryPage->filesLocations[0].name = ".";
	int newPageNumber = findNewPage(bitMap, lastAllocatedPage);
	newDirectoryPage->filesLocations[0].location = newPageNumber;
	char *map = loadPage(loadedPages, newPageNumber / 8);
	mapDirectoryToMap(&map[512 * (newPageNumber % 8)], newDirectoryPage, loadedPages, bitMap, lastAllocatedPage);
	updatePage(loadedPages, newPageNumber / 8);
	updatePage(loadedPages, 0);
	directory->numElements++;
	directory->filesLocations = realloc(directory->filesLocations, directory->numElements * sizeof(struct file_location));
	directory->filesLocations[directory->numElements - 1].name = (char *)malloc((strlen(newDirectory) + 1) * sizeof(char));
	strcpy(directory->filesLocations[directory->numElements - 1].name, newDirectory);
	directory->filesLocations[directory->numElements - 1].location = newPageNumber;
	map = loadPage(loadedPages, directory->filesLocations[0].location / 8);
	mapDirectoryToMap(&map[512 * (directory->filesLocations[0].location % 8)], directory, loadedPages, bitMap, lastAllocatedPage);
	updatePage(loadedPages, directory->filesLocations[0].location / 8);
	return 1;
}

char * fileLocationsToCharArr(struct file_location *filesLocations, int numElements, int *numPages){
	int allocations = 1;
	char * map = (char *)malloc(496 * sizeof(char));
	int freeSpace = 496;
	int index = 0;
	for(int i = 0; i < numElements; i++){
		int strLength;
		strLength = strlen(filesLocations[i].name) + 1;
		if(freeSpace > strLength + 4){
			strcpy(&map[index], filesLocations[i].name);
			index += strLength;
			writeIntToCharArr(&map[index],filesLocations[i].location);
			index += 4;
		}
		else{
			allocations++;
			map = realloc(map, (496 * allocations)*sizeof(char));
			i--;
		}
	}

	*numPages = allocations;
	return map;
}

int mapDirectoryToMap(char *map, struct directory_page *directory, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage){
	int numPages = 1;
	int nextDirectoryPage = directory->nextDirectoryPage;
	int ghostPages = directory->nextDirectoryPage;
	char * files = fileLocationsToCharArr(directory->filesLocations, directory->numElements, &numPages);
	for(int i = 0; i < numPages; i++){
		if(i > 0){
			int nextPage = getIntFromCharArr(&map[12]);
			if(nextPage != -1){
				char *map2 = loadPage(loadedPages, nextPage / 8);
				map = &map2[512 * (nextPage % 8)];
				nextDirectoryPage = getIntFromCharArr(&map[12]);
				ghostPages = -1;
			}
			else{
				int nextPage = findNewPage(bitMap,lastAllocatedPage);
				char *map2 = loadPage(loadedPages, nextPage / 8);
				writeIntToCharArr(&map[12], nextPage);
				map = &map2[512 * (nextPage % 8)];
				nextDirectoryPage = -1;
				ghostPages = getIntFromCharArr(&map[12]);
			}
		}
		writeIntToCharArr(&map[0], directory->empty);
		writeIntToCharArr(&map[4], directory->pageType);
		writeIntToCharArr(&map[8], directory->numElements);
		writeIntToCharArr(&map[12], nextDirectoryPage);
		memcpy(&map[16],&files[i * 496],496);
	}
	writeIntToCharArr(&map[12], -1);

	while(ghostPages != -1){
		char *map2 = loadPage(loadedPages, ghostPages / 8);
		map = &map2[512 * (ghostPages % 8)];
		writeIntToCharArr(&map[0], 0);
		writeIntToCharArr(&map[4], 0);
		freeMemoryPage(bitMap, &ghostPages);
		ghostPages = getIntFromCharArr(&map[12]);
	}
	return 1;
}

int loadDirectoryFromMap(struct directory_page *directory, char *map, struct loaded_pages *loadedPages){
	directory->empty = getIntFromCharArr(&map[0]);
	directory->pageType = getIntFromCharArr(&map[4]);
	directory->numElements = getIntFromCharArr(&map[8]);
	directory->nextDirectoryPage = getIntFromCharArr(&map[12]);
	directory->filesLocations = (struct file_location *) malloc(directory->numElements * sizeof(struct file_location));
	int nextDirectoryPage = directory->nextDirectoryPage;
	int allocatedPages = 1;
	char *locationData = (char *)malloc(496 * sizeof(char));
	memcpy(locationData, &map[16], 496);
	while(nextDirectoryPage != -1){
		allocatedPages++;
		map = loadPage(loadedPages, nextDirectoryPage / 8);
		locationData = realloc(locationData, (496 * allocatedPages) * sizeof(char));
		memcpy(&locationData[496 * (allocatedPages -1)], &map[(512 * (nextDirectoryPage % 8)) + 16], 496);
		nextDirectoryPage = getIntFromCharArr(&map[(512 * (nextDirectoryPage % 8)) + 12]);
	}

	int index = 0;
	for(int i = 0; i < directory->numElements; i++){
		int strlength = strlen(&locationData[index]) + 1;
		directory->filesLocations[i].name = (char *)malloc(strlength * sizeof(char));
		strcpy(directory->filesLocations[i].name, &locationData[index]);
		index += strlength;
		directory->filesLocations[i].location = getIntFromCharArr(&locationData[index]);
		index += 4;
	}
	return 1;
}

void directoryCopy(struct directory_page *dest, const struct directory_page *source){
  	dest->empty = source->empty;
		dest->pageType = source->pageType;
		dest->numElements = source->numElements;
		dest->nextDirectoryPage = source->nextDirectoryPage;
		dest->filesLocations = (struct file_location *)malloc(source->numElements * sizeof(struct file_location));
		for(int i = 0; i < source->numElements; i++){
			int strLength = strlen(source->filesLocations[i].name) + 1;
			dest->filesLocations[i].name = (char *)malloc(strLength * sizeof(char));
			strcpy(dest->filesLocations[i].name, source->filesLocations[i].name);
			dest->filesLocations[i].location = source->filesLocations[i].location;
		}
}

int countSetBits(int n)
{
  unsigned int count = 0;
  while (n)
  {
    count += n & 1;
    n >>= 1;
  }
  return count;
}

void printWorkingDirectory(struct directory_page *directory){
	char * name = directory->filesLocations[0].name;
	printf("%s/\n", name);
}


void list(struct directory_page *directory, struct loaded_pages *loadedPages){
	int numOfElements = directory->numElements;
	for (int i = 0; i < numOfElements; i++){
		printf("%s",directory->filesLocations[i].name);
		int pOrD = getPageType(loadedPages, directory->filesLocations[i].location);
		if(pOrD == 1){
			printf("/");
		}
		printf("\n");
	}

}

int getPageType(struct loaded_pages *loadedPages, int pageNumber){
	char * map = loadPage(loadedPages, pageNumber/8);
	int page = getIntFromCharArr(&map[512 * (pageNumber % 8) + 4]);
	return page;
}

void writeFile(char* filename, int amt, char* data, struct directory_page *directory, struct loaded_pages *loadedPages){
	//check if it is in the directory
	//check if it is a page
	int numOfElements = directory->numElements;
	int isInDirectory = 1;
	int location = -1;
	for (int i = 0; i < numOfElements; i++){
		if(!strcmp(directory->filesLocations[i].name, filename)){
			isInDirectory = 0;
			location = directory->filesLocations[i].location;
		}
	}
	char * map = loadPage(loadedPages, location/8);
	char * loadedMap = &map[512 * (location % 8)];
	//is in the directory
	if(!isInDirectory){
		if(amt > 496){
			//allocate new page
		}
		else{
			strncpy(loadedMap, data, amt);
			updatePage(loadedPages, location/8);
		}
	}
	else{
		//not in directory;
	}
}
