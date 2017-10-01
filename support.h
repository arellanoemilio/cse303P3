#ifndef SUPPORT_H__
#define SUPPORT_H__

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
 * This fuction take in an a directory and directory_page and intends to map it
 * to the char *. In the case that we run out of space in directory_page then
 * the function will seek a new page in the file system where it will be able to
 * write the rest of the directories. In the case that directory page is already
 * pointing to a next directory_page then that is the page where the mapping
 * continues. If the mapping completes and the directory_page is poiningt to a
 * next directory page then that page is emptied and the directory page stops
 * pointing to it.
 */
void writeDirectoriesToMap(char *, struct directory_page *, struct directory *);


#endif
