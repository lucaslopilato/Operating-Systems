// pcb.cc
//

#include "pcb.h"
#include <cstddef>

//----------------------------------------------------------------------
// PCB::PCB
//  Allocate a new process control block and initialize it with process
//  ID and a parent.
//----------------------------------------------------------------------

PCB::PCB(int Pid, int pPid)
{
    this->pid = Pid;
    this->parentPid = pPid;
    /* added to original code */
    this->exitStatus = ERR_NOT_FINISHED;
    this->openFiles = new UserOpenFile*[MAX_FILES];
    for(int i=0; i < MAX_FILES; i++){
        openFiles[i] = NULL;
    }
}

//----------------------------------------------------------------------
// PCB::~PCB
//  Deallocate a process control block.
//  ** Implement This **
//----------------------------------------------------------------------

PCB::~PCB()
{
    for(int i=0; i < MAX_FILES; i++){
        if(openFiles[i] != NULL)
            delete openFiles[i];
    }
    delete[] openFiles;
}

//----------------------------------------------------------------------
// PCB::getExitStatus()
//  ** Implement This **
//----------------------------------------------------------------------

int PCB::getExitStatus()
{
    return this->exitStatus;
}


//----------------------------------------------------------------------
// PCB::setExitStatus(int status)
//  ** Implement This **
//----------------------------------------------------------------------

void PCB::setExitStatus(int status)
{
    this->exitStatus = status;
}


//----------------------------------------------------------------------
// PCB::addOpenFile(UserOpenFile* file)
//  ** Implement This **
//----------------------------------------------------------------------

int PCB::addOpenFile(UserOpenFile* openFile)
{
    for(int i=3; i< MAX_FILES; i++){
        if(openFiles[i] == NULL){
            openFiles[i] = openFile;
            return i;
        }
    }
    return -1;
}

//----------------------------------------------------------------------
// PCB::closeOpenFile(int fileDescriptor)
//  ** Implement This **
//----------------------------------------------------------------------

void PCB::closeOpenFile(int fileDescriptor)
{
    if(openFiles[fileDescriptor] != NULL)
        delete openFiles[fileDescriptor];

    openFiles[fileDescriptor] = NULL;
}

//----------------------------------------------------------------------
// PCB::forkHelp(int pid, int parentPid)
//  ** Implement This **
//----------------------------------------------------------------------

PCB* PCB::forkHelp(int Pid, int pPid)
{
    PCB* child = new PCB(Pid, pPid);
    for(int i=0; i < MAX_FILES; i++){
        if(this->openFiles[i] == NULL)
            child->openFiles[i] = NULL;
        else
            child->openFiles[i] = new UserOpenFile(this->openFiles[i]);
    }
    return child;
}