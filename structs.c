#include"structs.h"

struct root_sector
{
  //int allocationPages;
  int directoryPages;
  int *rootDirectories;
}

struct directory{
  char *name;
  int location;
  int length;
}

struct directory_page{
    int numElements;
    //IMPORTANT: We should always allocate 508 bytes for directories
    struct directory *directories;
}
