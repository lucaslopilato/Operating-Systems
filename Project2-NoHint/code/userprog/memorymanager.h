// memorymanager.h
//  A class to manage the allocation of physical memory. Recall that a
//  process's virtual memory is divided into fixed size pages. The machine's
//	physical memory is divided into frames which are the same size as pages.
//	The page table is used to map each page of virtual memory to a frame of
//	physical memory.
//
//  The job of the memory manager is to keep track of which frames of
//	physical memory are free. When allocating space for a process the
//	AddrSpace class should be able to invoke the memory manager to claim the
//	a frame for each page in the process's virtual address space using the
//	allocFrame method. When a process is destroyed it should relinquish its
//	frames using freeFrame.

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "bitmap.h"

class MemoryManager {

public:
    MemoryManager();
    ~MemoryManager();

    int allocFrame();           // allocate a free frame to a process
    void freeFrame(int frame);  // free an allocated frame

private:
    BitMap *frames;             // Use a bitmap to keep track of memory usage

};

#endif // MEMORYMANAGER_H
