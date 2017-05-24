// processmanager.cc
//

#include "processmanager.h"

//----------------------------------------------------------------------
// ProcessManager::ProcessManager
//  Create a new process manager to manager MAX_PROCESSES processes.
//  ** Implement **
//----------------------------------------------------------------------

ProcessManager::ProcessManager()
{
    pids = new BitMap(MAX_PROCESSES);
    pmLock = new Lock("ProcessManagerLock");
    pcbList = new PCB*[MAX_PROCESSES];
    lockList = new Lock*[MAX_PROCESSES];
    conditionList = new Condition*[MAX_PROCESSES];

    for(int i=0; i < MAX_PROCESSES; i++){
        pcbList[i] = NULL;
        lockList[i] = new Lock("Exit Lock List");
        conditionList[i] = new Condition("Exit Signal List");
    }
}

//----------------------------------------------------------------------
// ProcessManager::~ProcessManager
//  Deallocate a process manager.
//  ** Implement **
//----------------------------------------------------------------------

ProcessManager::~ProcessManager()
{
    delete pids;
    for(int i=0; i < MAX_PROCESSES; i++){
        if(pcbList[i] != NULL)
            delete pcbList[i];

        delete lockList[i];
        delete conditionList[i];
    }

    delete[] pcbList;
    delete[] lockList;
    delete[] conditionList;
    delete pmLock;
}

//----------------------------------------------------------------------
// ProcessManager::allocPid
//  Allocate an unused PID to be used by a process.
//
//  ** Implement **
//----------------------------------------------------------------------

int ProcessManager::allocPid()
{
    int pid = pids->Find();
    return pid;
}

//----------------------------------------------------------------------
// ProcessManager::freePid
//  Deallocate a PID that is in use so that it can be allocated again by
//  another process.
//
//  ** Implement **
//----------------------------------------------------------------------

void ProcessManager::freePid(int pid)
{
    pids->Clear(pid);
    if(pcbList[pid] != NULL)
        delete pcbList[pid];

    pcbList[pid] = NULL;
}


//----------------------------------------------------------------------
// ProcessManager::trackPCB(int pid, PCB *pcb)
//  ** Implement **
//----------------------------------------------------------------------

void ProcessManager::trackPCB(int pid, PCB *pcb)
{
    this->pcbList[pid] = pcb;
}


//----------------------------------------------------------------------
// ProcessManager::getPCB(int pid)
//  ** Implement **
//----------------------------------------------------------------------

PCB* ProcessManager::getPCB(int pid)
{
    return pcbList[pid];
}


//----------------------------------------------------------------------
// ProcessManager::waitStateOn(int childPID, int parentPID)
//  ** Implement **
//----------------------------------------------------------------------

int ProcessManager::waitStateOn(int childPID, int parentPID)
{
    int exitStatus;
    lockList[childPID]->Acquire();
    fprintf(stderr, "Process %d waiting on process %d\n", parentPID, childPID);

    while(pcbList[childPID]->getExitStatus() == ERR_NOT_FINISHED){
        conditionList[childPID]->Wait(lockList[childPID]);
    }

    exitStatus = pcbList[childPID]->getExitStatus();
    fprintf(stderr, "Process %d finsihed waiting for process %d\n",parentPID, childPID);

    // Deallocate memory when currThread == parentThread
    if(parentPID == pcbList[childPID]->parentPid){
        freePid(childPID);
    }

    lockList[childPID]->Release();
    return exitStatus;
}


//----------------------------------------------------------------------
// ProcessManager::waitStateOnChild(int parentPID)
//  ** Implement **
//----------------------------------------------------------------------

void ProcessManager::waitStateOnChild(int parentPID)
{
    for(int i=0; i < MAX_PROCESSES; i++){
        if(pcbList[i] != NULL && pcbList[i]->parentPid == parentPID)
            waitStateOn(i, parentPID);
    }
}


//----------------------------------------------------------------------
// ProcessManager::broadcastOnExit(int pid)
//  ** Implement **
//----------------------------------------------------------------------

void ProcessManager::broadcastOnExit(int pid)
{
    lockList[pid]->Acquire();
    conditionList[pid]->Broadcast(lockList[pid]);
    lockList[pid]->Release();
}