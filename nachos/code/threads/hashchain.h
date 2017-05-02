/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 * modfied from link https://www.cs.ucsb.edu/~cs170/projects/nachos/threads/hashchain-submit.h
 **/

#include <semaphore.h>

//the macro HASHCHAIN disables synchronization
//#define HASHCHAIN

#ifndef HASHCHAIN
#include "rwlock.h"
#endif

class LinkedHashEntry {
private:
      int k;
      int v;
      LinkedHashEntry *next;
public:
      LinkedHashEntry(int key, int value); 
      int getKey(); 
      int getValue();
      void setValue(int value);
 
      LinkedHashEntry *getNext(); 
      void setNext(LinkedHashEntry *new_next); 
};


class HashMap {
private:
      LinkedHashEntry **table;
#ifndef HASHCHAIN
      RWLock *lock;
#endif

      int _get(int key); //internal get function (not threadsafe)
      void _put(int key, int value); //internal put function (not threadsafe)
public:
      HashMap(); 
      int get(int key);  //get value of key (or return -1)
      void put(int key, int value);  //put key,value pair
      void remove(int key); //delete key
      void increment(int key, int value); //increase key by value

      ~HashMap(); 
};