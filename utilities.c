#include <stdlib.h>
#include <stdio.h>
#include<string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "myMalloc.h"
#include "utilities.h"
#include "disk_ops.h"

int err ;

int writeDiskBlock(int fd, int block_num, void * buf){

	if (block_num > TOTAL_BLOCKS || block_num < 0)
		return -4 ; // trying to acess invalid block number

    if (lseek(fd, block_num * BLK_SIZE, SEEK_SET)==-1)
    	return -1 ;

    return write(fd, buf, BLK_SIZE) ;
}

int readDiskBlock(int fd, int block_num, void *buf){
	if (block_num > TOTAL_BLOCKS || block_num < 0)
		return -4 ; // trying to acess invalid block number

 	if (lseek(fd, block_num * BLK_SIZE, SEEK_SET) == -1 )
 		return -1 ;
 	
    return read(fd, buf, BLK_SIZE) ;
}

int eraseDiskBlock(int fd, int block_num){
	if (block_num > TOTAL_BLOCKS || block_num < 0)
		return -4 ; // trying to acess invalid block number

	char *b1 = (char*)mymalloc(BLK_SIZE) ;
	if (b1 == NULL)
		return -2 ; //insufficient memory

	err = readDiskBlock(fd, block_num, b1) ;
	if (err < 0){
		myfree(b1) ;
		return err ;
	}
		

	int i = 0 ;
	while (i < BLK_SIZE){
		b1[i] = '\0' ;
		i++ ;
	}

	err = writeDiskBlock(fd, block_num, b1) ;
	if (err < 0){
		myfree(b1) ;
		return err ;
	}
		
	
	myfree(b1) ;
	return 1 ;
}

void getFilename(char  * buffer, char * filename){	
	 /* buffer char pointer here is the block that contains the filename in it, we need to extract this
	 filename and copy the same into the filename char pointer. */
	int i = 0 ;
	while (i < FILE_NAME_SIZE && buffer[i] != '\0'){
		filename[i] = buffer[i] ;
		i++ ;
	}
	filename[i] = '\0' ;
}

off_t filesize(char *buffer){
	int i = FILE_NAME_SIZE ;
	off_t ans  = 0 ;

	while(i < BLK_SIZE && buffer[i] !='\0'){
		ans = ans*10 + buffer[i] - 48 ;
		i++ ;
	}
	return ans ;
}

int ifFreeBlock(int fd, int block_num){
	lseek(fd, block_num * BLK_SIZE, SEEK_SET) ;
	char *b2 = (char*)mymalloc(BLK_SIZE) ;
	if(b2 == NULL)
		return -2 ; 
	
	err = readDiskBlock(fd, block_num, b2) ;
	if(err<0){
		myfree(b2) ;
		return err ;
	}
		

	int i = 0 ;
	while(i < BLK_SIZE){
		if (b2[i] != '\0'){
			myfree(b2) ;
			return 0 ;
		}
			

		i++ ;
	}
	myfree(b2) ;
	return 1 ;
}

int next_free_block(int fd, long free_block){
	
	while (free_block < TOTAL_BLOCKS){
		if (ifFreeBlock(fd, free_block))
			return free_block ;

		else	free_block ++ ;
	}

	return -1 ;
}

int search_file (int fd, char * filename_to_search){
	
	char *filename = (char*)mymalloc(FILE_NAME_SIZE) ;
	char *header_buf = (char*)mymalloc(BLK_SIZE) ;
	char *b3 = (char*)mymalloc(BLK_SIZE) ;

	if(b3 == NULL || filename == NULL || header_buf == NULL)
		return -2 ; // insufficient memory

	for(int i = 0; i < RESERVED_BLOCKS; i++){
		
		err = readDiskBlock(fd, i, b3); //defined in utilities.c
		if (err<0){
			myfree(filename) ; myfree(header_buf) ; myfree(b3) ;
			return err ;
		}
			
		for (int j = 0; j < BLK_SIZE; j++)
			if (b3[j] == 'y'){
				
			err = readDiskBlock(fd, i * BLK_SIZE + j, header_buf);
			if (err<0){
				myfree(filename) ; myfree(header_buf) ; myfree(b3) ;
				return err ;
			}
				
			bzero(filename, FILE_NAME_SIZE) ;
			getFilename(header_buf, filename) ; // defined in utilities.c
			if (strcmp(filename, filename_to_search) == 0){
				myfree(filename) ; myfree(header_buf) ; myfree(b3) ;
				return i * BLK_SIZE + j ; //returning the block number of the header block.
			}
				
				
	
		
			}
	}
	myfree(filename) ; myfree(header_buf) ; myfree(b3) ;
	return -5; // file not on disk ;
}
