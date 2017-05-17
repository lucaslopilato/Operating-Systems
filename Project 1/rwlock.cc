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
        //Initialize Counter Variables
        this->AR = 0; //locks Active Reader
        this->WR = 0; //locks Waiting Reader
        this->AW = 0; //locks Active Writer
        this->WW = 0; //locks Waiting Writer
    #endif
}

RWLock::~RWLock(){
    #ifdef RWLOCK
        pthread_cond_destroy(&this->okToRead);
        pthread_cond_destroy(&this->okToWrite);
    #endif
    pthread_mutex_destroy(&this->lock);
}

void RWLock::startRead(){
    pthread_mutex_lock(&this->lock);
    #ifdef RWLOCK
        this->WR++;
        //Wait until there are no active or waiting writers
        while(WW!=0|| AW!= 0){
            pthread_cond_wait(&this->okToRead, &this->lock);
        }
        this->WR--;
        this->AR++;
        pthread_mutex_unlock(&this->lock);
    #endif
}

void RWLock::doneRead(){
    #ifdef RWLOCK
        //Decrement Number of readers
        pthread_mutex_lock(&this->lock);
        this->AR--;
        //Signal
        if(this->AR == 0 && WW > 0) // AW == 0)  
            pthread_cond_signal(&this->okToWrite);
    #endif
    pthread_mutex_unlock(&this->lock);
}

void RWLock::startWrite(){
    pthread_mutex_lock(&this->lock);
    #ifdef RWLOCK
        this->WW++;
        while(this->AW != 0 || this->AR != 0){
            //printf("write wait\n");
            //mem();
            pthread_cond_wait(&this->okToWrite, &this->lock);
        }
        this->WW--;
        //Increment Active Writers
        this->AW += 1;
        pthread_mutex_unlock(&this->lock);
    #endif
}

void RWLock::doneWrite(){
    #ifdef RWLOCK
        //Decrement Number of Active Writers
        pthread_mutex_lock(&this->lock);
        this->AW -= 1;
        if(this->WW > 0 ) // you finished writing no other writers or readers
            pthread_cond_signal(&this->okToWrite);
        else // no waiting writers, let them read
            pthread_cond_broadcast(&this->okToRead);
    #endif
    pthread_mutex_unlock(&this->lock);
}

#ifdef RWLOCK
void RWLock::mem(){
    //printf("AR: %d, WR: %d, AW: %d, WW: %d\n", AR, WR, AW, WW);

}
#endif
