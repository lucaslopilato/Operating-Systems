/**********
 *Read Write Lock
 *Implementation Based on https://www.cs.ucsb.edu/~cs170/slides/5synchAdv.pdf
 *
 *
 *
 **********/

#include "rwlock.h"

RWLock::RWLock(){
    //Initialize Lock for Read/Write counters
    //pthread_mutex_init(&this->lock, NULL);
    #ifdef RWLOCK


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
            printf("read lock wait\n");
            mem();

            pthread_cond_wait(&this->okToRead, &this->lock);
            this->WR--;
        }

        this->AR++;

        //WR--;
        printf("Read start\n");
        mem();
        pthread_mutex_unlock(&this->lock);

    //#else
    //    pthread_mutex_lock(&this->lock);
    #endif

}

void RWLock::doneRead(){
    
    #ifdef RWLOCK

        //Decrement Number of readers
        pthread_mutex_lock(&this->lock);
        this->AR--;

        //Signal
        if(this->AR == 0) // AW == 0)  //don't need active writers if some are waiting
            pthread_cond_signal(&this->okToWrite);
        else if(this->AW == 0)
            pthread_cond_signal(&this->okToRead); //writer priority

        //Release Active Readers
        printf("Read Lock Unlocked\n");
        mem();
    #endif

    pthread_mutex_unlock(&this->lock);

}

void RWLock::startWrite(){
    pthread_mutex_lock(&this->lock);
    #ifdef RWLOCK

        //Enter Queue
        //pthread_mutex_lock(&lock);
        //WW++;

        while(this->AW != 0 && this->AR != 0){
            this->WW++;
            printf("write wait\n");
            mem();
            pthread_cond_wait(&this->okToWrite, &this->lock);
            this->WW--;
        }

        //Increment Active Writers
        this->AW += 1;

        printf("write lock acquired\n");
        mem();

        //Exit Queue
        //WW--;
        pthread_mutex_unlock(&this->lock);

    //#else
    //    pthread_mutex_lock(&this->lock);
    #endif
}

void RWLock::doneWrite(){
    #ifdef RWLOCK

        //Decrement Number of Active Writers
        pthread_mutex_lock(&this->lock);
        this->AW -= 1;

        if(this->WW > 0 )// && AR == 0 && AW == 0) // you finished writing no other writers or readers
            pthread_cond_signal(&this->okToWrite);
        else if(this->WR > 0)//(AW == 0) // no waiting writers, let them read
            pthread_cond_broadcast(&this->okToRead);
        
        printf("write unlock\n"); 
        mem();   
    #endif
        
    pthread_mutex_unlock(&this->lock);
}

#ifdef RWLOCK
void RWLock::mem(){
    printf("AR: %d, WR: %d, AW: %d, WW: %d\n", AR, WR, AW, WW);

}
#endif
