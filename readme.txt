		Short Overview of the baby step 2 for file system


Disk size - 20 mb
block size - 512 bytes
total blocks - 40960
reserved blocks - [0,80)

The concept used to manage Virtual disk is :

* We reserve first 80 block of the disk, where we will have 80*512 bytes, 
  where each byte corresponds to a block number i.e. if the ith byte have the value 'y' then, ith block contains the header of a file.

* header of the file is a block which contains the information about the file eg. Filename, Filesize and starting block number.
 The actual data of every file starts from the block just after its header.

* Before we start copying to/from the Virtual Disk, the program calls a subroutine initial_disk_read(), it traverses through the 
  reserved blocks and extract file's information into a structure in RAM for all the files residing on the disk.

* The storage of file(s) information on the RAM is allowed only limited memory and this limited memory is managed by mymalloc() method, 
  made during the first assignment.

* when we call Vdls() subroutine, we read the structure and copy all the file names into an char array and then print it.
  If no file is present in the structure, we print a message "No file on the disk"

* When copying onto Virtual disk, we note down the first block which is free, we mark (blockno)th byte as 'y' in the reserved blocks.
  We also insert the file information into the structure for fast access of files
  Input : Suppose a file on AD has complete path as /home/anurag/Desktop/isdia.txt then, filepath and filename will be as follows :
	file path : /home/anurag/Desktop/
	filename : isdia.txt

* When copying from Virtual disk to AD, we traverse our structure ti check if the file is present or not.
  If the file is not present we return a suitable error value. But if present we copy it to the specified path
  INPUT : Suppose you want to copy a file named isdia.txt from Virtual disk to /home/anurag/outputs/ in AD then, 
   copy_to_path : /home/anurag/outputs/
   filename : isdia.txt

* The error handling is dealt with two values viz -1 and -2 and a global char array which tells the error description.
  -1 tells, its a system call error, and thus perror can give better information about this.
  -2 tells, its a error occured due to the our program implementation. So we copy the the error description  in error_msg
	which is a global char array.    

* We print the error by checking the value returned from the subroutine.

#######################################################################
TO RUN THE PROGRAM, type the following on the terminal:
$ make
$ ./all


