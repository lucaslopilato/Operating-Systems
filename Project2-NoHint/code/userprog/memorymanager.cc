// memorymanager.cc
//

#include "memorymanager.h"
#include "system.h"

//----------------------------------------------------------------------
// MemoryManager::MemoryManager
//  Create a new memory manager and initialize it with the size of
//  physical memory.
//----------------------------------------------------------------------

MemoryManager::MemoryManager()
{
    // Create a bitmap with one bit for each frame
    frames = new BitMap(NumPhysPages);
    mmLock = new Lock("MemoryManagerLock");     // added
}

//----------------------------------------------------------------------
// MemoryManager::~MemoryManager
//  Deallocate a memory manager.
//----------------------------------------------------------------------

MemoryManager::~MemoryManager()
{
    delete frames;
    delete mmLock;                              // added
}

//----------------------------------------------------------------------
// MemoryManager::allocFrame
//  Allocate a free frame of physical memory to be used by a process.
//  ** Implement **
//----------------------------------------------------------------------

int MemoryManager::allocFrame()
{
    int fIndex = frames->Find();
    if(fIndex == -1){
        DEBUG('m', "Couldn't find a page in the page table.\n");
        ASSERT(FALSE);
    } else{
        return fIndex;
    }
}

//----------------------------------------------------------------------
// MemoryManager::freeFrame
//  Deallocate a frame that is in use so that it can be allocated by
//  another process.
//  ** Implement **
//----------------------------------------------------------------------

void MemoryManager::freeFrame(int frame)
{
    frames->Clear(frame);
}
