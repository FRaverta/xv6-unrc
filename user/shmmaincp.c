/*
Program that prepare a stage for the interaction between shmproducer and shmconsumer.
Run shmmaincp amount_of_pages (where amount_of_pages is a the amount of memory 4kb-pages that have the shared memory block)
then, run producer||consumer with some of following commands
	*shmconsumer shmblock_descriptor full empty shmblock_sem (where full, empty, shmblock_sem are the semapores descriptors creates by maincp)
	*shmproducer shmblock_descriptor full empty shmblock_sem value (where full,empty and shmblock_sem are the semapores descriptors creates by maincp and value is a character that write producer)
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

	
#define PGSIZE 4096

void
report_sem_errors(int s)
{
	switch(s){		
		case -1: printf(1,"nroError:%d-sem_id is not in use, should be created\n",s);break;
		case -2: printf(1,"nroError:%d-the process has the maximun number of semaphores allowed",s);break; 
		case -3: printf(1,"nroError:%d-the system don't have any free semaphore",s);break;
		case -4: printf(1,"nroError:%d-wrong sem_id parameter, sem_id cannot be less than -1",s);break; 
	}
}	

void
report_shmcreate_errors(int s)
{
	switch(s){		
		case -1: printf(1,"nroError:%d-non available resources for current request\n",s);break;
		case -2: printf(1,"nroError:%d-size bad: it is less or equal to 0",s);break; 
		case -3: printf(1,"nroError:%d-size bad: the request block is bigger than the maximum block size",s);break;
		case -4: printf(1,"nroError:%d-current process has all shared memory blocks already busy",s);break; 
		case -5: printf(1,"nroError:%d-OS has all shared memory blocks already busy.",s);break; 
	}
}	

void
report_shmget_errors(int s)
{
	switch(s){		
		case -1: printf(1,"nroError:%d-current process has all shared memory blocks already busy.\n",s);break;
		case -2: printf(1,"nroError:%d-Sharem memory block with key descriptor hasn't been created.",s);break; 
		case -3: printf(1,"nroError:%d-Process haven't enought adresses space.",s);break;
	}
}	


int 
main(int argc,char *argv[]){
	int shmbd, aux;
	int full,empty,shmblock_sem;
	void *addr;
	int size;

	size = atoi(argv[1]);
	
	if(argc!=2){
		printf(1,"use shmmaincp amount_of_pages \n");	
		exit();
	}

	//create shm block and reports the result of this operation
	shmbd = shm_create(size * PGSIZE);
	if (shmbd < 0){report_shmcreate_errors(shmbd); exit();}

	if( (aux = shm_get(shmbd,&addr)) <0 ) {report_shmget_errors(aux); exit();}
	else printf(1,"shm block was get\n");

	//create semaphore and report the result of this operation
	full= semget(-1, (size * PGSIZE) - 4);
	empty= semget(-1,0);
	shmblock_sem= semget(-1,1);
	if(full<0 || empty<0 || shmblock_sem<0){
		if(full<0)
			report_sem_errors(full);
		if(empty<0)
			report_sem_errors(empty);
		if(shmblock_sem<0)
			report_sem_errors(shmblock_sem);

		exit();
	}

	*((int*) addr) = 3;
	
	printf(1,"shm block is created with id: %d and it has %d pages\n",shmbd,size);
	printf(1,"Semaphores have been created\n");
	printf(1,"Full id:%d\n",full);
	printf(1,"Empty id:%d\n",empty);
	printf(1,"shmblock_sem id:%d\n",shmblock_sem);

	//run for always
	for(;;);
}