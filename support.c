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

void writeDirectoriesToMap(char * map, struct directory_page *rootDirectoryPage, struct directory *rootDirectory){
	int spaceAvailable = 500;
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

}
