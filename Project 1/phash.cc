/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 **/


#include <pthread.h>
#include <iostream>
#include "phash.h"
#include "rwlock.h"

LinkedHashEntry:: LinkedHashEntry(int key, int value) {
            this->key = key;
            this->value = value;
            this->next = NULL;
      }

int 
LinkedHashEntry:: getKey() {
            return key;
      }
int 
LinkedHashEntry:: getValue() {
            return value;
      }
 
void 
LinkedHashEntry:: setValue(int value) {
            this->value = value;
      }
 

LinkedHashEntry * 
LinkedHashEntry:: getNext() {
            return next;
      }
 
void 
LinkedHashEntry:: setNext(LinkedHashEntry *next) {
            this->next = next;
      }


const int TABLE_SIZE = 128;
 
HashMap::HashMap() {
            table = new LinkedHashEntry*[TABLE_SIZE];
            // Set Lock
            this->rwlock = RWLock();
            for (int i = 0; i < TABLE_SIZE; i++)
                  table[i] = NULL;
      }

int 
HashMap::get(int key) {
            // Begin Reading through the hash table for value at key given
            // Lock the table so that no writes occur when obtaining values
            this->rwlock.startRead();
            int hash = (key % TABLE_SIZE);
            if (table[hash] == NULL){
                  this->rwlock.doneRead(); // Nothing to look at so unlock
                  return -1;
            }
            else {
                  LinkedHashEntry *entry = table[hash];
                  while (entry != NULL && entry->getKey() != key)
                        entry = entry->getNext();

                  // Finished loop through the hashtable element so unlock and return result
                  if (entry == NULL){
                        this->rwlock.doneRead();
                        return -1;
                  }
                  else{
                        this->rwlock.doneRead();
                        return entry->getValue();
                  }
            }
      }
 
void 
HashMap::put(int key, int value) {
            // Begin to write to the hashtable so we need to lock out readers
            this->rwlock.startWrite();
            int hash = (key % TABLE_SIZE);
            if (table[hash] == NULL)
                  table[hash] = new LinkedHashEntry(key, value);
            else {
                  LinkedHashEntry *entry = table[hash];
                  while (entry->getNext() != NULL)
                        entry = entry->getNext();
                  if (entry->getKey() == key)
                        entry->setValue(value);
                  else
                        entry->setNext(new LinkedHashEntry(key, value));
            }
            // done with updating the table element so unlock
            this->rwlock.doneWrite();
      }
 

void
HashMap:: remove(int key) {
            // Begin to write(remove) elements from hashtable so lock out readers
            this->rwlock.startWrite();
            int hash = (key % TABLE_SIZE);
            if (table[hash] != NULL) {
                  LinkedHashEntry *prevEntry = NULL;
                  LinkedHashEntry *entry = table[hash];
                  while (entry->getNext() != NULL && entry->getKey() != key) {
                        prevEntry = entry;
                        entry = entry->getNext();
                  }
                  if (entry->getKey() == key) {
                        if (prevEntry == NULL) {
                             LinkedHashEntry *nextEntry = entry->getNext();
                             delete entry;
                             table[hash] = nextEntry;
                        } else {
                             LinkedHashEntry *next = entry->getNext();
                             delete entry;
                             prevEntry->setNext(next);
                        }
                  }
            }
            this->rwlock.doneWrite(); // unlock
      }
 
HashMap:: ~HashMap() {
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
      }
