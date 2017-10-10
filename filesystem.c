#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "support.h"
#include "structs.h"
#include "filesystem.h"
#include <fcntl.h>
#include <sys/mman.h>



/*
 * generateData() - Converts source from hex digits to
 * binary data. Returns allocated pointer to data
 * of size amt/2.
 */
char* generateData(char *source, size_t size)
{
	char *retval = (char *)malloc((size >> 1) * sizeof(char));

	size_t i;
	for(i=0; i<(size-1); i+=2)
	{
		sscanf(&source[i], "%2hhx", &retval[i>>1]);
	}
	return retval;
}

/*
 * Creates a file with the name file and initializes a filesytem in it
 */
int initializeFileSystem(char *file, struct loaded_pages *loadedPages){
	FILE *fp = fopen(file, "w");
	int i;
	for(i = 0; i < 131072; i++){
		fprintf(fp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	}
	fclose(fp);

	struct root_sector *rootSector =  (struct root_sector *) malloc(sizeof(struct root_sector));
	rootSector->directoryPages = 3;
	rootSector->freeMemoryPages = (int *) malloc(2 * sizeof(int));
	rootSector->freeMemoryPages[0] = 1;
	rootSector->freeMemoryPages[1] = 2;
	rootSector->lastAllocatedPage = 3;

	struct free_memory_page *freeMemoryPage = (struct free_memory_page *) malloc(2 * sizeof(struct free_memory_page));
	freeMemoryPage[0].freePages = (char *) malloc(512 * sizeof(char));
	freeMemoryPage[0].freePages[0] = 0xf0;
	freeMemoryPage[1].freePages = (char *) malloc(512 * sizeof(char));

	struct directory_page *rootDirectory = (struct directory_page *) malloc(sizeof(struct directory_page));
	rootDirectory->empty = 1;
	rootDirectory->pageType = 1;
	rootDirectory->numElements = 1;
	rootDirectory->nextDirectoryPage = -1;
	rootDirectory->filesLocations = (struct file_location *) malloc(sizeof(struct file_location));
	rootDirectory->filesLocations[0].name = ".";
	rootDirectory->filesLocations[0].location = 3;

	int fileData = open(file, O_RDWR);
	char *map = (char *) mmap(NULL, 4096, PROT_WRITE, MAP_SHARED, fileData, 0);
	if(map == MAP_FAILED){
	  perror("mmap failed");
		close(fileData);
		return -1;
	}

	//Mapping the Root sector
	writeIntToCharArr(&map[0],rootSector->freeMemoryPages[0]);
	writeIntToCharArr(&map[4],rootSector->freeMemoryPages[1]);
	writeIntToCharArr(&map[8],rootSector->directoryPages);
	writeIntToCharArr(&map[12],rootSector->lastAllocatedPage);

	//Coping the free bits
	memcpy(&map[512], freeMemoryPage[0].freePages, 512);
	memcpy(&map[512*2], freeMemoryPage[1].freePages, 512);

	//Mapping the Addresses
	mapDirectoryToMap(&map[512*3], rootDirectory, loadedPages, freeMemoryPage, &rootSector->lastAllocatedPage);

	if(msync(map, 4096, MS_SYNC) == -1){
		close(fileData);
		exit(EXIT_FAILURE);
	}
	if (munmap(map, 4096) == -1){
	    close(fileData);
	    exit(EXIT_FAILURE);
	}

	// Un-mmaping doesn't close the file, so we still need to do that.
  close(fileData);
	return 1;
}

/*
 * Inicializes a filesytem in it
 */
int readFileSystemFromFile(char *file, struct root_sector *rootSector, struct free_memory_page *bitMap,
													 struct directory_page *rootDirectory, struct loaded_pages *loadedPages){
	if(verify(file) == 1){
		printf("The file was verified\n");
  	loadedPages->fileData = open(file, O_RDWR);
		char *map = loadPage(loadedPages, 0);
		if(map == MAP_FAILED){
			return -1;
		}
		rootSector->freeMemoryPages = (int *) malloc(2 * sizeof(int));
		rootSector->directoryPages = getIntFromCharArr(&map[8]);
		rootSector->freeMemoryPages[0] = getIntFromCharArr(&map[0]);
		rootSector->freeMemoryPages[1] = getIntFromCharArr(&map[4]);
		rootSector->lastAllocatedPage = getIntFromCharArr(&map[12]);

		bitMap[0].freePages = &map[512];
		bitMap[1].freePages = &map[512 * 2];

		loadDirectoryFromMap(rootDirectory, &map[512 * 3], loadedPages);
	}
	else{
		printf("File was not verified please privide an appropirete filesystem.");
		return -1;
	}
	return 1;
}

/*
 * filesystem() - loads in the filesystem and accepts commands
 */
void filesystem(char *file)
{
	/* pointer to the memory-mapped filesystem */
	/* TODO: find if file exists or not, if not then create it
	 * open file, handle errors, create it if necessary.
	 * should end up with map referring to the filesystem.
	 */

	struct loaded_pages *loadedPages = (struct loaded_pages *)malloc(sizeof(struct loaded_pages));
 	loadedPages->numberOfLoadedPages = 0;
 	loadedPages->loadedPagesList = (int *)malloc(0);
 	loadedPages->pages = (char **)malloc(0);

	FILE * fp = fopen(file,"r");
	if(fp == NULL){
		initializeFileSystem(file, loadedPages);
	}

	struct root_sector *rootSector = (struct root_sector *)malloc(sizeof(struct root_sector));
	struct free_memory_page *bitMap = (struct free_memory_page *)malloc (2 * sizeof(struct free_memory_page));
	struct directory_page *rootDirectory = (struct directory_page *)malloc(sizeof(struct directory_page));
	struct directory_page *currentDirectory = (struct directory_page *)malloc(sizeof(struct directory_page));

	readFileSystemFromFile(file, rootSector, bitMap, rootDirectory, loadedPages);
	/* You will probably want other variables here for tracking purposes */
	directoryCopy(currentDirectory, rootDirectory);

	/*
	 * Accept commands, calling accessory functions unless
	 * user enters "quit"
	 * Commands will be well-formatted.
	 */
	char *buffer = NULL;
	size_t size = 0;
	while(getline(&buffer, &size, stdin) != -1)
	{
		/* Basic checks and newline removal */
		size_t length = strlen(buffer);
		if(length == 0)
		{
			continue;
		}
		if(buffer[length-1] == '\n')
		{
			buffer[length-1] = '\0';
		}

		/* TODO: Complete this function */
		/* You do not have to use the functions as commented (and probably can not)
		 *	They are notes for you on what you ultimately need to do.
		 */

		if(!strcmp(buffer, "quit"))
		{
			//freeLoadedMaps(loadedPages);
			break;
		}
		else if(!strncmp(buffer, "dump ", 5))
		{
			if(isdigit(buffer[5]))
			{
			  int pageNum = atoi(buffer + 5);
			  int pageOffset = pageNum / 8;

			  unsigned char *mapper = (unsigned char*) loadPage(loadedPages, pageOffset);
			  int internalOffet = pageNum % 8;
			  int startByte = internalOffet *512;

			  int j;
			  int counter = 0;
			  int spaceCounter = 0;
			  for (j=0; j < 512; j+=4)
			  {
			    int i;
			    for (i=0; i < 4; i++)
					{
				  	printf("%02x", mapper[i + startByte]);
				  	printf(" ");
					}
			    counter+=4;
			    spaceCounter +=4;
			    if((counter%32) == 0)
					{
				  	printf("%s\n", "");
				  	spaceCounter = 0;
					}
			    if(spaceCounter == 16)
					{
				  	printf("%s", "    ");
					}
			    startByte += 4;
				}
			}
			else
			{
				char *filename = buffer + 5;

        char *space = strstr(buffer+5, " ");
        *space = '\0';

        int pageNum = atoi(space + 1);
         int pageOffset = pageNum / 8;

         unsigned char *mapper = (unsigned char*) loadPage(loadedPages, pageOffset);
         int internalOffet = pageNum % 8;
         int startByte = internalOffet *512;

        FILE *file;
        file = fopen(filename, "w");
        if( file== NULL)
        {
        	printf("Cannot open file for writing\n");
        }

        else{
					int counter = 0;
				  int spaceCounter = 0;
					for (int j=0; j < 512; j+=4)
				  {
				    for (int i=0; i < 4; i++)
						{
					  	fprintf(file, "%02x", mapper[i + startByte]);
					  	fprintf(file, " ");
						}
				    counter+=4;
				    spaceCounter +=4;
				    if((counter%32) == 0)
						{
					  	fprintf(file, "%s\n", "");
					  	spaceCounter = 0;
						}
				    if(spaceCounter == 16)
						{
					  	fprintf(file, "%s", "    ");
						}
				    startByte += 4;
					}
        }
			}
		}
		else if(!strncmp(buffer, "usage", 5))
		{
			int i;
			int totalSetBits = 0;

			for(i = 0; i < 512; i+=4)
			{
		 		char *charArr = malloc(4);
				char *charArr2 = malloc(4);
				int j;
				int indexer = 0;
				for(j=i; j<(i+4); j++)
				{
					charArr[indexer] = bitMap[0].freePages[j];
					charArr2[indexer] = bitMap[1].freePages[j];
					indexer++;
				}
			 	int temp = getIntFromCharArr(charArr);
				int temp2 = getIntFromCharArr(charArr2);
				int num = countSetBits(temp);
				int num2 = countSetBits(temp2);
				totalSetBits = totalSetBits + num + num2;
			}
			int systemUsage = 512 * totalSetBits;
			printf("%s%d%s\n", "Space used by filesystem: ", systemUsage, " bytes");
	  	int numFiles = 0;

			countNumFiles(rootDirectory, 1, loadedPages, &numFiles);

			int fileUsage = numFiles * 512;
			printf("%s%d%s\n", "Space used by actual files: ", fileUsage, " bytes");
		}
		else if(!strncmp(buffer, "pwd", 3))
		{
		  printWorkingDirectory(currentDirectory, loadedPages);
			printf("\n");
		}
		else if(!strncmp(buffer, "cd ", 3))
		{
			if (traverseToDirectory(currentDirectory, buffer+3, loadedPages) == -1){
				printf("The specified directory was not found.\n");
			}
		}
		else if(!strncmp(buffer, "ls", 2))
		{
			list(currentDirectory, loadedPages);
		}
		else if(!strncmp(buffer, "mkdir ", 6))
		{
			makeDirectory(currentDirectory, buffer+6, loadedPages, bitMap, &rootSector->lastAllocatedPage);
			if(currentDirectory->filesLocations[0].location == rootDirectory->filesLocations[0].location){
				directoryCopy(rootDirectory,currentDirectory);
			}
		}
		else if(!strncmp(buffer, "cat ", 4))
		{
			cat(buffer + 4, currentDirectory, loadedPages);

		}
		else if(!strncmp(buffer, "write ", 6))
		{
			char *filename = buffer + 6;
			char *space = strstr(buffer+6, " ");
			*space = '\0';
			size_t amt = atoi(space + 1);
			space = strstr(space+1, " ");

			char *data = generateData(space+1, amt<<1);

			if(filename[0] != '/'){
				writeFile(filename, amt, data, currentDirectory, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, currentDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(currentDirectory, &temp[512 * (currentDirectory->filesLocations[0].location%8)],loadedPages);
				if(currentDirectory->filesLocations[0].location == rootDirectory->filesLocations[0].location){
					directoryCopy(rootDirectory,currentDirectory);
				}

			}
			else{
				writeFile(filename, amt, data, rootDirectory, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, rootDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(rootDirectory, &temp[512 * (rootDirectory->filesLocations[0].location%8)],loadedPages);
				if(currentDirectory->filesLocations[0].location == rootDirectory->filesLocations[0].location){
					directoryCopy(currentDirectory, rootDirectory);
				}
			}
			free(data);
		}
		else if(!strncmp(buffer, "append ", 7))
		{
			char *filename = buffer + 7;

			char *space = strstr(buffer+7, " ");
			*space = '\0';
			size_t amt = atoi(space + 1);
			space = strstr(space+1, " ");

			char *data = generateData(space+1, amt<<1);

			int success = -1;
			if(filename[0] != '/'){
				success = appendWriteFile(filename, amt, data, currentDirectory, loadedPages, bitMap, &rootSector->lastAllocatedPage);
			}
			else{
				success = appendWriteFile(filename, amt, data, rootDirectory, loadedPages, bitMap, &rootSector->lastAllocatedPage);
			}

			if(success < 0){
				printf("Unable to append inserted data to desired file.\n");
			}

			free(data);
		}
		else if(!strncmp(buffer, "remove ", 7))
		{
			char *filename = buffer + 7;
			char *space = strstr(buffer+7, " ");
			*space = '\0';
			size_t start = atoi(space + 1);

			space = strstr(space+1, " ");
			size_t end = atoi(space + 1);


			if(filename[0] != '/'){
				remover(filename, start, end, currentDirectory, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, currentDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(currentDirectory, &temp[512 * (currentDirectory->filesLocations[0].location%8)],loadedPages);
				if(currentDirectory->filesLocations[0].location == rootDirectory->filesLocations[0].location){
					directoryCopy(rootDirectory,currentDirectory);
				}

			}
			else{
				remover(filename, start, end, rootDirectory, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, rootDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(rootDirectory, &temp[512 * (rootDirectory->filesLocations[0].location%8)],loadedPages);
				if(currentDirectory->filesLocations[0].location == rootDirectory->filesLocations[0].location){
					directoryCopy(currentDirectory, rootDirectory);
				}
			}
		}
		else if(!strncmp(buffer, "getpages ", 9))
		{
			getPages(buffer + 9, currentDirectory, loadedPages);
		}
		else if(!strncmp(buffer, "get ", 4))
		{
			char *filename = buffer + 4;
      char *space = strstr(buffer+4, " ");
      *space = '\0';
    	size_t start = atoi(space + 1);
      space = strstr(space+1, " ");
      size_t end = atoi(space + 1);
      struct directory_page *temp = malloc(sizeof(struct directory_page));
      directoryCopy(temp, currentDirectory);
      int pageNum = -1;
      traverseToFileDirectory(temp, filename,loadedPages, &pageNum);
			if(pageNum != -1)
      {
      	get(pageNum, start, end, loadedPages);
      }
      else
      {
      	printf("%s\n", "Not a valid file");
      }
		}
		else if(!strncmp(buffer, "rmdir ", 6))
		{


			if(*(buffer+6) != '/'){
				removeDirectory(currentDirectory, buffer + 6, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, currentDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(currentDirectory, &temp[512 * (currentDirectory->filesLocations[0].location%8)],loadedPages);
				if(currentDirectory->filesLocations[0].location == rootDirectory->filesLocations[0].location){
					directoryCopy(rootDirectory,currentDirectory);
				}
			}
			else{
				removeDirectory(rootDirectory, buffer + 6, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, rootDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(rootDirectory, &temp[512 * (rootDirectory->filesLocations[0].location%8)], loadedPages);
			}

			rmdir(buffer + 6);
		}
		else if(!strncmp(buffer, "rm -rf ", 7))
		{
			if(*(buffer+6) != '/'){
				removeRecursively(currentDirectory, buffer + 7, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, currentDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(currentDirectory, &temp[512 * (currentDirectory->filesLocations[0].location%8)],loadedPages);
				if(currentDirectory->filesLocations[0].location == rootDirectory->filesLocations[0].location){
					directoryCopy(rootDirectory,currentDirectory);
				}
			}
			else{
				removeRecursively(rootDirectory, buffer + 7, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, rootDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(rootDirectory, &temp[512 * (rootDirectory->filesLocations[0].location%8)], loadedPages);
			}
		}
		else if(!strncmp(buffer, "rm ", 3))
		{
			if(*(buffer+6) != '/'){
				removeFile(currentDirectory, buffer + 3, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, currentDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(currentDirectory, &temp[512 * (currentDirectory->filesLocations[0].location%8)],loadedPages);
				if(currentDirectory->filesLocations[0].location == rootDirectory->filesLocations[0].location){
					directoryCopy(rootDirectory,currentDirectory);
				}
			}
			else{
				removeFile(rootDirectory, buffer + 3, loadedPages, bitMap, &rootSector->lastAllocatedPage);
				char *temp = loadPage(loadedPages, rootDirectory->filesLocations[0].location/8);
				loadDirectoryFromMap(rootDirectory, &temp[512 * (rootDirectory->filesLocations[0].location%8)], loadedPages);
			}
			//rm(buffer + 3);
		}
		else if(!strncmp(buffer, "scandisk", 8))
		{
			//scandisk();
		}
		else if(!strncmp(buffer, "undelete ", 9))
		{
			//undelete(buffer + 9);
		}

		updateRootSector(rootSector, loadedPages);

		free(buffer);
		buffer = NULL;
	}
	free(buffer);
	buffer = NULL;

}

/*
 * help() - Print a help message.
 */
void help(char *progname)
{
	printf("Usage: %s [FILE]...\n", progname);
	printf("Loads FILE as a filesystem. Creates FILE if it does not exist\n");
	exit(0);
}

/*
 * main() - The main routine parses arguments and dispatches to the
 * task-specific code.
 */
int main(int argc, char **argv)
{
	/* for getopt */
	long opt;

	/* run a student name check */
	check_student(argv[0]);

	/* parse the command-line options. For this program, we only support */
	/* the parameterless 'h' option, for getting help on program usage. */
	while((opt = getopt(argc, argv, "h")) != -1)
	{
		switch(opt)
		{
		case 'h':
			help(argv[0]);
			break;
		}
	}

	if(argv[1] == NULL)
	{
		fprintf(stderr, "No filename provided, try -h for help.\n");
		return 1;
	}

	filesystem(argv[1]);
	return 0;
}
