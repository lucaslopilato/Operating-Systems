/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 **/


#include <pthread.h>
#include <iostream>
#include <unistd.h>
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
            // make an array of rwlocks for fine grained implementation
            rwlocksArray = new RWLock[TABLE_SIZE];
            for (int i = 0; i < TABLE_SIZE; i++){
                  table[i] = NULL;
            }
            for(int i = 0; i < TABLE_SIZE; i++){      
              rwlocksArray[i] = RWLock(); // this->rwlock = RWLock();

            }
            
      }

int 
HashMap::get(int key) {
            //usleep(1);
            int hash = (key % TABLE_SIZE);
            rwlocksArray[hash].startRead(); //Lock the specific index of the hashtable 
            //this->rwlock.startRead();     // (allows access to the rest of the table)
            if (table[hash] == NULL){
                  rwlocksArray[hash].doneRead(); // Nothing to look at so unlock
                  //this->rwlock.doneRead();
                  return -1;
            }
            else {
                  LinkedHashEntry *entry = table[hash];
                  while (entry != NULL && entry->getKey() != key)
                        entry = entry->getNext();

                  // Finished loop through the hashtable element so unlock and return result
                  if (entry == NULL){
                        rwlocksArray[hash].doneRead(); //this->rwlock.doneRead();
                        return -1;
                  }
                  else{
                        rwlocksArray[hash].doneRead(); //this->rwlock.doneRead();
                        return entry->getValue();
                  }
            }
      }
 
void 
HashMap::put(int key, int value) {
            // Begin to write to the hashtable so we need to lock out readers
            int hash = (key % TABLE_SIZE);
            rwlocksArray[hash].startWrite(); // only lock the one element
            //this->rwlock.startWrite(); // Coarse
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
            rwlocksArray[hash].doneWrite(); //this->rwlock.doneWrite();
      }
 

void
HashMap:: remove(int key) {
            // Begin to write(remove) elements from hashtable so lock out readers
            int hash = (key % TABLE_SIZE);
            rwlocksArray[hash].startWrite(); // only lock one element
            //this->rwlock.startWrite();    // Coarse
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
            rwlocksArray[hash].doneWrite(); // unlock
            //this->rwlock.doneWrite();     //Coarse
            
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
            delete[] rwlocksArray; //only for fine grain
}


