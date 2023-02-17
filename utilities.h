#include <math.h>
#include <sys/types.h>
#define DISKSIZE 20971520L 
#define FILE_NAME_SIZE 256
#define BLK_SIZE 512
#define TOTAL_BLOCKS DISKSIZE/BLK_SIZE
#define RESERVED_BLOCKS TOTAL_BLOCKS/BLK_SIZE
#define MOVE_BYTES ((int)ceil(log10(TOTAL_BLOCKS)) )

int fd_VD ;
long free_block ;

int writeDiskBlock(int fd, int block_num, void * buf) ;
int readDiskBlock(int fd, int block_num, void *buf) ;
int eraseDiskBlock(int fd, int block_num) ;
void getFilename(char  * buffer, char * filename) ;
int ifFreeBlock(int fd, int block_num) ;
off_t filesize(char *buffer) ;
int next_free_block(int fd, long free_block) ;
int search_file (int fd, char * filename_to_search) ;

