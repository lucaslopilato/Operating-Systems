//#include<stdio.h>
#include <pthread.h>
#include <queue>
#include <semaphore.h>

#ifndef _RWLOCK_H
#define _RWLOCK_H_

class RWLock{

/*
#ifdef RWLOCK
#else 
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
#endif*/
 
public:
    	RWLock();
    	~RWLock();
    //Reader
    	void startRead();
    	void doneRead();
    // Writer
    	void startWrite();
    	void  doneWrite();

private:
    std::queue<pthread_t*> readers, writers;
    bool locked;
};

