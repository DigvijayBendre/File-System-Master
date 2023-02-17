#include "disk_ops.h"
#include "utilities.h"
#include "myMalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

extern int errno;

char nullbyte[] = "\0";

int main()
{
	system("clear") ;

	int err =1 ;
    //Let's create a disk of about 20MB - poor man's harddisk
    //our company name is teasage - producers of such great disks in a jiffy
    //with this we will compete with seagate :)
    fd_VD = open("disk.teasage", O_RDWR | O_CREAT, 00700);
    
    if (fd_VD ==-1) //some error
    {
        printf("Error: cannot create the grand disk.\n Errorno: %d\n", errno);
        perror("Open error.");
        exit(1);
    }
   
    //writing  null byte to the disk
   
    if (write(fd_VD, nullbyte, 1) != 1){
        perror("write error");
        exit(1) ;
    }

    if (lseek(fd_VD, DISKSIZE, SEEK_SET) == -1)
        perror("lseek error");

    if (write(fd_VD, nullbyte, 1) != 1)
        perror("write error");
	
	// allocating fixed memory for storing file's info onto RAM
	//initialize_mem() defined in myMalloc.c
    initialize_mem() ;
 	
	char *file_path = (char*)mymalloc(FILE_NAME_SIZE) ;
	char *filename = (char*)mymalloc(FILE_NAME_SIZE) ;

	int choice ;
	free_block = RESERVED_BLOCKS;

	while(1){
		printf("\n1. Veiw files on Disk\n2. Copy to Disk\n3. Copy from Disk\n4. Delete file\n5. exit\n") ;
		scanf( "%d", &choice) ;

		if (choice == 1){
			int buf_len = 20, len_bytes = 2 ;
			char *buf = (char*)mymalloc(buf_len) ;
			if(buf == NULL)
				return -2;

			int i=len_bytes-1 , len = buf_len;

			while(i>=0){
				buf[i] = 48 + buf_len%10 ;
				buf_len /= 10 ;
				i -- ;
			}
			buf[len_bytes] = '\0', i = len_bytes + MOVE_BYTES  ;
			while(i > len_bytes){
				buf[i] = 48 ;
				i-- ;
			}
			printf("Files currently on the disk\n") ;
			
			do{
				
				i = len_bytes + MOVE_BYTES + 1 ;
				while (i < len){
					buf[i] = '\0';
					i++;
				}
				bzero(&buf[len_bytes + MOVE_BYTES + 1], len - len_bytes - MOVE_BYTES - 1) ;
				err = Vdls(buf) ;
				if (err<0){
					printf("error no is : %d\n", err) ;
					return err ;
				}
					
				printf ("%s", &buf[len_bytes + MOVE_BYTES + 1]) ;
				
				//calculating current block in the buffer
				i = len_bytes + 1 , buf_len = 0 ;
				while(i <= len_bytes + MOVE_BYTES){
					buf_len = buf_len*10 + (buf[i]-48) ;
					i++ ;
				}
			}while(buf_len < TOTAL_BLOCKS) ;
			myfree(buf) ;
			printf("\n") ;
			err = 1 ;
		}

		else if (choice == 2){
			bzero(filename, FILE_NAME_SIZE) ;
			bzero(file_path, FILE_NAME_SIZE) ;

			printf("Enter file path :");
			scanf( "%s", file_path) ;
			printf("Enter file name :");
			scanf( "%s", filename) ;

			err = VdCpto(file_path, filename) ; // defined in disk_operations.c
			if (err < 0){
				printf("returned with exit code %d\n", err) ;
				if (err == -1)
					perror("yeh h: ") ;
			}
			else 	printf("success!\n" );
			err = 1 ;
		}

		else if(choice == 3){
			bzero(filename, FILE_NAME_SIZE) ;
			bzero(file_path, FILE_NAME_SIZE) ;

			printf("Copy to path :");
			scanf("%s", file_path) ;
			printf("Enter file name :");
			scanf( "%s", filename) ;			

			err = VdCpfrom(file_path, filename) ; // defined in disk_operations.c
			if(err<0)
				printf("returned with exit code %d\n", err) ;
			
			else	printf("success!\n");
			err = 1 ;	
		}

		else if (choice == 4){
			bzero(filename, FILE_NAME_SIZE) ;
			printf("Enter file name :");
			scanf("%s", filename) ;			

			err = deleteFileFromDisk(filename) ;
			if (err<0)
				printf("returned with exit code %d\n", err) ;
			else
				printf("%s deleted from the disk.\n",filename) ;
			
			err=1 ;
		}

		else if(choice == 5)
			break;
	}
	myfree(filename) ;
	myfree(file_path) ;
    close(fd_VD) ;
    return 1;
}


