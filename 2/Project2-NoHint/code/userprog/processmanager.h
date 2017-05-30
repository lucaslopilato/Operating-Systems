// processmanager.h
//  The process manager is responsible for keeping track of the currently
//	running processes.
//

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "bitmap.h"
#include "pcb.h"
#include "../threads/synch.h"

#define MAX_PROCESSES 8

class ProcessManager {

public:
    ProcessManager();
    ~ProcessManager();

    int allocPid();                     // Allocate a new PID
    void freePid(int pid);              // Free an allocated PID
    
    /* Additions to original code */
    void trackPCB(int pid, PCB *pcb);   // Allocate an accompanying PCB to the pid
    PCB* getPCB(int pid);
    int getPIDsFree(){
        return pids->NumClear();
    }

    int waitStateOn(int cPID, int pPID);
    void waitStateOnChild(int pPID);

    void broadcastOnExit(int pid);

    Lock *pmLock;
    /* End of changes */

private:
    BitMap *pids;                       // Table to keep track of PIDs

    /* Additions to original code */
    PCB **pcbList;                     //Table for PCBs
    Condition **conditionList;            //Table for broadcasting exit signals
    Lock **lockList;                   //Locks for Condition Variables
    /* End of Changes */
};

#endif // PROCESSMANAGER_H
