/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 * modified from link https://www.cs.ucsb.edu/~cs170/projects/nachos/threads/hashchain-submit.cc
 **/

//Reference hashchain.cc implementation for proj1b. Note that lots of yields are inserted to help uncover synchronization bugs

#include "system.h" //for currentThread->Yield()

#include <iostream>
#include <unistd.h>
#include "hashchain.h"

#ifdef HASHCHAIN
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#else
#define START_READ() lock[key % TABLE_SIZE].startRead()
#define END_READ() lock[key % TABLE_SIZE].doneRead()
#define START_WRITE() lock[key % TABLE_SIZE].startWrite()
#define END_WRITE() lock[key % TABLE_SIZE].doneWrite()
#endif

#define YIELD() currentThread->Yield()



LinkedHashEntry:: LinkedHashEntry(int key, int value) {
  this->k = key;
  this->v = value;
  this->next = NULL;
}

int 
LinkedHashEntry:: getKey() {
  return k;
}
int 
LinkedHashEntry:: getValue() {
  return v;
}

void 
LinkedHashEntry:: setValue(int value) {
  this->v= value;
}


LinkedHashEntry * 
LinkedHashEntry:: getNext() {
  return next;
}

void 
LinkedHashEntry:: setNext(LinkedHashEntry *new_next) {
  this->next = new_next;
}


const int TABLE_SIZE = 128;

HashMap::HashMap() {
  table = new LinkedHashEntry*[TABLE_SIZE];
  for (int i = 0; i < TABLE_SIZE; i++)
    table[i] = NULL;

#ifndef HASHCHAIN
  lock = new RWLock[TABLE_SIZE]();
#endif
}

int 
HashMap::get(int key) { //TODO: make this function threadsafe
  //return _get(key);
  START_READ();
  int value=_get(key);
  END_READ();
  return value;
}

void 
HashMap::put(int key, int value) { //TODO: make this function threadsafe
  START_WRITE();
  _put(key,value);
  END_WRITE();
}


int 
HashMap::_get(int key) { //internal get() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  //usleep(1);
  if (table[hash] == NULL) {
    YIELD();
    return -1;
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry != NULL && entry->getKey() != key) {
      entry = entry->getNext();
      YIELD();
    }
    if (entry == NULL) {
      YIELD();
      return -1;
    } else { 
      YIELD();
      return entry->getValue();
    }
  }
  return -1; //should never get here (just for complaining compilers)
}

void
HashMap::_put(int key, int value) { //internal get() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  if (table[hash] == NULL) {
    YIELD();
    table[hash] = new LinkedHashEntry(key, value);
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      entry->setValue(value);
    } else {
      YIELD();
      entry->setNext(new LinkedHashEntry(key, value));
    }
  }
  YIELD();
}


void
HashMap::remove(int key) { //TODO: make this function threadsafe
  int hash = (key % TABLE_SIZE);
  START_WRITE();
  if (table[hash] != NULL) {
    YIELD();
    LinkedHashEntry *prevEntry = NULL;
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      prevEntry = entry;
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      if (prevEntry == NULL) {
        LinkedHashEntry *nextEntry = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        table[hash] = nextEntry;
      } else {
        LinkedHashEntry *next = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        prevEntry->setNext(next);
      }
    }
  }
  END_WRITE();
}

void
HashMap::increment(int key, int value) { //TODO: make this function threadsafe
  START_WRITE();
  _put(key,_get(key)+value);
  END_WRITE();
}

HashMap::~HashMap() {
  for (int i = 0; i < TABLE_SIZE; i++)
    if (table[i] != NULL) {
      LinkedHashEntry *prevEntry = NULL;
      LinkedHashEntry *entry = table[i];
      while (entry != NULL) {
        prevEntry = entry;
        entry = entry->getNext();
        delete prevEntry;
      }
    }
  delete[] table;
#ifndef HASHCHAIN
  delete[] lock;
#endif
}

