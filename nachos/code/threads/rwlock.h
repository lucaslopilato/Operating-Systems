#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <synch.h>


#ifndef _RWLOCK_H_
#define _RWLOCK_H_

class RWLock{
 
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

    // Precompilers to change RWLock for each task ()
    #ifndef P1_RWLOCK
    #ifdef  P1_SEMAPHORE
        // Task 1: mutex -> Semaphore
        Semaphore* semaphore;
    #else
        // Task 2: nachOS Lock
        Lock* lock;
    #endif //P1_Semaphore
    #else
        
        int AR; //Number of Active Readers
        int WR; //Number of Waiting Readers
        int AW; //Number of Active Writers
        int WW; //Number of Waiting Writers
        //Condition Vars
        Condition* okToRead; 
        Condition* okToWrite;
        Lock* lock;
    #endif
    
    
};

#endif //RWLOCK_H


