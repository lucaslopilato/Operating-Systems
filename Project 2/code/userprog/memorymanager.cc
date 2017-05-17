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
}

//----------------------------------------------------------------------
// MemoryManager::~MemoryManager
//  Deallocate a memory manager.
//----------------------------------------------------------------------

MemoryManager::~MemoryManager()
{
    delete frames;
}

//----------------------------------------------------------------------
// MemoryManager::allocFrame
//  Allocate a free frame of physical memory to be used by a process.
//----------------------------------------------------------------------

int MemoryManager::allocFrame()
{
    return 0;
}

//----------------------------------------------------------------------
// MemoryManager::freeFrame
//  Deallocate a frame that is in use so that it can be allocated by
//  another process.
//----------------------------------------------------------------------

void MemoryManager::freeFrame(int frame)
{
}
