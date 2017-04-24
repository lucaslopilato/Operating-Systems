//#include<stdio.h>
//#include<pthread.h>
#include<semaphore.h>

class RWLock{
private:
#ifdef RWLOCK
	pthread_rwlock_t rwlock ;
#else 
	pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER; 
#endif
 
public:
    	RWLock();
    	~RWLock();
    //Reader
    	void startRead();
    	void doneRead();
    // Writer
    	void startWrite();
    	void  doneWrite();
};

