/**********
 *Read Write Lock
 *Implementation Based on https://www.cs.ucsb.edu/~cs170/slides/5synchAdv.pdf
 *
 *
 *
 **********/

#include "rwlock.h"

RWLock::RWLock(){
    #ifdef RWLOCK
        //Initialize Locks for Read/Write counters
        for(int i=0; i<4; i++)
            pthread_mutex_init(&this->locks[i], NULL);

        //Initialize Counter Variables
        this->AR = 0; //locks[0]
        this->WR = 0; //locks[1]
        this->AW = 0; //locks[2]
        this->WW = 0; //locks[3]

        //Initialize Condition Variables
        pthread_cond_init(&this->okToRead, NULL);
        pthread_cond_init(&this->okToWrite, NULL);
    #else
        pthread_mutex_init(&this->lock, NULL);
    #endif
}

RWLock::~RWLock(){
    #ifdef RWLOCK
        for(int i=0; i<4; i++)
            pthread_mutex_destroy(&this->locks[i]);

        pthread_cond_destroy(&this->okToRead);
        pthread_cond_destroy(&this->okToWrite);
    #else
        pthread_mutex_destroy(&this->lock);
    #endif
}

void RWLock::startRead(){
    #ifdef RWLOCK
    #else
        pthread_mutex_lock(&this->lock);
    #endif

}

void RWLock::doneRead(){
    #ifdef RWLOCK
    #else
        pthread_mutex_unlock(&this->lock);
    #endif
}

void RWLock::startWrite(){
    #ifdef RWLOCK
    #else
        pthread_mutex_lock(&this->lock);
    #endif
}
void RWLock::doneWrite(){
    #ifdef RWLOCK
    #else
        pthread_mutex_unlock(&this->lock);
    #endif
}
