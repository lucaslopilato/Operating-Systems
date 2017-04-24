//#include<stdio.h>
//#include<pthread.h>
#include<semaphore.h>

#ifndef _RWLOCK_H
#define _RWLOCK_H_

class RWLock{
private:
#ifdef RWLOCK

    int AR = 0; //Number of active readers
    int WR = 0; //Number of waiting readers
    int AW = 0; //Number of active writers
    int WW = 0; //Number of waiting writers
    pthread_// Condition okToread
    // Condition okToWrite

#else 
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
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

