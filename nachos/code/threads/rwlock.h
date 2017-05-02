#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef _RWLOCK_H_
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
    #ifdef RWLOCK
        void mem();        //Counter Locks
        //pthread_mutex_t lock;

        int AR; //Number of Active Readers
        int WR; //Number of Waiting Readers
        int AW; //Number of Active Writers
        int WW; //Number of Waiting Writers

        //Condition Vars
        pthread_cond_t okToRead = PTHREAD_COND_INITIALIZER; 
        pthread_cond_t okToWrite = PTHREAD_COND_INITIALIZER;
    #endif
        pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    
};

#endif //RWLOCK_H


