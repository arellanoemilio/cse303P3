#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/mman.h>
#include "support.h"
#include "structs.h"
#include "filesystem.h"
#include <fcntl.h>

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

void initializeFileSystem(char *file){
	FILE *fp = fopen(file, "w");
	int i;
	for(i = 0; i < 512; i++){
		fprintf(fp, "0");
	}
	fclose(fp);
	//fp = fopen(file, "w");
	struct root_sector *rootSector =  (struct root_sector *) malloc(sizeof(struct root_sector));
	rootSector->directoryPages = 3;
	rootSector->freeMemoryPages = (int *) malloc(2 * sizeof(int));
	rootSector->freeMemoryPages[0] = 1;
	rootSector->freeMemoryPages[1] = 2;
	rootSector->lastAllocatedPage = 3;
	printf("created rootSector\n");
	struct free_memory_page *freeMemoryPage = (struct free_memory_page *) malloc(2 * sizeof(struct free_memory_page));
	freeMemoryPage[0].freePages = (char *) malloc(512 * sizeof(char));
	freeMemoryPage[0].freePages[1] = 0xf0;
	freeMemoryPage[1].freePages = (char *) malloc(512 * sizeof(char));

	printf("created freePages\n");
	struct directory_page *rootDirectoryPage = (struct directory_page *) malloc(sizeof(struct directory_page));
	rootDirectoryPage->empty = 1;
	rootDirectoryPage->pageType = 1;
	rootDirectoryPage->numElements = 1;
	rootDirectoryPage->nextDirectoryPage = -1;

	printf("created directoryPage\n");
	struct directory *rootDirectory = (struct directory *)malloc(sizeof(struct directory));
	rootDirectory->parentDirectory = NULL;
	rootDirectory->name = "/";
	rootDirectory->isFile = 0;
	rootDirectory->children = NULL;
	rootDirectory->contents = -1;
	rootDirectoryPage->directories = rootDirectory;
	printf("created rootDirectory\n");

	int fileData = open(file, O_RDWR);
	int *map = (int *) mmap(NULL, 512, PROT_WRITE, MAP_SHARED, fileData, 0);

	printf("1\n");
	//strcpy(map, "helloworld");
	if(map == MAP_FAILED){
	  perror("mmap failed");
	}
	printf("%c\n", map[0]);

	map[0] = rootSector->freeMemoryPages[0];
	// printf("1.2\n");
	map[1] = rootSector->freeMemoryPages[1];
	// printf("1.3\n");
	map[2] = rootSector->directoryPages;
	// printf("1.4\n");
	map[3] = rootSector->lastAllocatedPage;

	printf("2\n");
	if(msync(map, 128, MS_SYNC) == -1){
		printf("3\n");
		close(fileData);
		printf("we fucked up");
		exit(EXIT_FAILURE);
	}
	if (munmap(map, 128) == -1)
	  {
	    printf("4\n");
	    close(fileData);
	    perror("Error un-mmapping the file");
	    exit(EXIT_FAILURE);
	  }
	
	// Un-mmaping doesn't close the file, so we still need to do that.
    close(fileData);
    //   fclose(fp);

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

	FILE * fp = fopen(file,"r");
	if(fp == NULL){
		initializeFileSystem(file);
	}

	exit(0);

	//int fd = open(file, O_RDONLY);

	//char *map = mmap(NULL,512, PROT_READ, MAP_SHARED,fd,0);

	//struct root_sector root;



	/* You will probably want other variables here for tracking purposes */


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
			break;
		}
		else if(!strncmp(buffer, "dump ", 5))
		{
			if(isdigit(buffer[5]))
			{
				//dump(stdout, atoi(buffer + 5));
			}
			else
			{
				/*char *filename = buffer + 5;
				*/
				char *space = strstr(buffer+5, " ");
				*space = '\0';
				//open and validate filename
				//dumpBinary(file, atoi(space + 1));
			}
		}
		else if(!strncmp(buffer, "usage", 5))
		{
			//usage();
		}
		else if(!strncmp(buffer, "pwd", 3))
		{
			//pwd();
		}
		else if(!strncmp(buffer, "cd ", 3))
		{
			//cd(buffer+3);
		}
		else if(!strncmp(buffer, "ls", 2))
		{
			//ls();
		}
		else if(!strncmp(buffer, "mkdir ", 6))
		{
			//mkdir(buffer+6);
		}
		else if(!strncmp(buffer, "cat ", 4))
		{
			//cat(buffer + 4);
		}
		else if(!strncmp(buffer, "write ", 6))
		{
			/*char *filename = buffer + 6;
			*/char *space = strstr(buffer+6, " ");
			*space = '\0';
			size_t amt = atoi(space + 1);
			space = strstr(space+1, " ");

			char *data = generateData(space+1, amt<<1);
			//write(filename, amt, data);
			free(data);
		}
		else if(!strncmp(buffer, "append ", 7))
		{
			/*char *filename = buffer + 7;
			*/
			char *space = strstr(buffer+7, " ");
			*space = '\0';
			size_t amt = atoi(space + 1);
			space = strstr(space+1, " ");

			char *data = generateData(space+1, amt<<1);
			//append(filename, amt, data);
			free(data);
		}
		else if(!strncmp(buffer, "getpages ", 9))
		{
			//getpages(buffer + 9);
		}
		else if(!strncmp(buffer, "get ", 4))
		{
			/*char *filename = buffer + 4;
			*/char *space = strstr(buffer+4, " ");
			*space = '\0';
			/*size_t start = atoi(space + 1);
			*/
			space = strstr(space+1, " ");
			/*size_t end = atoi(space + 1);
			*/
			//get(filename, start, end);
		}
		else if(!strncmp(buffer, "rmdir ", 6))
		{
			//rmdir(buffer + 6);
		}
		else if(!strncmp(buffer, "rm -rf ", 7))
		{
			//rmForce(buffer + 7);
		}
		else if(!strncmp(buffer, "rm ", 3))
		{
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
