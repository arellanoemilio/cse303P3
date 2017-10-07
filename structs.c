
//There should only every be one of these in the file system in location 0
struct root_sector
{
  //int allocationPages;
  int directoryPages;
  /*
   * IMPORTANT: we need extctly two, each one can hold a bit for 4096 pages
   * the 1st-3rd bit in the first page should always be one since these three
   * pages will always be one since the first page is root, the second is the
   * the first freeMemoryPage and the third is the second freeMemoryPage
   */
  int *freeMemoryPages;
  int lastAllocatedPage;

};

struct free_memory_page{
  //IMPORTAT: this should allocate 512 bytes, that is equal to 4096 bits.
  char *freePages;
};

struct loaded_pages{
  int fileData;
  int numberOfLoadedPages;
  int *loadedPagesList;
  char **pages;
};

struct file_location{
  char *name;
  int location;
};

struct directory_page{
    int empty;
    //PageType should be 1
    int pageType;
    int numElements;
    int nextDirectoryPage;
    //IMPORTANT: We should always allocate 496 bytes for directories
    //order of files will be itself then parrent
    struct file_location * filesLocations;
};

struct data_page{
  int empty;
  //PageType should be 2
  int pageType;
  int size;
  int nextDataPage;
  //IMPORTANT: we should allocate no more than 500 bytes for the data
  char *data;
};
