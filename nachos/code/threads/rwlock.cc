/**********
 *Read Write Lock
 *Implementation Based on https://www.cs.ucsb.edu/~cs170/slides/5synchAdv.pdf
 *
 **********/

#include "rwlock.h"

// Precompiler directives to change behavior (derived from example on "Update 4/28" )

#ifndef P1_RWLOCK
#ifdef P1_SEMAPHORE
    //Task 1 (using semaphore in place of mutex)
    RWLock::RWLock() {
        semaphore = new Semaphore("Semaphore_Lock",1); 
    }
    RWLock::~RWLock() { 
        delete semaphore;
    }
    //NOTE Task 1: startRead should be the same as startWrite
    void RWLock::startRead() {
        semaphore->P(); 
    } 
    void RWLock::startWrite() {
        semaphore->P();
    }
    //NOTE Task 1: doneRead should be the same as doneWrite
    void RWLock::doneRead() { 
        semaphore->V();
    } 
    void RWLock::doneWrite() {
        semaphore->V(); 
    }
#else
    //Task 2 (using NACHOS Lock)
    RWLock::RWLock() { 
        lock = new Lock("nachOS_Lock");
    }
    RWLock::~RWLock() { 
        delete lock;
    }
    //NOTE Task 2: startRead should be the same as startWrite
    void RWLock::startRead() {
        lock->Acquire();             //Acquire() implemented in Task 2 (Wesley)
    } 
    void RWLock::startWrite() {
        lock->Acquire();            //Acquire() implemented in Task 2 (Wesley) 
    }
    //NOTE Task 2: doneRead should be the same as doneWrite
    void RWLock::doneRead() {
        lock->Release();             //Release implemented in Task 2 (Wesley)
    } 
    void RWLock::doneWrite() {
        lock->Release();             //Release() implemented in Task 2 (Wesley)
    }
#endif
#else
    //Task 3 code (full rwlock)
    RWLock::RWLock() {
        AW = 0; // Active Writers
        WW = 0; // Waiting Writers
        AR = 0; // Active Readers
        WR = 0; // Waiting Readers
        okToRead = new Condition("okToRead");
        okToWrite = new Condition("okToWrite");
        lock = new Lock("RWLock");
    }
    RWLock::~RWLock() {
        delete okToWrite;
        delete okToRead;
        delete lock; 
    }
    
    // similar logic to Part A
    
    void RWLock::startRead() {
        lock->Acquire();                    //Acquire() implemented in Task 3 (Lucas)
        this->WR++;
        while((this->WW + this->AW) > 0)    // No writers waiting or actvively writing
            okToRead->Wait(lock);           //Wait() implemented in Task 3 (Lucas)
        this->WR--;
        this->AR++;
        lock->Release();                    //Release() implemented in Task 3 (Lucas)
    }

    void RWLock::doneRead() { 
        lock->Acquire();                    //Acquire() implemented in Task 2 (Lucas)
        AR--;
        if(this->AR == 0 && this->WW > 0)   // no active readers or waiting writers
            okToWrite->Signal(lock);        //Signal() implemented in Task 3 (Lucas)
        lock->Release();                    //Release() implemented in Task 3 (Lucas)
    }

    void RWLock::startWrite() {
        lock->Acquire();                    //Acquire() implmented in Task 3 (Lucas)
        this->WW++;
        while((this->AW + this->AR) >0)     // No one is actively writing or reading
            okToWrite->Signal(lock);
        this->WW--;
        this->AW++;
        lock->Release();                     //Release() implemented in Task 3 (Lucas)
    }
    void RWLock::doneWrite() { 
        lock->Acquire();                    //Acquire() implmented in Task 3 (Lucas)
        this->AW--;
        if(this->WW > 0)
            okToWrite->Signal(lock);        //Signal() implemented in Task 3 (Lucas)
        else if(this->WR > 0)
            okToRead->Broadcast(lock);      //Broadcast() implemented in Task 3 (Lucas)
        lock->Release();                    //Release() implemented in Task 3 (Lucas)
    }
#endif


/*          Implementation of RWLock from Part A
RWLock::RWLock(){
    
    #ifdef RWLOCK
        //Initialize Counter Variables
        this->AR = 0; //locks[0]
        this->WR = 0; //locks[1]
        this->AW = 0; //locks[2]
        this->WW = 0; //locks[3]

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
            
            //printf("read lock wait\n");
            //mem();
            pthread_cond_wait(&this->okToRead, &this->lock);
            
        }

        this->WR--;

        this->AR++;

        //printf("Read start\n");
        //mem();
        pthread_mutex_unlock(&this->lock);

    #endif

}
*/
/*

void RWLock::doneRead(){
    
    #ifdef RWLOCK

        //Decrement Number of readers
        pthread_mutex_lock(&this->lock);
        this->AR--;

        //Signal
        if(this->AR == 0 && WW > 0) // AW == 0)  //don't need active writers if some are waiting
            pthread_cond_signal(&this->okToWrite);
        //else if(this->AW == 0)
        //    pthread_cond_signal(&this->okToRead); //writer priority

        /*
            if(AR == 0 and WW > 0)
                signal
        */

        //Release Active Readers
        //printf("Read Lock Unlocked\n");
        //mem();
//    #endif

//    pthread_mutex_unlock(&this->lock);

//}
/*
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

        //printf("write lock acquired\n");
        //mem();

        pthread_mutex_unlock(&this->lock);

    #endif
}

void RWLock::doneWrite(){
    #ifdef RWLOCK

        //Decrement Number of Active Writers
        pthread_mutex_lock(&this->lock);
        this->AW -= 1;

        if(this->WW > 0 )// && AR == 0 && AW == 0) // you finished writing no other writers or readers
            pthread_cond_signal(&this->okToWrite);
        else //if(this->WR > 0)//(AW == 0) // no waiting writers, let them read
            pthread_cond_broadcast(&this->okToRead);
        
        //printf("write unlock\n"); 
        //mem();   
    #endif
        
    pthread_mutex_unlock(&this->lock);
}

#ifdef RWLOCK
void RWLock::mem(){
    //printf("AR: %d, WR: %d, AW: %d, WW: %d\n", AR, WR, AW, WW);

}
#endif
*/