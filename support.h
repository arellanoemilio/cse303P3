#ifndef SUPPORT_H__
#define SUPPORT_H__
#include "structs.h"

/*
 * Store information about the student who completed the assignment, to
 * simplify the grading process.  This is just a declaration.  The definition
 * is in student.c.
 */
extern struct student_t
{
	char *name;
	char *email;
} student;

/*
 * This function verifies that the student name is filled out
 */
void check_student(char *);

/*
 * This fuction takea in an int and converts it into its 4 bit parts and then
 * it stores them in the char *. This fuction stores the int least significant
 * bit first. This function expects that the char pointer has at least 4 bytes
 * allocated and it overwrites them.
 */
void writeIntToCharArr(char *, int);

/*
 * Given a set of two free_memory_pages and the last allocated page number this
 * method returns the next allocated page.
 */
int findNewPage(struct free_memory_page *bitMap, int *lastAllocatedPage);

int freeMemoryPage(struct free_memory_page *bitMap, int *lastAllocatedPage);
/*
 * This Function take in a char pointer of at least 4 bytes and transforms the
 * four bytes into an int. The bytes are read as least significan first.
 */
int getIntFromCharArr(char *charPointer);

/*
 * returns 1 if filename is a valid filesytem else returns -1
 */
int verify(char *filename);

/*
 * Returns the 8 consecutive pages of 512 byte pages as a char pointer
 */
char * loadPage(struct loaded_pages *loadedPages, int pageOffset);

/*
 * Retruns page number of the requested by the folder name, returns -1
 * if the directory is not found, the directory is a file, the directory is
 * located in an invalid location. This also changes the value of
 * currentDirectory to the directory traversed to.
 */
int traverseToDirectory(struct directory_page *currentDirectory, char *directoryName, struct loaded_pages *loadedPages);

int traverseToFileDirectory(struct directory_page *currentDirectory, char *directoryName, struct loaded_pages *loadedPages, int *filePage);

int pageContainsDirectory(struct directory_page *current, char *folderName);

/*
 * Returns an initialized directory_page from a 512 byte map.
 struct directory_page loadMapIntoPage(char * map);
 */
/*
 * Stores the page offset given
 * returns the page -1 if failed
 */
int updatePage(struct loaded_pages *loadedPages, int pageOffset);

int updateFile(int pageNum, struct loaded_pages *loadedPages);

void updateRootSector(struct root_sector *rootSector, struct loaded_pages *loadedPages);

/*
 * Given a currentDirectory, this method removes the directory specified if it
 * is empty
 */
int removeDirectory(struct directory_page *directory, char *directoryName, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage);

int removeFile(struct directory_page *directory, char *directoryName, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage);

int removeRecursively(struct directory_page *directory, char *directoryName, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage);

int makeDirectory(struct directory_page *directory, char *directoryName, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage);

/*
 * converts a list of file_locations into a character array for purpose of
 * mapping
 */
char * fileLocationsToCharArr(struct file_location *filesLocations, int numElements,int *numPages);

int mapDirectoryToMap(char *map, struct directory_page *rootDirectory, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage);

int loadDirectoryFromMap(struct directory_page *directory, char *map, struct loaded_pages *loadedPages);

int mapDataPageToMap(char *map, struct data_page *dataPage, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage);

void directoryCopy(struct directory_page *dest, const struct directory_page *source);

int countSetBits(int n);

void printWorkingDirectory(struct directory_page *directory, struct loaded_pages *loadedPages);

void list(struct directory_page *directory, struct loaded_pages *loadedPages);

int getPageType(struct loaded_pages *loadedPages, int pageNumber);

int writeFile(char* filename, int amt, char* newData, struct directory_page *directory, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage);

void cat(char * str, struct directory_page *directory, struct loaded_pages *loadedPages);

int appendWriteFile(char* filename, int amt, char* newData, struct directory_page *directory, struct loaded_pages *loadedPages, struct free_memory_page *bitMap, int *lastAllocatedPage);

#endif
