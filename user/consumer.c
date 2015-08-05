/*
Program that represents a consumer.
	*consumer full empty file (where full,empty and file are the semapores creates by maincp)
*/

#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

//auxiliar function to report errors abaout semaphore use
void
report_errors(int s)
{
	switch(s){		
		case -1: printf(1,"nroError:%d-sem_id is not in use, should be created\n",s);break;
		case -2: printf(1,"nroError:%d-the process has the maximun number of semaphores allowed",s);break; 
		case -3: printf(1,"nroError:%d-the system don't have any free semaphore",s);break;
		case -4: printf(1,"nroError:%d-wrong sem_id parameter, sem_id cannot be less than -1",s);break; 
	}
}	


/*
Consumer program. 
Consumer read a file pased in argv.  
*/
int 
main(int argc,char *argv[]){
	int full, empty, file;
	int fd;
	int c;
	char* file_name;
	int i;

	if(argc < 5){printf(1,"use consumer file_name full empty file\n");exit();}
	
	//take all paramethers.
	file_name = argv[1];
	full = atoi(argv[2]);
	empty = atoi(argv[3]);
	file = atoi(argv[4]);
	
	if(full == -1 || empty == -1 || file == -1){printf(1,"All sempahores should be created before consumer invocation\n");exit();}
	
	//obtain semaphores
	full = semget(full,0);
	empty = semget(empty,0);
	file = semget(file,0);
	if(full < 0 || empty < 0 || file < 0){
		if(full < 0)
			report_errors(full);
		if(empty < 0)
			report_errors(empty);
		if(file < 0)
			report_errors(file);

		exit();
	}
	printf(1,"Semaphores have been created\n");

	
	//consumer loop
	for(i=0;i<10;i++){
		semdown(empty);
		semdown(file);
			fd = open(file_name,O_RDWR);//open file					
			read(fd,&c,sizeof(c)); //consume
			printf(1,"consumer %d\n",c);
		semup(file);				
		semup(full);
		
	}
	exit();
	
}