// processmanager.cc
//

#include "processmanager.h"

//----------------------------------------------------------------------
// ProcessManager::ProcessManager
//  Create a new process manager to manager MAX_PROCESSES processes.
//----------------------------------------------------------------------

ProcessManager::ProcessManager()
{
    pids = new BitMap(MAX_PROCESSES);
}

//----------------------------------------------------------------------
// ProcessManager::~ProcessManager
//  Deallocate a process manager.
//----------------------------------------------------------------------

ProcessManager::~ProcessManager()
{
    delete pids;
}

//----------------------------------------------------------------------
// ProcessManager::allocPid
//  Allocate an unused PID to be used by a process.
//
//  For now do nothing.
//----------------------------------------------------------------------

int ProcessManager::allocPid()
{
    return 0;
}

//----------------------------------------------------------------------
// ProcessManager::freePid
//  Deallocate a PID that is in use so that it can be allocated again by
//  another process.
//
//  For now do nothing.
//----------------------------------------------------------------------

void ProcessManager::freePid(int pid)
{
}
