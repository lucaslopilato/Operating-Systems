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
        //Initialize Lock for Read/Write counters
        //pthread_mutex_init(&this->lock, NULL);

        //Initialize Counter Variables
        this->AR = 0; //locks[0]
        this->WR = 0; //locks[1]
        this->AW = 0; //locks[2]
        this->WW = 0; //locks[3]

        //Initialize Condition Variables
        //pthread_cond_init(&this->okToRead, NULL);
        //pthread_cond_init(&this->okToWrite, NULL);
    #endif
        //pthread_mutex_init(&this->lock, NULL);
    
}

RWLock::~RWLock(){
    
    #ifdef RWLOCK
        //pthread_mutex_destroy(&this->lock);

        pthread_cond_destroy(&this->okToRead);
        pthread_cond_destroy(&this->okToWrite);
    #endif
    pthread_mutex_destroy(&this->lock);
}

void RWLock::startRead(){
    pthread_mutex_lock(&this->lock);
    #ifdef RWLOCK
        //Enter Queue
        //pthread_mutex_lock(&lock);
        //WR++;

        //Wait until there are no active or waiting writers
        while(WW != 0 && AW != 0){
            this->WR++;
            pthread_cond_wait(&okToRead, &lock);
            this->WR--;
        }

        AR++;

        //WR--;
        pthread_mutex_unlock(&lock);

    //#else
    //    pthread_mutex_lock(&this->lock);
    #endif

}

void RWLock::doneRead(){
    
    #ifdef RWLOCK

        //Decrement Number of readers
        pthread_mutex_lock(&lock);
        AR--;

        //Signal
        if(WW > 0 && AR == 0 ) // AW == 0)  //don't need active writers if some are waiting
            pthread_cond_signal(&okToWrite);
        //else if(AW == 0)
        //    pthread_cond_signal(&okToRead); //writer priority

        //Release Active Readers
        pthread_mutex_unlock(&this->lock);

    #else
        pthread_mutex_unlock(&this->lock);
    #endif
}

void RWLock::startWrite(){
    pthread_mutex_lock(&this->lock);
    #ifdef RWLOCK

        //Enter Queue
        //pthread_mutex_lock(&lock);
        //WW++;

        while(AW != 0 && AR != 0){
            WW++;
            pthread_cond_wait(&okToWrite, &lock);
            WW--;
        }

        //Increment Active Writers
        AW++;

        //Exit Queue
        //WW--;
        pthread_mutex_unlock(&lock);

    //#else
    //    pthread_mutex_lock(&this->lock);
    #endif
}

void RWLock::doneWrite(){
    #ifdef RWLOCK

        //Decrement Number of Active Writers
        pthread_mutex_lock(&lock);
        AW--;

        if(WW > 0 )// && AR == 0 && AW == 0) // you finished writing no other writers or readers
            pthread_cond_signal(&okToWrite);
        else if(WR > 0)//(AW == 0) // no waiting writers, let them read
            pthread_cond_broadcast(&okToRead);

        pthread_mutex_unlock(&lock);        

    #else
        pthread_mutex_unlock(&this->lock);
    #endif
}
