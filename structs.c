#include"structs.h"

//There should only every be one of these in the file system in location 0
struct root_sector
{
  //int allocationPages;
  int directoryPages;
  int *rootDirectories;
  /*
   * IMPORTANT: we need extctly two, each one can hold a bit for 4096 pages
   * the 1st-3rd bit in the first page should always be one since these three
   * pages will always be one since the first page is root, the second is the
   * the first freeMemoryPage and the third is the second freeMemoryPage
   */
  int *freeMemoryPages;
  int lastAllocatedPage;

}

struct freeMemoryPage{
  //IMPORTAT: this should allocate 512 bytes, that is equal to 4096 bits.
  char *freePages;
}

//This one i feel needs to change we should discuss this one more
struct directory{
  struct directory *parentDirectory;
  // Just the name of that folder
  char *name;
  int isFile;
  // If is file is 1 then children would be NULL
  struct directory *children;
  // If is file is 0 then contenents would be -1
  int contents;
}


struct directory_page{
    int empty;
    //PageType should be 1
    int pageType;
    int numElements;
    int nextDirectoryPage;
    //IMPORTANT: We should always allocate 500 bytes for directories
    struct directory *directories;
}

struct data_page{
  int empty;
  //PageType should be 2
  int pageType;
  int size;
  int nextDataPage;
  //IMPORTANT: we should allocate no more than 500 bytes for the data
  char *data
}
