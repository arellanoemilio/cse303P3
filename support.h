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
 * Retruns the loaded directory_page requested by the name, returns NULL
 * if the directory is not found, the directory is a file, the directory is
 * located in an invalid location.
 */
struct directory_page traverseToDirectory(struct directory_page currentDirectory,
	 																				char *directoryName,
																					struct loaded_pages *loadedPages);

/*
 * Returns an initialized directory_page from a 512 byte map.
 */
struct directory_page loadMapIntoPage(char * map);

/*
 * Stores the page offset given
 * returns the page -1 if failed
 */
int updatePage(struct loaded_pages *loadedPages, int pageOffset);

/*
 * Given a currentDirectory, this method removes the directory specified if it
 * is empty
 */
int removeDirectory(struct directory_page currentDirectory, char *directoryName, struct loaded_pages *loadedPages);

#endif
