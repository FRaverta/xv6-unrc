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
	int fd;
	fd=open("number.txt",O_CREATE|O_RDWR);
	close(fd);
	int full= semget(-1,1);
	int empty= semget(-1,0);
	if(fork()==0){//producer		
		int i;
		semget(full,1);
		semget(empty,0);
		for(i=0;i<12;i++){			
			semdown(full);
			fd=open("number.txt",O_RDWR);//reopen for reset file descriptor
			write(fd,&i,sizeof(i));
			printf(1,"producer %d\n",i );
			semup(empty);

		}
	}else{//consumer
		int c;
		int i;
		for(i=0;i<12;i++){			
			semdown(empty);
			fd=open("number.txt",O_RDWR);//reopen for reset file descriptor		
			read(fd,&c,sizeof(c));
			printf(1,"consumer %d\n",c);			
			semup(full);
		}
		wait();
	}
	close(fd);//close file
	exit();
}
	