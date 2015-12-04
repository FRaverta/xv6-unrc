/*
* Program that represents a consumer who get character from some shared memory block.
* shmconsumer shmblock_descriptor full empty shmblock_sem (where full, empty, shmblock_sem are the semapores descriptors creates by shmmaincp)
* 
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

#define element_that_consume 500
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

void
report_shmget_errors(int s)
{
	switch(s){		
		case -1: printf(1,"nroError:%d-current process has all shared memory blocks already busy.\n",s);break;
		case -2: printf(1,"nroError:%d-Sharem memory block with key descriptor hasn't been created.",s);break; 
		case -3: printf(1,"nroError:%d-Process haven't enought adresses space.",s);break;
		case -4: printf(1,"nroError:%d-Invalid key in shm_get.",s);break;
	}
}	

/*
Consumer program. 
Consumer read a character from shared memory block whose id is pased in argv too.  */
int 
main(int argc,char *argv[])
{
	int full, empty, shmblock_sem, shmbd, aux;
	uint pos;
	void * addr;

	if(argc < 5){printf(1,"use producer shmblock_descriptor full empty shmblock_sem element_that_produce\n");exit();}
	
	//take all paramethers
	shmbd = atoi(argv[1]);
	full = atoi(argv[2]);
	empty = atoi(argv[3]);
	shmblock_sem = atoi(argv[4]);

	if(full == -1 || empty == -1 || shmblock_sem == -1){printf(1,"All sempahores should be created before consumer invocation\n");exit();}
	
	//obtain semaphores
	full = semget(full,0);
	empty = semget(empty,0);
	shmblock_sem = semget(shmblock_sem,0);
	if(full < 0 || empty < 0 || shmblock_sem < 0){
		if(full < 0)
			report_errors(full);
		if(empty < 0)
			report_errors(empty);
		if(shmblock_sem < 0)
			report_errors(shmblock_sem);

		exit();
	}

	printf(1,"Semaphores have been created\n");

	//obtain shared memory block
	if( (aux = shm_get(shmbd,&addr)) < 0 ) {report_shmget_errors(aux); exit();}
	else printf(1,"shm block was get at: %d\n",(int)addr);
	
	int i;
	//producer loop
	for(i=0;i<element_that_consume;i++){
		semdown(empty);
		semdown(shmblock_sem);
			pos = *((int*)addr);
			printf(0, "Consumer %c at pos: %d\n", *((char*)(addr + pos)), pos);
			pos--;
			*((int*)addr) = pos;
		semup(shmblock_sem);				
		semup(full);		
	}
	
	exit();
}
