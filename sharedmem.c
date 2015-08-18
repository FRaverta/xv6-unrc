#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

struct shmblock{
  uint amount_of_pages; //amount of pages that block assign really < MAXSHMPBLOCK
  uint references; //amount of process that refer this block <= NPROC
  char* pages[MAXSHMPBLOCK]; //page's physical address
};

struct {
  struct spinlock shmlock;
  struct shmblock blocks[MAXSHM]; 
}shmtable;

int
shm_init()
{
  int i,j;

  initlock(&shmtable.shmlock,"shmtable");
  
  for(i = 0; i<MAXSHM; i++){
  	 shmtable.blocks[i].amount_of_pages = 0;
  	 shmtable.blocks[i].references = -1;
  	 //free references to physical address (for care)
    for(j=0 ; j<MAXSHMPBLOCK ;j++)
      shmtable.blocks[i].pages[j] = 0;
  }

  return 0;
}

int
shm_freeblock(int key)
{
  int i;

  acquire(&shmtable.shmlock);
  
  if(shmtable.blocks[key].references < 0)
    panic("free shared memory block not busy");

  shmtable.blocks[key].references = -1;
  //free references to physical address (for care)
  for(i=0;i<MAXSHMPBLOCK;i++)
    shmtable.blocks[key].pages[i] = 0;

  release(&shmtable.shmlock);
  return 0;
}

int
shm_assign_page_to_block(int key,char* mem)
{
  acquire(&shmtable.shmlock);
  shmtable.blocks[key].pages[shmtable.blocks[key].amount_of_pages] = mem;
  shmtable.blocks[key].amount_of_pages++;
  release(&shmtable.shmlock);

  return 0;
}

//Obtain virtual address of block wich id  
//is key in addr.
// 
int
shm_get(int key, void **addr)
{

		return 0;
}

//Free shared memory block.
//It has been assigned to proc previously.
int
shm_close(int key)
{
		return 0;
}

//Create a shared memory block 
//Errors:
//  -1: non available resources for current request
//  -2: size bad: it is less or equal to 0
//  -3: size bad: the request block is bigger than the maximum block size
//  -4: current process has all shared memory blocks already busy.
//  -5: OS has all shared memory blocks already busy.
int
shm_create(int size)
{
		int proc_shmb_id;//current proc shared memory block id
		int os_shmb_id; //operative system shared memory block id
		int amount_of_pages; //amount of pages request
		uint from;
		uint until; 
		
		if (size <= 0){
			cprintf("Shared memory block create error: the block's size must be lest than 0\n");
			return -2;
		}

		size = PGROUNDUP(size);
		amount_of_pages = size / PGSIZE; 
	 if( amount_of_pages > MAXSHMPBLOCK ){
	 		cprintf("Shared memory block create error: the block's size must be at up %d \n",MAXSHMPBLOCK * PGSIZE);
	 		return -3;
	 	}

	 	//check if current process can have a new shared memory block. 
	 	//In other words, if it has an free shared memory slot
	 	for(proc_shmb_id = 0; proc_shmb_id < MAXSHMPROC; proc_shmb_id++)	
	 		 if(proc->shm_blocks[proc_shmb_id] == -1)
	 			  break;
	 	if(proc_shmb_id == MAXSHMPROC){
	 			//report error
	 			cprintf("Shared memory block create error: Proc: %d has all shared memory blocks already busy\n",proc->pid);
	 			return -4;	
	 	}

	//tour so's shared memory block.
	 	acquire(&shmtable.shmlock);
	 	for(os_shmb_id = 0; os_shmb_id < MAXSHM; os_shmb_id++)
	 		 if(shmtable.blocks[os_shmb_id].references == -1){
	 		   shmtable.blocks[os_shmb_id].references = 1; //reserve the shared memory slot
	 		   break;
	 		  }
	 	release(&shmtable.shmlock);
	 	if(os_shmb_id == MAXSHM){
	 			//report error
	 			cprintf("Shared memory block create error: XV6 already assign all shared memory blocks\n");
	 			return -5;	
	 	}

	//get physical pages
	 	
	//assign block
	 	//map virtual address to physical pages
	 	from = SHMMEMORYBASE + (proc_shmb_id * MAXSHMPBLOCK * PGSIZE);
	 	until= from + (amount_of_pages * PGSIZE);
	 	if(shm_allocuvm(proc->pgdir,from,until,os_shmb_id) == 0)
	 			cprintf("error en shm_allocuvm");
	 	proc->shm_blocks[proc_shmb_id] = os_shmb_id;

	//return block's id
	 	return os_shmb_id;
}

