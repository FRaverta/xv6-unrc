//A  semaphore
struct semaphore{
	int value; //current value
	int refcount; //amount of references to this semaphore
};