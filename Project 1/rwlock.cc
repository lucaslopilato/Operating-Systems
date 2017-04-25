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
        //Enter Queue
        pthread_mutex_lock(&locks[1]);
        WR++;
        pthread_mutex_unlock(&locks[1]);

        //Get lock for active writers
        pthread_mutex_lock(&locks[2]);

        //Wait until there are no active writers
        while(AW != 0)
            pthread_cond_wait(&okToRead, &locks[2]);

        //Acquire lock for AR
        pthread_mutex_lock(&locks[0]);
        AR++;
        pthread_mutex_unlock(&locks[0]);

        //Exit Queue
        pthread_mutex_lock(&locks[1]);
        WR--;
        pthread_mutex_unlock(&locks[1]);

    #else
        pthread_mutex_lock(&this->lock);
    #endif

}

void RWLock::doneRead(){
    #ifdef RWLOCK

        //Decrement Number of readers
        pthread_mutex_lock(&locks[0]);
        AR--;

        if(AR == 0){
            pthread_cond_signal(&okToWrite);
            pthread_cond_signal(&okToRead);
        }

        //Release Active Readers
        pthread_mutex_unlock(&locks[0]);

    #else
        pthread_mutex_unlock(&this->lock);
    #endif
}

void RWLock::startWrite(){
    #ifdef RWLOCK

        //Enter Queue
        pthread_mutex_lock(&locks[3]);
        WW++;
        pthread_mutex_unlock(&locks[3]);

        //Acquire Active Writers
        pthread_mutex_lock(&locks[2]);
        while(AW != 0)
            pthread_cond_wait(&okToWrite, &locks[2]);

        //Acquire Active Readers
        while(AR != 0)
            pthread_cond_wait(&okToWrite, &locks[0]);

        //Increment Active Writers
        AW++;

        //Unlock Acquired Locks
        pthread_mutex_unlock(&locks[2]);
        pthread_mutex_unlock(&locks[0]);

        //Exit Queue
        pthread_mutex_lock(&locks[3]);
        WW--;
        pthread_mutex_unlock(&locks[3]);

    #else
        pthread_mutex_lock(&this->lock);
    #endif
}
void RWLock::doneWrite(){
    #ifdef RWLOCK

        //Decrement Number of Active Writers
        pthread_mutex_lock(&locks[2]);
        AW--;

        if(AW == 0){
            pthread_cond_signal(&okToWrite);
            pthread_cond_signal(&okToRead);
        }

        //Release Active Readers
        pthread_mutex_unlock(&locks[2]);

    #else
        pthread_mutex_unlock(&this->lock);
    #endif
}
