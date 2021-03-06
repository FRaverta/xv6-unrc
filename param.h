#define NPROC         64  // maximum number of processes
#define KSTACKSIZE  4096  // size of per-process kernel stack
#define NCPU           8  // maximum number of CPUs
#define NOFILE        16  // open files per process
#define NFILE        100  // open files per system
#define NINODE        50  // maximum number of active i-nodes
#define NDEV          10  // maximum major device number
#define ROOTDEV        1  // device number of file system root disk
#define MAXARG        32  // max exec arguments
#define MAXOPBLOCKS   10  // max # of blocks any FS op writes
#define LOGSIZE        (MAXOPBLOCKS*3)  // max data sectors in on-disk log
#define NBUF           (MAXOPBLOCKS*3)  // size of disk block cache
#define QUANTUM						  4  // number of ticks before context switching
#define LEVELS         4  // amount of levels in multilevel feedback table
#define MAXSEM         20 // amount of semaphores in the OS
#define MAXSEMPROC     8  // amount of semaphores available per process
#define PAGSTACKPROC   3  // maximun amount of memory pages for stack per process
#define MAXSHM         10 // maximun amount of shared memory blocks in OS
#define MAXSHMPROC     4  // maximum amount of shared memory blocks per process in OS 
#define MAXSHMPBLOCK   4  // maximum amount of pages in shared memory block       
