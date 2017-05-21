/*
 * MemoryManager header
 *
 * Used to facilitate contiguous virtual memory.  Utilizes the provided bitmap
 * data structure to store the state of pages in the page table.
*/

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "bitmap.h"
#include "synch.h"

class Lock; 

class MemoryManager {

    public:
        MemoryManager();
        ~MemoryManager();
        int getPage();                // allocates the first clear page
        void clearPage(int);          // frees the page at specified index
        int getNumFreePages();        // returns the number of free pages

        Lock *lock;

    private:
        BitMap* physPageAllocation;
};

#endif // MEMORY_MANAGER_H
