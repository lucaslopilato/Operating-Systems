#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef RWLOCK_H
#define RWLOCK_H

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
    void signal();

    #ifdef RWLOCK
        //Counter Locks
        pthread_mutex_t lock;

        int AR; //Number of Active Readers
        int WR; //Number of Waiting Readers
        int AW; //Number of Active Writers
        int WW; //Number of Waiting Writers

        //Condition Vars
        pthread_cond_t okToRead, okToWrite;

    #else 
        pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER; 
    #endif
};

#endif //RWLOCK_H


