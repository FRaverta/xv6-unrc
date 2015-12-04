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
test1()
{
	int block1,block2,block3,block4;
	void* addr1;void* addr2;void* addr3;void* addr4;

	printf(0,"Executing test1\n");	
	
	block1 = shm_create(4096);
	block2 = shm_create(4096);
	block3 = shm_create(4096);
	block4 = shm_create(4096);

	if(block1 < 0 || block2 < 0 || block3 < 0 || block4 < 0){
		printf(1,"****Error creating shared memory blocks*****\n");
		return;
	}

	if (shm_get(block1,&addr1) < 0) {printf(0,"error getting block\n");return;}
	if (shm_get(block2,&addr2) < 0) {printf(0,"error getting block\n");return;}
	if (shm_get(block3,&addr3) < 0) {printf(0,"error getting block\n");return;}

	//printf(0,"\n %d\n %d\n %d\n \n",(int)addr1,(int)addr2,(int)addr3);
	shm_close(block1);
	shm_get(block4,&addr4);
	if(addr4 >= addr2 || addr4 >= addr3){printf(1,"****Error when in alloc orden*****\n");return;}
	
	if(! (addr4 < addr2 && addr2 < addr3)){printf(1,"****Error when in alloc orden*****\n");return;}

	if ((block1 = shm_create(4096)) < 0){printf(1,"****Error recreating shared memory blocks*****\n"); return;}
	
	if (shm_get(block1,&addr1) < 0) {printf(0,"error getting block\n");return;}
	//printf(0,"\n %d\n %d\n %d\n %d\n \n",(int)addr4,(int)addr2,(int)addr3,(int)addr1);
	if(! (addr4 < addr2 && addr2 < addr3 && addr3 < addr1)){printf(1,"****Error when in alloc orden*****\n");return;}

	shm_close(block2);
	shm_close(block3);

	if ((block3 = shm_create(4096)) < 0){printf(1,"****Error recreating shared memory blocks*****\n"); return;}
	if (shm_get(block3,&addr3) < 0) {printf(0,"error getting block\n");return;}
	//printf(0,"\n %d\n %d\n %d\n \n",(int)addr4,(int)addr3,(int)addr1);
	if(! (addr4 < addr3 && addr3 < addr1)){printf(1,"****Error when in alloc orden*****\n");return;}
	shm_close(block1);shm_close(block3);shm_close(block4);
	printf(0, "End Executing test1\n");
}

void
test2(){
	printf(0,"Executing test2\n");	

	int block1,block2,block3,block4;
	void* addr1;void* addr2;void* addr3;void* addr4;
	
	block1 = shm_create(4096*4);
	block2 = shm_create(4096*4);
	block3 = shm_create(4096*4);
	block4 = shm_create(4096*4);	

	if (shm_get(block1,&addr1) < 0) {printf(0,"error getting block\n");return;}
	if (shm_get(block2,&addr2) < 0) {printf(0,"error getting block\n");return;}
	if (shm_get(block3,&addr3) < 0) {printf(0,"error getting block\n");return;}
	if (shm_get(block4,&addr4) < 0) {printf(0,"error getting block\n");return;}

	if ( fork() == 0 ){
		*((int*)addr1) = 1;
		*((int*)addr2) = 2;
		*((int*)addr3) = 3;
		*((int*)addr4) = 4;
		shm_close(block1);shm_close(block2);shm_close(block3);shm_close(block4);
		exit();
	}else{
		wait();
		if(*((int*)addr1) != 1) {printf(1,"****Error getting values*****\n");return;}
		if(*((int*)addr2) != 2) {printf(1,"****Error getting values*****\n");return;}
		if(*((int*)addr3) != 3) {printf(1,"****Error getting values*****\n");return;}
		if(*((int*)addr4) != 4) {printf(1,"****Error getting values*****\n");return;}
		shm_close(block1);shm_close(block2);shm_close(block3);shm_close(block4);
		printf(0, "End Executing test2\n");
	}
}

void
test3()
{
	int d;
	void* x;
	void* xx;
	int i = 0;
	int psize = 4096;
	int size = psize * 3;
	void* last_pos;

	printf(0,"Executing test3\n");
	d=shm_create(size);
	shm_get(d, (void*) &x);
	xx = x;	
	last_pos = size + x;
	for(; x < last_pos; x += 4)		
		*((int *) x) = i++;
	
	
	if(fork()==0){		
		int ii;
		int* value;
		int last = -1;


		value =  xx;			
		for(ii=0; ii < size; ii += 4){				
			if (*value <= last ){printf(1, "******Error reading in child test1******\n"); exit();}
			else value++;
		}
		if (shm_close(d) < 0) {printf(1, "******Error close shm block in child test1******\n"); exit();}		
		exit();
	}else{		
	
		wait();

		if (shm_close(d) < 0)
			printf(1,"******Error close shm block in father test1******\n");	
		printf(0, "End Executing test3\n");
	}
}


int
main(int argc, char *argv[])
{

	test1();
	test2();
	test3();
	exit();
}
