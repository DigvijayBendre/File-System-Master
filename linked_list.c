#include <stdio.h>
#include<stdlib.h>
#include<string.h> 
#include "myMalloc.h"
#include "disk_ops.h"

typedef struct filedata {
	char * filename ;
	off_t size ;
	int block_num ;
	struct filedata *next ;
}node ;

node *head = NULL ;

int insert (char * filename, off_t size, int block_num){		
	
	if (if_present(filename) != -1){  // means file is already present
		bzero(error_msg, 256) ;
		strcpy(error_msg, "File with same name exists on the disk.") ;
		return -1 ;
	} 

	//using mymalloc
	node *new = (node *)mymalloc(sizeof(node)) ;	//defined in myMallloc.c
	if (new == NULL){
		bzero(error_msg, 256) ;
		strcpy(error_msg, "Memory allocation failed; Overflow") ;
		return -1 ;
	}

	new->filename  = (char *)mymalloc(strlen(filename) + 1) ;	//defined in myMallloc.c
	if (new->filename == NULL){
		bzero(error_msg, 256) ;
		strcpy(error_msg, "Memory allocation failed; Overflow") ;
		return -1 ;
	}

	strcpy(new->filename, filename);
	new->block_num = block_num ;
	new->size = size ;
	new->next = NULL ;
	
	if (head == NULL)
		head = new ;

	else{
		node *temp = head ;
		while (temp->next != NULL)
			temp  = temp->next ;

		temp->next = new ;	
	}
	
	return 1 ;
}

int display(char * total_files){

	if (head == NULL){
		total_files[0] = '\0' ;
		return -1 ;
	}
	
	node *ptr = head ;

	while(ptr!= NULL){
		strcat(total_files, ptr->filename) ;
		strcat(total_files, ", ") ;
		ptr = ptr->next ;
	}

	return 1 ;
}

// if file is present, returns block number at the which the file header is present
// -1 otherwise
int if_present(char *filename){
	if (head == NULL)
		return -1 ;

	node * ptr = head ;
	while (ptr != NULL){
		if  (strcmp(ptr->filename, filename) == 0)
			return ptr->block_num ;

		ptr = ptr->next ;
	}

	return -1 ;
}