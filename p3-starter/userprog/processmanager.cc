/*
 * ProcessManager implementation
 *
 * This class keeps track of the current PCBs and manages their creation and
 * deletion.
*/

#include "processmanager.h"
#include "utility.h" // definition of NULL
#include "system.h" // definition of processManagerLock

//-----------------------------------------------------------------------------
// ProcessManager::ProcessManager
//     Constructor that initializes the PCB list, condition list, lock list, 
//     and address space list to MAX_PROCESSES size.
//-----------------------------------------------------------------------------
ProcessManager::ProcessManager() : processesBitMap(MAX_PROCESSES) {

    pcbList = new PCB*[MAX_PROCESSES];
    conditionList = new Condition*[MAX_PROCESSES];
    lockList = new Lock*[MAX_PROCESSES];
    addrSpaceList = new AddrSpace*[MAX_PROCESSES];

    numAvailPIDs = MAX_PROCESSES;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        lockList[i] = NULL;
        conditionList[i] = NULL;
        pcbList[i] = NULL;
        addrSpaceList[i] = NULL;
    }
}

//-----------------------------------------------------------------------------
// ProcessManager::~ProcessManager
//     Destructor that frees up the memory allocated by the constructor.
//-----------------------------------------------------------------------------

ProcessManager::~ProcessManager() {
    delete [] pcbList;
    delete [] conditionList;
    delete [] lockList;
    delete [] addrSpaceList;
}

//-----------------------------------------------------------------------------
// ProcessManager::getPID
//     Returns the first free PID found.
//-----------------------------------------------------------------------------

int ProcessManager::getPID() {

    int newPID = processesBitMap.Find();
    processesWaitingOnPID[newPID] = 0;
    processesWaitingOnPID[newPID]++;
    return newPID;
}

//-----------------------------------------------------------------------------
// ProcessManager::clearPID
//     Decrements the number of processes waiting on this guy before its PID
//     can be reused, and frees the respective PID for re-use if possible.
//-----------------------------------------------------------------------------

void ProcessManager::clearPID(int pid) {

    processesWaitingOnPID[pid]--;
    if (processesWaitingOnPID[pid] == 0) {
        processesBitMap.Clear(pid);
    }
}

//-----------------------------------------------------------------------------
// ProcessManager::addProcess
//     Add a new process to the list.
//-----------------------------------------------------------------------------

void ProcessManager::addProcess(PCB* pcb, int pid) {
    pcbList[pid] = pcb;
}

//-----------------------------------------------------------------------------
// ProcessManager::join
//     Allows process A to wait on another process B in order to perform a join
//     system call.
//-----------------------------------------------------------------------------

void ProcessManager::join(int pid) {

    Lock* lockForOtherProcess = lockList[pid];
    if (lockForOtherProcess == NULL) {
        lockForOtherProcess = new Lock("");
        lockList[pid] = lockForOtherProcess;
    }

    Condition* conditionForOtherProcess = conditionList[pid];
    if (conditionForOtherProcess == NULL) {
        conditionForOtherProcess = new Condition("");
        conditionList[pid] = conditionForOtherProcess;
    }

    lockForOtherProcess->Acquire();

    processesWaitingOnPID[pid]++;
    conditionForOtherProcess->Wait(lockForOtherProcess);
    processesWaitingOnPID[pid]--; 

    if (processesWaitingOnPID[pid] == 0) {
        processesBitMap.Clear(pid);
    }
    lockForOtherProcess->Release();
}

//-----------------------------------------------------------------------------
// ProcessManager::broadcast
//     Lets everyone know that the process has changed status so that other 
//     processes can act accordingly if they are waiting.
//-----------------------------------------------------------------------------

void ProcessManager::broadcast(int pid) {

    Lock* lock = lockList[pid];
    Condition* condition = conditionList[pid];
    pcbStatuses[pid] = pcbList[pid]->status;

    if (condition != NULL) { // something is waiting on this process
        lock->Acquire();
        condition->Broadcast(lock);
        lock->Release();
    }
}

//-----------------------------------------------------------------------------
// ProcessManager::getStatus
//     Returns the status of a given process
//-----------------------------------------------------------------------------

int ProcessManager::getStatus(int pid) {
    if (processesBitMap.Test(pid) == 0) {
        return -1; // process finished
    }
    return pcbStatuses[pid];
}
//-----------------------------------------------------------------------------
// ProcessManager::isAllFinished
//     Returns 1 if all processes other than 0 have finished
//     Otherwise 0
//-----------------------------------------------------------------------------

int ProcessManager::isAllFinished() { //Is all finished except 0?
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (processesBitMap.Test(i) != 0) {
        	return 0; // process i not finished
    	}
    }
    return 1;
}
