// exception.cc
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include <stdio.h>
#include <string.h>
#include "copyright.h"
#include "syscall.h"
#include "system.h"
#include "machine.h"
#include "pcb.h"
#include "../threads/system.h"
#include "../machine/machine.h"
#include "addrspace.h"
#include "../filesys/openfile.h"

#define MAX_FILENAME 256
#define DEFAULT_SIZE 512
#define USER_READ 0 // passed as type for userReadWrite
#define USER_WRITE 1 // passed as type for userReadWrite

// Implemented already 
void doExit();
int doExec(char* filename);
void doWrite();

/* Additions to original code, Implement these */
SpaceId doFork();
void ForkBridge(int newProcess);
void doYield();
int doJoin();

void doCreate(char* filename);
int doOpen(char* filename);
int doRead();
void doClose();

// Not required functions for implementation, just helpers for this attempt
void readFileNameUserToKernel(char* filename);
int moveBytesMemoryToKernel(int virtualAddress, char* bufferspace, int size);
int userReadWrite(int virtAddr, char* buffer, int size, int type);
/* End of Additions */

//----------------------------------------------------------------------
// ExceptionHandler
//  Entry point into the Nachos kernel.  Called when a user program
//  is executing, and either does a syscall, or generates an addressing
//  or arithmetic exception.
//
//  For system calls, the following is the calling convention:
//
//  system call code -- r2
//    arg1 -- r4
//    arg2 -- r5
//    arg3 -- r6
//    arg4 -- r7
//
//  The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//  "which" is the kind of exception.  The list of possible exceptions
//  are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int result, type = machine->ReadRegister(2);
    char* fileName = new char[MAX_FILENAME];

    if (which == SyscallException) {
        switch (type) {
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Fork:
                DEBUG('a', "Fork() system call invoked.\n");
                result = doFork();
                machine->WriteRegister(2, result);
                break;
            case SC_Exit:
                DEBUG('a', "Exit() system call invoked.\n");
                doExit();
                break;
            case SC_Exec:
                DEBUG('a', "Exec() system call invoked.\n");
                readFileNameUserToKernel(fileName);
                result = doExec(fileName);
                machine->WriteRegister(2, result);
                break;
            case SC_Yield:
                DEBUG('a', "Yield() system call invoked.\n");
                doYield();
                break;
            case SC_Join:
                DEBUG('a', "Join() system call invoked.\n");
                result = doJoin();
                machine->WriteRegister(2, result);
                break;
            case SC_Create:
                DEBUG('a', "Create() system call invoked.\n");
                readFileNameUserToKernel(fileName);
                doCreate(fileName);
                break;
            case SC_Open:
                DEBUG('a', "Open() system call invoked.\n");
                readFileNameUserToKernel(fileName);
                result = doOpen(fileName);
                machine->WriteRegister(2, result);
                break;
            case SC_Read:
                DEBUG('a', "Read() system call invoked.\n");
                result = doRead();
                machine->WriteRegister(2, result);
                break;
            case SC_Write:
                DEBUG('a', "Write() system call invoked.\n");
                doWrite();
                break;
            case SC_Close:
                DEBUG('a', "Close() system call invoked.\n");
                doClose();
                break;
            default:
                printf("Unexpected system call %d. Halting.\n", type);
                interrupt->Halt();
        }
    } else {
        printf("Unexpected user mode exception %d. Halting.\n", which);
        interrupt->Halt();
    }

    // Increment program counter
    machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PrevPCReg) + 4);

}

//----------------------------------------------------------------------
// doExit
// ** Not implemented in orginal source code **
//----------------------------------------------------------------------

void doExit()
{
    int status = machine->ReadRegister(4);  // Get status code
    int currentPID = currentThread->space->getPID();
    PCB* currentPCB = processManager->getPCB(currentPID);
    currentPCB->setExitStatus(status);

    delete currentThread->space;
    currentThread->space = NULL;

    processManager->waitStateOnChild(currentPID);
    processManager->broadcastOnExit(currentPID);

    fprintf(stderr, "Process %d exiting\n", currentPID);
    if(currentPID == 0)
        interrupt->Halt();

    currentThread->Finish();

}

//----------------------------------------------------------------------
// execLauncher
//----------------------------------------------------------------------

void execLauncher(int unused)
{
    //AddrSpace *curAddrSpace = currentThread->space;
    //curAddrSpace->InitRegisters();
    //curAddrSpace->RestoreState();
    currentThread->space->InitRegisters();
    currentThread->space->SaveState();
    currentThread->space->RestoreState();
    machine->Run();
}

//----------------------------------------------------------------------
// doExec
//----------------------------------------------------------------------

int doExec(char *fileName)
{


    // Finally it needs an address space. We will initialize the address
    // space by loading in the program found in the executable file that was
    // passed in as the first argument.

    OpenFile *execFile = fileSystem->Open(fileName);
    if(execFile == NULL){
        fprintf(stderr, "Couldn't open file %s to execute. Terminating process\n", fileName);
        return -1;
    }

    //Create new childSpace
    AddrSpace* childSpace = new AddrSpace(execFile);
    //Check validity of child space exit if invalid
    if(!childSpace->isValid()){
        delete childSpace;
        delete execFile;
        return -1;
    }

    // Initializing PCB
    int childPid = childSpace->getPID();
    int parentPid = currentThread->space->getPID();
    PCB *childPcb = new PCB(childPid, parentPid);
    processManager->trackPCB(childPid, childPcb);

    // Create kernel thread to manage process
    Thread *childThread = new Thread(fileName);

    childThread->space = childSpace;
    childPcb->thread = childThread;

    /* End of Additions */
    fileSystem->Remove(fileName);
    delete execFile;

    // We launch the process with the kernel threads Fork() function. Note
    // that this is different from our implementation of Fork()!

    childPcb->thread->Fork(execLauncher, machine->ReadRegister(RetAddrReg));
    fprintf(stderr, "Process %d executing process %d\n", parentPid, childPid);

    // Because we just overwrote the current thread's address space when we
    // called `new AddrSpace(execFile)` it can no longer be allowed to
    // execute so we call Finish(). You will have to fix this in your
    // implementation once you implement multiple address spaces.
    currentThread->Yield();

    // We shouldn't reach this point in the code...
    return childPid;
}
//----------------------------------------------------------------------
// doWrite
//----------------------------------------------------------------------

void doWrite()
{
    int userBufVirtAddr = machine->ReadRegister(4);
    int userBufSize = machine->ReadRegister(5);
    int dstFile = machine->ReadRegister(6);
    int bytesActuallyWriten = 0;

    //int i, userBufPhysAddr, bytesToEndOfPage, bytesToCopy, bytesCopied = 0;
    char *kernelBuf = new char[userBufSize + 1];

    if (dstFile == ConsoleOutput) {
        moveBytesMemoryToKernel(userBufVirtAddr, kernelBuf, userBufSize);
        kernelBuf[userBufSize] = 0;
        openFileManager->consoleWriteLock->Acquire();
        for(int i=0; i < userBufSize; i++){
            UserConsolePutChar(kernelBuf[i]);
        }
        openFileManager->consoleWriteLock->Release();
    }
    else{
        moveBytesMemoryToKernel(userBufVirtAddr, kernelBuf, userBufSize);
        int currentPID = currentThread->space->getPID();
        UserOpenFile* uoFile = processManager->getPCB(currentPID)->getOpenFile(dstFile);
        OpenFile* oFile = openFileManager->getOpenFile(uoFile->fileTableIndex)->openFile;

        bytesActuallyWriten = oFile->WriteAt(kernelBuf, userBufSize, uoFile->currentPosition);
        uoFile->currentPosition += bytesActuallyWriten;
    }

    delete[] kernelBuf;
}


//----------------------------------------------------------------------
// doFork
//----------------------------------------------------------------------

SpaceId doFork()
{
    if(currentThread->space->getPageNum() > memoryManager->getFreeFrameNum()  || processManager->getPIDsFree() <= 0)
        return -1;

    int funcAddr = machine->ReadRegister(4);                        // func Address in 4th register
    Thread* childThread = new Thread("Forked Child");               // new kernel thread for child
    AddrSpace *copyAddrSpace = new AddrSpace(currentThread->space); // duplicate address space of process

    int parentPID, childPID;
    parentPID = currentThread->space->getPID();
    childPID = copyAddrSpace->getPID();

    fprintf(stderr, "Process %d forked process %d\n",parentPID,childPID);

    PCB *childPCB = processManager->getPCB(parentPID)->forkHelp(childPID,parentPID);
    processManager->trackPCB(childPID, childPCB);       // link childPCB to childPID
    childPCB->thread = childThread;                     // set child thread
    childPCB->thread->space = copyAddrSpace;            // set duplicate address space to child
    childPCB->thread->Fork(ForkBridge, funcAddr);       // new thread creates a bridge to execute user function
    doYield();                                          // Current thread yields to new thread
    return childPID;
}


//----------------------------------------------------------------------
// ForkBridge
//----------------------------------------------------------------------

void ForkBridge(int newProcess)
{
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    machine->WriteRegister(PCReg, newProcess);
    machine->WriteRegister(PrevPCReg, newProcess - 4);
    machine->WriteRegister(NextPCReg, newProcess + 4);

    machine->Run();
    ASSERT(FALSE);  // shouldn't get here
}


//----------------------------------------------------------------------
// doYield
//----------------------------------------------------------------------

void doYield()
{
    currentThread->SaveUserState();
    currentThread->Yield();
    currentThread->RestoreUserState();
}


//----------------------------------------------------------------------
// doJoin
//----------------------------------------------------------------------

int doJoin()
{
    printf("In Join\n");
    int childPID = machine->ReadRegister(4);
    int childExitStatus = processManager->waitStateOn(childPID, currentThread->space->getPID());
    return childExitStatus;
}


//----------------------------------------------------------------------
// doCreate
//----------------------------------------------------------------------

void doCreate(char* filename)
{
    bool created = fileSystem->Create(filename, DEFAULT_SIZE);
}


//----------------------------------------------------------------------
// doOpen
//----------------------------------------------------------------------

int doOpen(char* filename)
{
    UserOpenFile* userOpenFile;
    int fileDescr;

    OpenFile* openFile = fileSystem->Open(filename);
    if(openFile == NULL){
        fprintf(stderr, "Could not Find %s\n", filename);
        return -1;
    }


    int ftIndex = openFileManager->addOpenFile(openFile, filename);
    if(ftIndex == ERR_FAIL_ADD)
        return -1;
    userOpenFile = new UserOpenFile(filename, ftIndex, 0);
    fileDescr = processManager->getPCB(currentThread->space->getPID())->addOpenFile(userOpenFile);

    return fileDescr;
}


//----------------------------------------------------------------------
// do
//----------------------------------------------------------------------

int doRead()
{
    int readAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int fileID = machine->ReadRegister(6);

    char* buffer = new char[size + 1];
    int numActualBytesRead; //= size;
    int bytesRead = 0;

    if (fileID == ConsoleInput) {//Read data from the console to the system buffer
        while (bytesRead < size && UserConsoleGetChar() != '\n') {
            //buffer[bytesRead] = getchar();
            buffer[bytesRead] = UserConsoleGetChar();
            bytesRead++;
        }
    }
    else {

        //Read data from the file to the system buffer
        UserOpenFile* userFile = processManager->getPCB(currentThread->space->getPID())->getOpenFile(fileID);

        //Now from openFileManger, find the SystemOpenFile data structure for this userFile.
        //SysOpenFile*  sysFile = openFileManager->getFile(userFile->filename, userFile->fileTableIndex);    
        OpenFile* opFile = openFileManager->getOpenFile(userFile->fileTableIndex)->openFile;

        //Use ReadAt() to read the file at selected offset to this system buffer buffer[]
        numActualBytesRead = opFile->ReadAt(buffer, size, userFile->currentPosition);

        // Adust the offset in userFile to reflect my current position.
        //userFile->currentPosition = size+1;
        userFile->currentPosition += size;

    }

    //Null Terminate String
    buffer[size] = '\0';

    //Now copy data from the system buffer to the targted main memory space using userReadWrite()
    userReadWrite(readAddr, buffer, size, USER_READ);

    delete [] buffer;
    return numActualBytesRead;
}


//----------------------------------------------------------------------
// doClose
//----------------------------------------------------------------------

void doClose()
{
    //TODO
    int fileDescr = machine->ReadRegister(4);
    PCB* currentPCB = processManager->getPCB(currentThread->space->getPID());
    UserOpenFile* userOpenFile = currentPCB->getOpenFile(fileDescr);
    if(userOpenFile == NULL){
        fprintf(stderr, "Can't find file w/ file descriptor %d in process %d\n", fileDescr,
                            currentThread->space->getPID());
        return;
    }

    int ftIndex = userOpenFile->fileTableIndex;
    openFileManager->reduceOpenFiles(ftIndex);
    currentPCB->closeOpenFile(fileDescr);
}




/* -------------- Helper Functions -----------------------------------*/

//----------------------------------------------------------------------
// readFileNameUserToKernel
//----------------------------------------------------------------------

void readFileNameUserToKernel(char* filename)
{

    int currPosition = 0;
    int filenameA = machine->ReadRegister(4);

    do{
        moveBytesMemoryToKernel(filenameA, filename+currPosition, 1);
        filenameA++;
    } while (filename[currPosition++] != 0);

    filename[currPosition] = 0;
}


//----------------------------------------------------------------------
// moveBytesMemoryToKernel
//----------------------------------------------------------------------

int moveBytesMemoryToKernel(int virtualAddress, char* bufferspace, int size)
{

    int physcialAddr = 0;
    int remaining = 0;
    int bytesTransfered = 0;
    int bytesRemaining = 0;

    while(size > 0){
        physcialAddr = currentThread->space->Translate(virtualAddress);
        remaining = PageSize - physcialAddr % PageSize;
        bytesRemaining = remaining < size ? remaining : size;
        bcopy(machine->mainMemory + physcialAddr, bufferspace + bytesTransfered, bytesRemaining);
        size -= bytesRemaining;
        bytesTransfered += bytesRemaining;
        virtualAddress += bytesRemaining;
    }

    return bytesTransfered;
}

//----------------------------------------------------------------------
// Helper function that either reads from main mem into a buffer or 
// writes from a buffer into main mem.
//----------------------------------------------------------------------

int userReadWrite(int virtAddr, char* buffer, int size, int type) {

    int physAddr = 0;
    int numBytesFromPSLeft = 0;
    int numBytesCopied = 0;
    int numBytesToCopy = 0;

    if (type == USER_READ) { // Read and copy data from the system buffer to the user space in main memory
        while (size > 0) {
            //Translate the virtual address to phyiscal address physAddr 
            physAddr = currentThread->space->Translate(virtAddr);

            numBytesFromPSLeft = PageSize - physAddr % PageSize;
            numBytesToCopy = (numBytesFromPSLeft < size) ? numBytesFromPSLeft : size;
            bcopy(buffer + numBytesCopied, machine->mainMemory + physAddr, numBytesToCopy);
            numBytesCopied += numBytesToCopy;
            size -= numBytesToCopy;
            virtAddr += numBytesToCopy;
        }
    }
    else if (type == USER_WRITE) { // Copy data from the user's main memory to the system buffer
        while (size > 0) {
            //Translate the virtual address to phyiscal address physAddr  
            physAddr = currentThread->space->Translate(virtAddr);

            numBytesFromPSLeft = PageSize - physAddr % PageSize;
            numBytesToCopy = (numBytesFromPSLeft < size) ? numBytesFromPSLeft : size;
            bcopy(machine->mainMemory + physAddr, buffer + numBytesCopied, numBytesToCopy);
            numBytesCopied += numBytesToCopy;
            size -= numBytesToCopy;
            virtAddr += numBytesToCopy;
        }
    }
    else {
        //printf ("Invalid type passed in.\n");
        ASSERT(FALSE);
    }
    return numBytesCopied; 
}

