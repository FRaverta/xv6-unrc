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
/*
int main(int argc, char *argv[]){
	int d;
	void* x;
	int * i;
	d=shm_create(0x2000);
	printf(0,"shared memory descriptor: %d\n",d);
	shm_get(d, (void*) &x);
	printf(0," after get in proc: %d\n", ((int) x));
	x= x + 0x1FF8; // x - x + 0x0FFF, x+ 0x1000 - x+ 0x1FFF
	i  = (int *) x;
	*i = 10;
	printf(0,"value %d\n",*((int*)  x));
	
	if(fork()==0){
		void* y;

		shm_get(d, (void*) &y);
		d= (int) y;
		printf(0,"after get in child:%d\n", d);
		printf(0,"value in child %d\n",* ((int*) (y + 0x1FF8)));
	}

	wait();
	
	exit();
}
*/

int main(int argc, char *argv[]){
	int d;
	void* x;
	void* xx;
	int i = 0;
	int size=0x2000;
	void* last_pos;

	d=shm_create(size);
	printf(0,"shared memory descriptor: %d\n",d);
	shm_get(d, (void*) &x);
	xx = x;	 
	printf(0,"After get in proc: %d\n", ((int) x));
	last_pos = size + x;
	for(; x < last_pos; x += 4)
		*((int *) x) = i++;
	
	
	if(fork()==0){		
		//void* xx;
		int ii;
		int* value;
		int aux;

		aux= 0;//shm_get(d, (void*) &xx);
		if(aux<0)
			printf(0,"Child Error: %d in get\n",aux);
		else{
			printf(0,"After get in child: %d\n",((int) xx));
			printf(0,"Printing in child\n");
			for(ii=0; ii < size; ii += 4){
				value = xx + ii;
				printf(0,"%d\n",*value);
			}
			printf(0,"End Printing in child\n");		
			aux=shm_close(d);
			printf(0,"Close in child %d \n",aux);		
		}		
	}
	wait();
	shm_close(d);
	exit();
}