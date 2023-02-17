CC = gcc

option=-c

all:	teasage.o  myMalloc.o utilities.o disk_operations.o
		$(CC) teasage.o  myMalloc.o utilities.o disk_operations.o -o  all

teasage.o:	teasage.c disk_ops.h utilities.h
			$(CC) $(option) teasage.c

myMalloc.o:	myMalloc.c myMalloc.h
			$(CC) $(option) myMalloc.c

disk_operations.o :	disk_operations.c myMalloc.h utilities.h
					$(CC) $(option) disk_operations.c

utilities.o:	utilities.c utilities.h myMalloc.h
				$(CC) $(option) utilities.c


clean: 
		rm -rf *o all
