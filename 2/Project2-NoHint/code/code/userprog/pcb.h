// pcb.h
//  The process control block is a data structure used to keep track of a
//	single user process. For now, each process has an ID, a parent and a
//	pointer to its kernel thread.

#ifndef PCB_H
#define PCB_H

//#include "system.h"
#include "useropenfile.h"
#define ERR_NOT_FINISHED -1
#define ERR_NO_PARENT_PID -1
#define MAX_FILES 32


class Thread;

class PCB {

public:
    PCB(int Pid, int pPid);
    ~PCB();

    int pid;            // Process ID
    int parentPid;      // Parent's Process ID
    Thread *thread;     // Kernel thread that controls this process

    /* Changes from original Source code */
    int exitStatus;
    UserOpenFile** openFiles;       // track the Process open files

    int getExitStatus();
    void setExitStatus(int stat);

    int addOpenFile(UserOpenFile* openFile);
    void closeOpenFile(int FileDescriptor);
    
    UserOpenFile* getOpenFile(int FileDescriptor){
        return openFiles[FileDescriptor];
    }

    PCB* forkHelp(int pid, int parentPid);
    
    /* end of changes */
};

#endif // PCB_H
