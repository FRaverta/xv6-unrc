#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
//with only a page for stack, throw a Stack Owerflow since n_invo= 127.
int 
f(int n_invo)
{
	if (n_invo==1)
		return n_invo;
	else
		return f(n_invo-1);
}

int 
main(int argc,char *argv[])
{
	if(argc!=2)
		printf(1,"Use recursion int:Number of invocation to a recursive function(>0)\n");	
	else{ 
		int v;
		v=atoi(argv[1]);
		if(v<1)
			printf(1,"Use recursion int:Number of invocation to a recursive function(>0)\n");	
		else{				
			printf(1,"Running program with %d recursion calls\n",v);
			f(v);			
		}
	}
	exit();
}

