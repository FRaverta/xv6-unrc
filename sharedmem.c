#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

#define INVALID_START_VA 0xFFFFFFFF

struct shmblock{
  uint amount_of_pages; //amount of pages that block assign really < MAXSHMPBLOCK
  uint references; //amount of process that refer this block <= NPROC
  char* pages[MAXSHMPBLOCK]; //page's physical address + KERNBASE 
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

//This method must be called with unique acess to shmtable.
int
shm_freeblock(int key)
{
  int i;

  
  if(shmtable.blocks[key].references < 0)
    panic("free shared memory block not busy");

  shmtable.blocks[key].references = -1;

  shmtable.blocks[key].amount_of_pages = 0;

  //free references to physical address (for care)
  for(i = 0; i < MAXSHMPBLOCK ; i++)
    shmtable.blocks[key].pages[i] = 0;
  return 0;
}

//This method must be called with unique acess to shmtable.
int
shm_assign_page_to_block(int key,char* mem)
{
  shmtable.blocks[key].pages[shmtable.blocks[key].amount_of_pages] = mem;
  shmtable.blocks[key].amount_of_pages++;

  return 0;
}

//Obtain virtual address of block wich id  
//is key in addr.
// -1: current process has all shared memory blocks already busy.
// -2: Sharem memory block with key descriptor hasn't been created.
// -3: Process haven't enought adresses space.
int
shm_get(int key, void **addr)
{
  int proc_shmb_id;
  uint start_va;

  //check if key is a id for some Shared Memory Block that has been already created.
  //tour so's shared memory block.
    acquire(&shmtable.shmlock);
    if (shmtable.blocks[key].references == -1){
        release(&shmtable.shmlock);
        //report error
        cprintf("Shared memory block with descriptor: %d hasn't been created\n",key);
        return -2;  
    }
    
  //search for process shared memory block descriptor
  for(proc_shmb_id = 0; proc_shmb_id < MAXSHMPROC; proc_shmb_id++)  
     if(proc->shm_blocks[proc_shmb_id].block_id == key)
        break;

  if(proc_shmb_id == MAXSHMPROC){
    //check if current process can have a new shared memory block. 
    for(proc_shmb_id = 0; proc_shmb_id < MAXSHMPROC; proc_shmb_id++)  
       if(proc->shm_blocks[proc_shmb_id].block_id == -1)
          break;

    if(proc_shmb_id == MAXSHMPROC){
      release(&shmtable.shmlock);
      //report error
      cprintf("Shared memory block create error: Proc: %d has all shared memory blocks already busy\n",proc->pid);
      return -1;  
    }else{
          //take process shared memory free block descriptor
          proc->shm_blocks[proc_shmb_id].block_id = key;
          proc->shm_blocks[proc_shmb_id].start_va = (char*) INVALID_START_VA;
          shmtable.blocks[key].references++;
    }
  } 
  
  //check if current shared memory block is mapped
  if(proc->shm_blocks[proc_shmb_id].start_va == ((char*) INVALID_START_VA)){
    //map proc's virtual addresses to shm block physical addresses. 
    if ( (start_va = shm_allocuvm(cpu->proc->pgdir,shmtable.blocks[key].pages, shmtable.blocks[key].amount_of_pages)) >= 0)
      proc->shm_blocks[proc_shmb_id].start_va = (char*) start_va;
    else{
      release(&shmtable.shmlock);
      return -3;
    }
  }

  release(&shmtable.shmlock);
  *addr = proc->shm_blocks[proc_shmb_id].start_va; 
	return 0;
}

//Free shared memory block.
//It has been assigned to proc previously.
// -1: Current process can't get the block key. 
// -2: Invalid key
int
shm_close(int key)
{
    int id;

    if(key<0 || key>MAXSHM)
      return -2;

    for(id = 0; id < MAXSHMPROC; id++)
      if(proc->shm_blocks[id].block_id == key)
        break;
    
    if (id == MAXSHMPROC)
      return -1;

    acquire(&shmtable.shmlock);
    if( shmtable.blocks[key].references < 0)
      panic("shm_close: Shared memory slot without references");
   
    shmtable.blocks[key].references--;
    
    if ( shmtable.blocks[key].references == 0){
      unmappages(proc->pgdir, proc->shm_blocks[id].start_va, shmtable.blocks[key].amount_of_pages * PGSIZE , 1);
      //free SO's shared memory slot
      shmtable.blocks[key].amount_of_pages = 0;
      shmtable.blocks[key].references = -1;            
    }else
      unmappages(proc->pgdir, proc->shm_blocks[id].start_va, shmtable.blocks[key].amount_of_pages * PGSIZE , 0);
    
    release(&shmtable.shmlock);    
    proc->shm_blocks[id].block_id = -1; //free procee's shared memory slot
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
		int so_shmb_id, proc_shmb_id; //operative system shared memory block id
		int amount_of_pages; //amount of pages request
		char* mem;
		uint assg_mem , i;
		
		if (size <= 0){
			cprintf("Shared memory block create error: the block's size must be less than 0\n");
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
       if(proc->shm_blocks[proc_shmb_id].block_id == -1)
          break;
    if(proc_shmb_id == MAXSHMPROC){
        //report error
        cprintf("Shared memory block create error: Proc: %d has all shared memory blocks already busy\n",proc->pid);
        return -4;  
    }

	 //tour so's shared memory block.
	 	acquire(&shmtable.shmlock);
	 	for(so_shmb_id = 0; so_shmb_id < MAXSHM; so_shmb_id++)
	 		 if(shmtable.blocks[so_shmb_id].references == -1){
	 		   shmtable.blocks[so_shmb_id].references = 1; //reserve the shared memory slot
	 		   break;
	 		  }
	 	
	 	if(so_shmb_id == MAXSHM){
        release(&shmtable.shmlock);
	 			//report error
	 			cprintf("Shared memory block create error: XV6 already assign all shared memory blocks\n");
	 			return -5;	
	 	}

	//get physical pages
		for(assg_mem = 0 ; assg_mem < size; assg_mem += PGSIZE){
    		mem = kalloc();
    		if(mem == 0){
      			cprintf("allocuvm out of memory\n");

    			//Free pages that have been assigned already, if there are.
			    for(i = 0; i < shmtable.blocks[so_shmb_id].amount_of_pages ; i++)
       		    	kfree(shmtable.blocks[so_shmb_id].pages[i]);
       		    
       		    //Free shared memory block    
		      	shm_freeblock(so_shmb_id);
            release(&shmtable.shmlock);
      			
            return -1;
    		}

    		memset(mem, 0, PGSIZE);    		    
    		//modified so shared block table
    		shm_assign_page_to_block(so_shmb_id,mem);    
    	}
  release(&shmtable.shmlock);

  proc->shm_blocks[proc_shmb_id].block_id = so_shmb_id;
  proc->shm_blocks[proc_shmb_id].start_va = (char*) INVALID_START_VA;
	
  //return block's id
	 	return so_shmb_id;
}

//copy father shared memory descriptors and virtual mapping
//-1 invalid arguments
int
copy_shm_descriptor(struct proc* pfrom,struct proc* pto){
  int i;
  int block_id;//SO's shared memory block descriptor
  uint start_va;

  if(pfrom == 0 || pto == 0)
    return -1;

  for(i=0; i < MAXSHMPROC; i++){
    block_id = pfrom->shm_blocks[i].block_id;  
    if(block_id != -1){
      start_va = (uint) pfrom->shm_blocks[i].start_va;
      pto->shm_blocks[i].block_id = block_id;
      pto->shm_blocks[i].start_va = (char*) start_va;
      
      acquire(&shmtable.shmlock);
      shmtable.blocks[block_id].references++;
      
      if(start_va != INVALID_START_VA)
          shm_from_allocuvm(pto->pgdir,start_va, shmtable.blocks[block_id].pages, shmtable.blocks[block_id].amount_of_pages);      

      release(&shmtable.shmlock);
    }
  }
  
  return 0;
}
