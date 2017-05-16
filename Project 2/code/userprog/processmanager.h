// processmanager.h
//  The process manager is responsible for keeping track of the currently
//	running processes.
//

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "bitmap.h"

#define MAX_PROCESSES 8

class ProcessManager {

public:
    ProcessManager();
    ~ProcessManager();

    int allocPid();         // Allocate a new PID
    void freePid(int pid);  // Free an allocated PID

private:
    BitMap *pids;           // Table to keep track of PIDs

};

#endif // PROCESSMANAGER_H
