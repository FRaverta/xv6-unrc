/*
Program that prepare a stage for 
the interaction between producer,consumer.
Run maincp filename (where file_name is a string)
then, run producer||consumer with some of following commands
	*consumer full empty file (where full,empty and file are the semapores creates by maincp)
	*producer full empty file value (where full,empty and file are the semapores's ids creates by maincp and value is a value that write producer)
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

int 
main(int argc,char *argv[]){
	int fd;
	int full,empty,file;
	//struct spinlock lock;
	char* file_name;
	
	if(argc<2){
		printf(1,"use maincp file_name \n");	
		exit();
	}

	//create file and reports the result of this operation
	file_name=argv[1]; //obtain file name passed as parameter.
	fd=open(file_name,O_CREATE);
	if(fd<0){printf(1,"Error in create file: %s\n",file_name); exit();}
	else printf(1,"file: %s is created\n",file_name);

	//create semaphore and report the result of this operation
	full= semget(-1,1);
	empty= semget(-1,0);
	file= semget(-1,1);
	if(full<0 || empty<0 || file<0){
		if(full<0)
			report_errors(full);
		if(empty<0)
			report_errors(empty);
		if(file<0)
			report_errors(file);

		exit();
	}
	printf(1,"Semaphores have been created");
	printf(1,"Full id:%d\n",full);
	printf(1,"Empty id:%d\n",empty);
	printf(1,"File id:%d\n",file);

	//run for always
	for(;;);
}
