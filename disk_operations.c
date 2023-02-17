#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "disk_ops.h"
#include "utilities.h"
#include "myMalloc.h"

int block_num_padding(int block_num, char *res){

	int i = MOVE_BYTES- 1 ;
	while(i >= 0 && block_num>0){
		res[i] = 48 + block_num%10 ;
		i-- ; block_num /= 10 ;
	}

	while (i >= 0){
		res[i] = 48 ;
		i-- ;
	}
	return 1 ;
}
void setPrevMoveByte_ofCurBlock(int prev, int free_block, char *buf){

	char res[MOVE_BYTES] ;
	block_num_padding(prev, res) ;
	
	int i = MOVE_BYTES- 1 , j = BLK_SIZE - MOVE_BYTES- 1 ;
	while (i >= 0){
		buf[j] = res[i] ;
		j-- ; i-- ;
	}
}

void setNextMoveByte_ofPrevBlock(int fd_VD, int prev, int free_block){
	char buf[BLK_SIZE], res[MOVE_BYTES] ;
	readDiskBlock(fd_VD, prev, buf) ;
	
	block_num_padding(free_block, res) ;
	
	int i = MOVE_BYTES - 1 , j = BLK_SIZE-1;
	while (i >= 0){
		buf[j] = res[i] ;
		j-- ; i-- ;
	}

	writeDiskBlock(fd_VD, prev, buf) ;
}

int getPrevMoveByte(int fd, int block_num){
	char buf[BLK_SIZE] ;
	readDiskBlock(fd, block_num, buf) ;
	int prev_blk = 0 , i = BLK_SIZE - 2*MOVE_BYTES ;
	
	if (buf[i] == '\0')
		return -13 ;
	
	while (i < BLK_SIZE - MOVE_BYTES){
		prev_blk = prev_blk*10 + (buf[i] - 48) ;
		i++ ;
	}

	return prev_blk ;
}

int getNextMoveByte(int fd, int block_num){
	char buf[BLK_SIZE] ;
	readDiskBlock(fd, block_num, buf) ;

	int next_blk = 0 , i = BLK_SIZE - MOVE_BYTES ;

	if (buf[i] == '\0')
		return -13 ;

	while (i < BLK_SIZE ){
		next_blk = next_blk*10 + (buf[i] - 48) ;
		i++ ;
	}

	return next_blk ;
}

int Vdls(char *  bufptr)
{
	int err = 1 ;
	char *fname = (char*)mymalloc(FILE_NAME_SIZE);
	char *header = (char*)mymalloc(BLK_SIZE);
	char *buffer = (char*)mymalloc(BLK_SIZE); 

	if (fname == NULL || header == NULL || buffer == NULL)
		return -2 ; // insufficient memory

	int buf_len= 0 , i= 0, j=0 ;

	//getting the file length from the buffer pointer
	while (bufptr[i] != '\0'){

		//if any byte does not represents a char between ascii value [48, 57], then the length is invalid
		if (bufptr[i]<48 || bufptr[i] > 57)
			return -3 ; // expected an ascii value between [48, 57]
	
		buf_len = buf_len*10 + (bufptr[i]-48) ;
		i++ ;
	}
	int len_bytes = i , cur_blk = 0, k, c = len_bytes + MOVE_BYTES +1, len = buf_len ;
	i++ ;
	
	//getting the current block number we are reading
	while (i <= len_bytes + MOVE_BYTES){

		//if any byte does not represents a char between ascii value [48, 57], then the current block cannot be calculated
		if (bufptr[i]<48 || bufptr[i]>57)
			return -3 ; // expected an ascii value between [48, 57]
			
		cur_blk = cur_blk*10 + (bufptr[i] - 48) ;
		i++ ;
	}
	
	i = cur_blk/BLK_SIZE ;
	j = cur_blk%BLK_SIZE ;
	buf_len = buf_len -(len_bytes + MOVE_BYTES) ;
	
	for(i ; i < RESERVED_BLOCKS; i++){
		err = readDiskBlock(fd_VD, i, buffer); //defined in utilities.c
		if (err<0){
			myfree(fname) ; myfree(header) ; myfree(buffer) ;
			return err ;
		}
			
		for (j ; j < BLK_SIZE; j++){
			if (buffer[j] == 'y'){ 
	
				err = readDiskBlock(fd_VD, i * BLK_SIZE + j, header);
				if (err<0){
					myfree(fname) ; myfree(header) ; myfree(buffer) ;
					return err ;
				}
					

				bzero(fname, FILE_NAME_SIZE) ;			
				getFilename(header, fname) ; // defined in utilities.c
				
				if (buf_len < strlen(fname)+1){ 
					// if buffer is out of space
					//put the current block into the bufptr's reserved bytes, so as to restart from this block in the next call
 					k = len_bytes + MOVE_BYTES, cur_blk = i*BLK_SIZE + j ;

					while(cur_blk > 0){
						bufptr[k] = cur_blk % 10 + 48 ;
						cur_blk /= 10 ;
						k -- ;
					}
					while(k > len_bytes){
						bufptr[k] = 48 ;
						k-- ;
					}
					
					myfree(fname) ; myfree(header) ; myfree(buffer) ;
					return 1 ;

				}
		
				//append the filename to the bufptr
				k = 0;
				while(k < strlen(fname)){
					bufptr[c] = fname[k] ;
					c++; k++ ;
				}
	
				bufptr[c] = '\n' ;
				c++ ;
				buf_len -= (strlen(fname) + 1) ; //reduce the size of the bufptr by the length of currently added filename.
			}
		}
		j = 0 ;
	}

	// if number of bytes required by the total filenames were less than the bufptr size, 
	// we put TOTAL_BLOCKS in the reserved bytes of the bufptr, so as to meet the terminating condition.
	k = len_bytes + MOVE_BYTES, cur_blk = i*BLK_SIZE + j ;
	while(cur_blk > 0){
		bufptr[k] = cur_blk % 10  + 48 ;
		cur_blk /= 10 ;
		k-- ;
	}
	while (k > len_bytes){
		bufptr[k] = 48 ;
		k-- ;
	}
	
	myfree(fname) ; myfree(header) ; myfree(buffer) ;
	return 1 ;
}

int VdCpto(char * file_path, char * fileName)
{
	int err = 1 ;
	//first check if file is already present in the disk.
	int ret_blk  = search_file (fd_VD, fileName) ;
	if (ret_blk > 0){
		return -6 ;
	} // file already present

	char *filename = (char*)mymalloc(FILE_NAME_SIZE);
	char *header = (char*)mymalloc(BLK_SIZE);
	char *buffer = (char*)mymalloc(BLK_SIZE); 
	if (filename == NULL || header == NULL || buffer == NULL)
		return -2 ; // insufficient memory

	file_path = strcat(file_path,fileName) ; 

	int file_fd = open(file_path, O_RDONLY) ;
	if (file_fd == -1){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return -1 ;
	}	

	off_t fileLength = lseek(file_fd, 0, SEEK_END) ;
	if (fileLength == -1){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return -1 ;
	}	

	lseek(file_fd, 0, SEEK_SET) ; //setting pointer to begining of the file .
	
	//First block of the file will represent its filename, filesize
	int i = 0 , j = 0 ;
	//putting filename in the first 256 bytes of the block
	while (fileName[i] != '\0'){
		header[i] = fileName[i] ;
		i++ ;
	}

	while(i < FILE_NAME_SIZE) //padding the bytes remaining out of 256 with null char
		header[i++] = 0 ;

	off_t fl = fileLength ;
	int digits = 0 ;
	while (fl > 0){
		digits++ ;
		fl /= 10 ;
	}

	j = FILE_NAME_SIZE + digits - 1;
	i = j ;
	fl = fileLength ;
	//putting filelength's digits as characters from the 256th byte.
	while (fl > 0 ){
		header[i--] = 48 + fl%10 ;
		fl  =  fl/10 ;
	}

	j++ ;
	while (j < BLK_SIZE)
		header[j++] = '\0' ;


	free_block = next_free_block(fd_VD, free_block) ;
	if (free_block == -1){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return -7 ; //disk full
	}
		

	//turn on the (block_num)th byte as 'y' in the reserved disk space, representing a file starts from this block.

	err = readDiskBlock(fd_VD, free_block/BLK_SIZE, buffer) ; //defined in utilities.c
	if (err<0){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return err ;
	}
		
	
	buffer[free_block%BLK_SIZE]  = 'y' ;

	err = writeDiskBlock(fd_VD, free_block/BLK_SIZE, buffer);	//defined in utilities.c
	if (err<0){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return err ;
	}
		

	err = writeDiskBlock(fd_VD, free_block, header) ;	
	if (err<0){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return err ;
	}
		
	
	// now copying the file data onto the disk (poor man's)
	while(fileLength > 0){
		if (read(file_fd, buffer, BLK_SIZE - 2*MOVE_BYTES) < 0){
			myfree(filename) ; myfree(header) ; myfree(buffer) ;
			return -1 ;
		}
			

		int prev = free_block ;

		free_block = next_free_block(fd_VD, free_block) ;
		if (free_block == -1){
			myfree(filename) ; myfree(header) ; myfree(buffer) ;
			return -7 ; //disk full
		}
			

		setPrevMoveByte_ofCurBlock(prev, free_block, buffer) ;
		setNextMoveByte_ofPrevBlock(fd_VD, prev, free_block) ;
		
        //here at last few bytes, the next block in sequence needs to be mentioned
        //so read number of bytes  = BLK_SIZE - digits(No_of_blocks)
        
        err = writeDiskBlock(fd_VD, free_block, buffer) ;
		if (err<0){
			myfree(filename) ; myfree(header) ; myfree(buffer) ;
			return err ;
		}
		

        fileLength = fileLength -  BLK_SIZE + 2*MOVE_BYTES  ;
	}
	
	
	close(file_fd) ;
	myfree(filename) ; myfree(header) ; myfree(buffer) ;

	return 1 ;
}

int VdCpfrom(char * copy_to_path, char * fileName)
{
	int err = 1 ;
	char *filename = (char*)mymalloc(FILE_NAME_SIZE);
	char *header = (char*)mymalloc(BLK_SIZE);
	char *buffer = (char*)mymalloc(BLK_SIZE);
	if(filename == NULL || header == NULL || buffer == NULL)
		return -2 ; // insufficient memory

	int cur_blk, prev_blk  = search_file (fd_VD, fileName) ;
	if (prev_blk < 0){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return -5 ; // file not present
	}
		
	
	err = readDiskBlock(fd_VD, prev_blk, buffer) ;
	if(err<0){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return err;
	}
		

	off_t extracted_filelength = filesize(buffer) ;	 //defined in utilities.c
	off_t fl = extracted_filelength ;
	strcat(copy_to_path, fileName) ;	
	int new_copy = open(copy_to_path, O_WRONLY | O_CREAT, 00700) , ret;
	if (new_copy < 0){
		myfree(filename) ; myfree(header) ; myfree(buffer) ;
		return -1 ;
	}

	while (fl > 0 ) {
		if (fl> BLK_SIZE - 2*MOVE_BYTES)
            ret = BLK_SIZE - 2*MOVE_BYTES ;
        else
            ret = fl ;

        cur_blk = getNextMoveByte(fd_VD, prev_blk) ;
        err = readDiskBlock(fd_VD, cur_blk, buffer) ; 
		if(err<0){
			myfree(filename) ; myfree(header) ; myfree(buffer) ;
			return err ;
		}

        if(write(new_copy, buffer, ret) < 0) {
			myfree(filename) ; myfree(header) ; myfree(buffer) ;
			return -1 ;
		}
       
        fl -= ret ;
        prev_blk = cur_blk ;
	}

	close(new_copy) ;
	myfree(filename) ; myfree(header) ; myfree(buffer) ;
	return 1 ; 
}

int deleteFileFromDisk( char * filename)
{	
	int err = 1 ;
	// first search, if the file exists or not
	int cur_blk, prev_blk  = search_file (fd_VD, filename) ;
	
	if (prev_blk < 0)
		return -5 ; // file not present

	char *header = (char*)mymalloc(BLK_SIZE);
	char *buffer = (char*)mymalloc(BLK_SIZE);
	if(header == NULL || buffer == NULL)
		return -2 ; // insufficient memory

	//turn off the byte that coresponds to the header block of the file being deleted.
	err = readDiskBlock(fd_VD, prev_blk/BLK_SIZE, buffer) ; //defined in utilities.c
	if (err<0){
		myfree(header) ; myfree(buffer) ;
		return err ;
	}

	buffer[prev_blk%BLK_SIZE]  = '\0' ;
	
	err = writeDiskBlock(fd_VD, prev_blk/BLK_SIZE, buffer);	//defined in utilities.c
	if (err<0){
		myfree(header) ; myfree(buffer) ;
		return err ;
	}

	err = readDiskBlock(fd_VD, prev_blk, buffer) ;
	if (err<0){
		myfree(header) ; myfree(buffer) ;
		return err ;
	}

	off_t fl = filesize(buffer) ;
	
	//if the header block lies behind the curr free_block being pointed, then point the current free_block as header block
	// as it is about to be freed
	if (prev_blk < free_block)
		free_block = prev_blk ;

	//erasing evry block that belongs to that file

	do{
		cur_blk = getNextMoveByte(fd_VD, prev_blk) ;
		eraseDiskBlock(fd_VD, prev_blk) ;
		prev_blk = cur_blk ;
	}while(prev_blk > 0) ; 

	myfree(header) ; myfree(buffer) ;

	return 1 ;
}