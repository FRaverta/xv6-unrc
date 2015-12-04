#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "mmu.h"
#include "proc.h"
#include "semaphore.h"

//Table of system semaphores
struct{ 
	struct semaphore list[MAXSEM];
	struct spinlock lock;
	int amountcsems; // Amount of semaphores that all system are using
}semtable;


//function that return 1 if current process is sempahore sem_id's owner
//otherwise return 0
static int
isSemOwnerProc(int sem_id)
{
	int i;

	//search if sem_id is a semaphore of current process
	for(i=0;i<MAXSEMPROC;i++)
		if(proc->sems[i] == sem_id)
			return 1;
		
		return 0;
}

//init semaphores
void 
seminit()
{
	int i;

	initlock(&semtable.lock,"semtable");
	semtable.amountcsems=0;
	//initialice semaphores references counter
	for(i=0;i<MAXSEM;i++)
		semtable.list[i].refcount=0;
}

//set id to proc's semaphores selectors and increment process semaphores conunter
/*
Errors 
	-2 the process has the maximun number of semaphores allowed
*/	
static int 
addsem_to_pselectors(int sem_id)
{
	int i;
	int result = -2;

	for(i=0;i<MAXSEMPROC;i++)
		if(proc->sems[i] == -1){	
			proc->sems[i] = sem_id; 
			proc->amountcsems++;//increment amount process's semaphores
			result = 0;
			break;
		}
	return result;
}



//Obtain a semaphore descriptor
/*
return values
	>=0 method finish ok
	-1 sem_id is not in use, should be created
	-2 the process has the maximun number of semaphores allowed 
	-3 the system don't have any free semaphore.
	-4 wrong sem_id parameter, sem_id cannot be less than -1 
*/
int 
semget(int sem_id, int init_value)
{
	switch(sem_id){
		case -1://proccess request to create a semaphore   
				if (proc->amountcsems == MAXSEMPROC) 
                    //return code for notify that procces has the maximum amount of semaphores allowed
					return -2;
				
				acquire(&semtable.lock);				
				if (semtable.amountcsems == MAXSEM){ 
					release(&semtable.lock);
					return -3;//return code for notify that OS has the maximum amount of semaphores allowed	
				}
				
				//else search a free semaphore and return its id
				int i;
				
				for(i=0;i<MAXSEM;i++)
					if(semtable.list[i].refcount==0){
						semtable.list[i].refcount++; //update refcount to 1
						semtable.list[i].value=init_value; //set semaphore initial value
						semtable.amountcsems++;//increment system busy semaphores
						release(&semtable.lock);
	                	
	                	//find free space in process semaphores selectors and set it.
						if (addsem_to_pselectors(i) != 0)
	                	 	panic("SO intent to assign a semaphore to a process and all process descriptors are busy");
						return i;
					}

				release(&semtable.lock);
				panic("there aren't available semaphores but OS intent to give one");				
        
        default:
        		if (sem_id < -1)
        			return -4;	
                if (semtable.list[sem_id].refcount == 0)
                	return -1;
	       		
	       		//add semaphore id to proc semaphores id's
                int result;

                result=addsem_to_pselectors(sem_id);
                if(result == 0){
				//lock semtable for update semaphore with id=sem_id atomically 
					acquire(&semtable.lock);
			    		semtable.list[sem_id].refcount++;						
					release(&semtable.lock);	
					result=sem_id;
				}
                return result;
	}
}

//Free a semaphore
/*
Errors
	-1 semaphore wasn't obtained by current process 
	-2 sem_id isn't a correct semaphore id
*/
int 
semfree(int sem_id)
{
	int i;

	if(sem_id<0 || sem_id>=MAXSEM)
		return -2;//sem_id isn't a correct semaphore id
	
	//search if sem_id is a semaphore of current process
	for(i=0;i<MAXSEMPROC;i++)
		if(proc->sems[i] == sem_id){
			 proc->sems[i]=-1;//free process semaphore selectors
			 proc->amountcsems--;//update process semaphores counter
			 break;	
		}

	if(i==MAXSEMPROC) return -1; //sempahore isn't obtained by current process
	acquire(&semtable.lock);
	if(semtable.list[sem_id].refcount==0) panic("SO intent do decrement a semaphore refcount but this hasn't been create");
	semtable.list[sem_id].refcount--;//update semaphores references
	if(semtable.list[sem_id].refcount==0)
	  semtable.amountcsems--;//decrement system busy semaphores
	release(&semtable.lock);
	
	return 0;
}

//Decrement semaphore value if it is possible, else sleep current proc
/*
Error
	-1 sem_id isn't a semaphore used by current process
	-2 sem_i<0 so sem_id isn't a correct semaphore id.
*/
int 
semdown(int sem_id)
{

	if(sem_id<0) return -2;

	if(!isSemOwnerProc(sem_id)) return -1;//notify that it isn't a process's semaphore
	
	acquire(&semtable.lock);
	struct semaphore *s=&semtable.list[sem_id];
	for(;;){
		if (s->value > 0){
			 s->value--;
			 release(&semtable.lock);
			 break;
		}else
		  sleep(s,&semtable.lock);
	}
	return 0;
}

//Increment semaphore value and wake up procces that are waiting for current semaphore
/*
Error
	-1 sem_id isn't a semaphore used by current process
	-2 sem_i<0 so sem_id isn't a correct semaphore id.
*/
int 
semup(int sem_id)
{

	if(sem_id<0) return -2;
			
	if(!isSemOwnerProc(sem_id)) return -1; //notify that it isn't a process's semaphore
	
	acquire(&semtable.lock);
	struct semaphore *s= &semtable.list[sem_id];	
	s->value++;//update semaphore value
	release(&semtable.lock);
	wakeup(s);//wake up process that are waiting for s
	return 0;
}

int 
copy_sem_descriptor(struct proc* pfrom,struct proc* pto)
{
	int i;

	if (pfrom == 0 || pto == 0)
			return -1;
	for (i = 0; i<MAXSEMPROC; i++){
  	if (pfrom->sems[i] != -1){
  			pto->sems[i] = pfrom->sems[i];
  			pto->amountcsems++;
 				acquire(&semtable.lock);
 				semtable.list[pfrom->sems[i]].refcount++;
 				release(&semtable.lock);
  	}
 }
 return 0;
}