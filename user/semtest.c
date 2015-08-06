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
			fseek(fd,0); //move logical cursor to the begin
			write(fd,&i,sizeof(i));
			printf(1,"producer %d\n",i );
			semup(empty);
		}
	}else{//consumer
		int c;
		int i;
		for(i=0;i<12;i++){			
			semdown(empty);
			fseek(fd,0); //move logical cursor to the begin
			read(fd,&c,sizeof(c));
			printf(1,"consumer %d\n",c);			
			semup(full);
		}
		wait();
	}
	int c;
	fseek(fd,0);
	read(fd,&c,sizeof(c));
	printf(1,"Finaly %d\n",c);
	close(fd);//close file
	exit();
}
	