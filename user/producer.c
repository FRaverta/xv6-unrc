/*
Program that represents a producer.
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
Producer program. 
Producer write a number pased in argv in a file pased in argv too.  
*/
int 
main(int argc,char *argv[]){
	int full, empty, file;
	int fd;
	int nro;
	char* file_name;
	int i;

	if(argc < 6){printf(1,"use producer file_name full empty file element_that_produce\n");exit();}
	
	//take all paramethers
	file_name = argv[1];
	full = atoi(argv[2]);
	empty = atoi(argv[3]);
	file = atoi(argv[4]);
	nro = atoi(argv[5]);

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
	

	//producer loop
	for(i=0;i<10;i++){
			semdown(full);
			semdown(file);			
			fd=open(file_name,O_RDWR);//open file
			write(fd,&nro,sizeof(nro));	//produce					
			printf(1,"producer %d\n",nro);
			semup(empty);
			semup(file);			
			sleep(4);	
	}
	
	exit();
}
