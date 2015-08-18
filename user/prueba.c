#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

/*
Example of producer-consumer. 
Producer write numbers in a share file
and consumer reads these.  
*/
int main(int argc, char *argv[]){
	int * x;
	x= (int*) 0x7FFF7000;
	printf(0,"shared memory descriptor: %d\n",shm_create(0x2000));
	*x = 10;
	//x= (int*) 0x7FFF8001;
	//*x = 2;
	//x = (int*) 0x7FFF8000;
	printf(0,"value %d\n",*x);
	
	exit();
}