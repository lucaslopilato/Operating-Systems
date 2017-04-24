/**********
 *Read Write Lock
 *Implementation Based on https://www.cs.ucsb.edu/~cs170/slides/5synchAdv.pdf
 *
 *
 *
 **********/

#include "rwlock.h"

RWLock::RWLock() : locked(false){}
RWLock::~RWLock();

void RWLock::startRead(){

}

void RWLock::doneRead();

void RWLock::startWrite();
void RWLock::doneWrite();
