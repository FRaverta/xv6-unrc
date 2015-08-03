#include "types.h"
#include "user.h"
int main(int argc, char *argv[]){
 int i;

 for(i=1; i<10000000 ; i++)
   set_priority(3);
 exit();
}


