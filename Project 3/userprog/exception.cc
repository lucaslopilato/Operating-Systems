// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include "pcb.h"

#define MAX_FILENAME_LEN 128
#define USER_READ 0 // passed as type for userReadWrite
#define USER_WRITE 1 // passed as type for userReadWrite

void IncrementPC(void);
int forkImpl(void);
void copyStateBack(int forkPC);
void yieldImpl(void);
void exitImpl(void);
int joinImpl(void);
SpaceId execImpl(char* filename);
void execHelper(void);
void readFilenameFromUsertoKernel(char* filename);
void createImpl(char* filename);
char* copyString(char* oldStr);
int openImpl(char* filename);
int userReadWrite(int virtAddr, char* buffer, int size, int type);
void writeImpl(void);
int readImpl(void);
void closeImpl(void);
void pageFaultHandler(void);

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which)
{
    int newProcessPID = 0;
    int otherProcessStatus = 0;
    int result = 0;
    char* filename = new char[MAX_FILENAME_LEN];

    int type = machine->ReadRegister(2);
    PCB* pcb = (currentThread->space)->getPCB();

    if (which == SyscallException) {

        switch (type) { /* Find out what type of syscall we're dealing with */

            case SC_Halt:
                DEBUG('v',"System Call: %d invoked Halt\n", pcb->getPID());
                interrupt->Halt();
                break;
            case SC_Fork:
                DEBUG('v',"System Call: %d invoked Fork\n", pcb->getPID());
                newProcessPID = forkImpl();
                machineLock->Acquire();
                machine->WriteRegister(2, newProcessPID);
                machineLock->Release();
                break;
            case SC_Yield:
                DEBUG('v',"System Call: %d invoked Yield\n", pcb->getPID());
                yieldImpl();
                break;
            case SC_Exec:
                DEBUG('v',"System Call: %d invoked Exec\n", pcb->getPID());
                readFilenameFromUsertoKernel(filename);
                newProcessPID = execImpl(filename);
                machineLock->Acquire();
                machine->WriteRegister(2, newProcessPID);
                machineLock->Release();
                break;
            case SC_Join:
                DEBUG('v',"System Call: %d invoked Join\n", pcb->getPID());
                otherProcessStatus = joinImpl();
                machineLock->Acquire();
                machine->WriteRegister(2, otherProcessStatus);
                machineLock->Release();
                break;
            case SC_Exit:
                DEBUG('v',"System Call: %d invoked Exit\n", pcb->getPID());
                exitImpl();
                break;
            case SC_Create:
                DEBUG('v',"System Call: %d invoked Create\n", pcb->getPID());
                readFilenameFromUsertoKernel(filename);
                createImpl(filename);
                break;
            case SC_Open:
                DEBUG('v',"System Call: %d invoked Open\n", pcb->getPID());
                readFilenameFromUsertoKernel(filename);
                copyString(filename); // copy bytes to a fresh piece of memory
                result = openImpl(filename); 
                machine->WriteRegister(2, result);
                break;
            case SC_Write:
                DEBUG('v',"System Call: %d invoked Write\n", pcb->getPID());
                writeImpl();
                break;
            case SC_Read:
                DEBUG('v',"System Call: %d invoked Read\n", pcb->getPID());
                result = readImpl();
                machine->WriteRegister(2, result);
                break;
            case SC_Close:
                DEBUG('v',"System Call: %d invoked Close\n", pcb->getPID());
                closeImpl();
                break;
            default:
                DEBUG('v',"System Call: %d invoked an unknown syscall!\n", 
                    pcb->getPID());
                ASSERT(FALSE);
        }
        
        delete [] filename;
        IncrementPC();

    } else if (which == PageFaultException) {
        pageFaultHandler();
    } else {
        fprintf(stderr,"Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}

//----------------------------------------------------------------------
// Fork system call implementation
//----------------------------------------------------------------------

int forkImpl()
{
    // Create a new kernel thread
    Thread* childThread = new Thread("user-level child process");

    int newProcessPC = machine->ReadRegister(4);

    // Construct new PCB
    int currPID = currentThread->space->getPCB()->getPID();
    int newPID = processManager->getPID();
    if (newPID == -1) {
        fprintf(stderr,"Process %d is unable to fork a new process\n", currPID);
        return -1;
    }
    PCB* newPCB = new PCB(newPID, currPID);
    newPCB->status = P_RUNNING;
    newPCB->process = childThread;
    processManager->addProcess(newPCB, newPID);

    // Make a copy of the address space, save registers
    childThread->space = new AddrSpace(currentThread->space, newPCB);
    int childNumPages = childThread->space->getNumPages();

    if (childThread->space->pageTable == NULL) {
        fprintf(stderr,"Process %d Fork: start at address 0x%x with %d pages"
               " memory failed\n", currPID, newProcessPC, childNumPages);
        return -1;
    }

    childThread->space->SaveState();
    childThread->SaveUserState();

    // Mandatory printout of the forked process
    DEBUG('v',"Process %d Fork: start at address 0x%x with %d pages memory\n",
        currPID, newProcessPC, childNumPages);
    
    // Set up the function that new process will run and yield
    childThread->Fork(copyStateBack, newProcessPC);
    currentThread->Yield(); 
    return newPID;
}

//----------------------------------------------------------------------
// copyStateBack
//      The "dummy" function from 2.1.1 of NACHOS implementation that 
//      copies back the machine registers, PC and return registers saved
//      from before the yield was performed.
//      
//----------------------------------------------------------------------

void copyStateBack(int forkPC)
{
    // Get fresh registers, but use copy of addr space
    currentThread->RestoreUserState();

    // Set the PC and run
    machineLock->Acquire();
    machine->WriteRegister(PCReg, forkPC);
    machine->WriteRegister(PrevPCReg, forkPC - 4);
    machine->WriteRegister(NextPCReg, forkPC + 4);
    machineLock->Release();

    currentThread->space->RestoreState();
    machine->Run();
    ASSERT(FALSE); // should never reach here
}

//----------------------------------------------------------------------
// Yield system call implementation.
//----------------------------------------------------------------------

void yieldImpl()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    currentThread->SaveUserState();
    currentThread->Yield();
    currentThread->RestoreUserState();
    currentThread->space->RestoreState();
    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Exit system call implementation.
//----------------------------------------------------------------------

void exitImpl()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    int status = machine->ReadRegister(4);
    int currPID = currentThread->space->getPCB()->getPID();
    
    DEBUG('v',"Process %d exits with %d\n", currPID, status);
    currentThread->space->getPCB()->status = status;
    processManager->broadcast(currPID);

    delete currentThread->space;
    currentThread->space = NULL;
   
    processManager->clearPID(currPID);
    (void) interrupt->SetLevel(oldLevel);
    currentThread->Finish();
}

//----------------------------------------------------------------------
// Join system call implementation.
//----------------------------------------------------------------------

int joinImpl()
{
    int otherPID = machine->ReadRegister(4);
    currentThread->space->getPCB()->status = P_BLOCKED;
    int otherProcessStatus = processManager->getStatus(otherPID);
    
    if (otherProcessStatus < 0) {
        return processManager->getStatus(otherPID);
    }
    processManager->join(otherPID);
    currentThread->space->getPCB()->status = P_RUNNING;
    return processManager->getStatus(otherPID);
}

//----------------------------------------------------------------------
// Exec system call helper.
//----------------------------------------------------------------------

void execHelper(int uselessArg)
{
    AddrSpace* currAddrSpace = currentThread->space;
    currAddrSpace->InitRegisters();
    currAddrSpace->SaveState();
    currAddrSpace->RestoreState();
    machine->Run();
}

//----------------------------------------------------------------------
// Exec system call implementation.
//----------------------------------------------------------------------

SpaceId execImpl(char* filename)
{
    int currPID = currentThread->space->getPCB()->getPID();

    // Open the file
    OpenFile* fileToExecute = fileSystem->Open(filename);
    if (fileToExecute == NULL) {
        fprintf(stderr,"Unable to open file %s for execution. Terminating.\n", filename);
        return -1;
    }

    // Create a PCB for the new process, along with its running thread
    Thread* newThread = new Thread("Executing process");
    int newPID = processManager->getPID();
    PCB* pcb = new PCB(newPID, currPID);
    pcb->status = P_RUNNING;
    pcb->process = newThread;

    // Give it an address space
    AddrSpace* newSpace = new AddrSpace(fileToExecute, pcb);
    if (!newSpace->isValid()) {
        DEBUG('v',"Exec Program: %d loading %s failed\n", currPID, filename);
        delete fileToExecute;
        delete newSpace;
        return -1;
    }
    newThread->space = newSpace;
    processManager->addProcess(pcb, newPID);

    // Close file and execute new process
    delete fileToExecute;
    DEBUG('v',"Exec Program: %d loading %s\n", currPID, filename);
    newThread->Fork(execHelper, 0);
    currentThread->Yield();
    return newPID;
}

//----------------------------------------------------------------------
// Helper function that brings bytes from memory to kernel space.
//----------------------------------------------------------------------

int moveBytesFromMemToKernel(int virtAddr, char* buffer, int size)
{
    int physAddr = 0;
    int remainingFromPage = 0;
    int bytesCopied = 0;
    int bytesToCopy = 0;
    ExceptionType exception; // for PageFault, etc.
    
    while (size > 0) {

        // Need to use the machine->Translate so that page faults handled
        // properly.
        do 
        {
            exception = machine->Translate(virtAddr, &physAddr, size, FALSE);
            if (exception != NoException) 
            {
                machine->RaiseException(exception, virtAddr);
            }
        } while (exception != NoException);

        remainingFromPage = PageSize - physAddr % PageSize;
        bytesToCopy = remainingFromPage < size ? remainingFromPage : size;
        bcopy(machine->mainMemory + physAddr, buffer + bytesCopied, bytesToCopy);
        size -= bytesToCopy;
        bytesCopied += bytesToCopy;
        virtAddr += bytesToCopy;
    }

    return bytesCopied;
}

//----------------------------------------------------------------------
// Helper function to bring a string that represents a filename from 
// user space to kernel space.
//----------------------------------------------------------------------

void readFilenameFromUsertoKernel(char* filename)
{
    int currPosition = 0;
    int filenameArg = machine->ReadRegister(4);

    do {
        moveBytesFromMemToKernel(filenameArg, filename + currPosition, 1);
        filenameArg++;
    } while (filename[currPosition++] != 0);

    filename[currPosition] = 0; // terminate C string
}

//----------------------------------------------------------------------
// Create file system call implementation.
//----------------------------------------------------------------------

void createImpl(char* filename)
{
    bool successfulCreate = fileSystem->Create(filename, 0);
    ASSERT(successfulCreate); // should never fail
}

//----------------------------------------------------------------------
// Helper function to copy a string to a fresh piece of memory so we
// don't run into funky pointer stuff.
//----------------------------------------------------------------------

char* copyString(char* oldStr)
{
    char* newStr = new char[MAX_FILENAME_LEN];
    for (int i = 0; i < MAX_FILENAME_LEN; i++) {
        newStr[i] = oldStr[i];
        if (oldStr[i] == NULL) {
            break;
        }
    }
    return newStr;
}

//----------------------------------------------------------------------
// Open file system call implementation.
//----------------------------------------------------------------------

int openImpl(char* filename)
{
    int index = 0;
    SysOpenFile* currSysFile = fileManager->getFile(filename, index);

    if (currSysFile == NULL) { // if no process currently has it open
        OpenFile* openFile = fileSystem->Open(filename);
        if (openFile == NULL) {
            fprintf(stderr,"Unable to open the file %s\n", filename);
            return -1;
        }
        SysOpenFile currSysFile;
        currSysFile.file = openFile;
        currSysFile.numProcessesAccessing = 1;
        currSysFile.filename = copyString(filename);
        index = fileManager->addFile(currSysFile);
    }
    else { // the file is already open by another process
        currSysFile->numProcessesAccessing++;
    }

    // Either way, add it to the current PCB's open file list
    UserOpenFile currUserFile;
    currUserFile.indexInSysOpenFileList = index;
    currUserFile.currOffsetInFile = 0;
    int currFileID = currentThread->space->getPCB()->addFile(currUserFile);
    return currFileID;
}

//----------------------------------------------------------------------
// Helper function that either reads from main mem into a buffer or 
// writes from a buffer into main mem.
//----------------------------------------------------------------------

int userReadWrite(int virtAddr, char* buffer, int size, int type)
{
    int physAddr = 0;
    int numBytesFromPSLeft = 0;
    int numBytesCopied = 0;
    int numBytesToCopy = 0;
    ExceptionType exception;

    if (type == USER_READ) // from buffer to main memory
    {
        while (size > 0)
        {
            // Need to use the machine->Translate so that page faults handled
            // properly, instead of the addrspace->Translate
            do 
            {
                exception = machine->Translate(virtAddr, &physAddr, size, FALSE);
                if (exception != NoException) 
                {
                    machine->RaiseException(exception, virtAddr);
                }
            } while (exception != NoException);
            numBytesFromPSLeft = PageSize - physAddr % PageSize;
            numBytesToCopy = (numBytesFromPSLeft < size) ? numBytesFromPSLeft : size;
            bcopy(buffer + numBytesCopied, machine->mainMemory + physAddr, numBytesToCopy);
            numBytesCopied += numBytesToCopy;
            size -= numBytesToCopy;
            virtAddr += numBytesToCopy;
        }
    }
    else if (type == USER_WRITE) // from main memory to buffer
    {
        while (size > 0)
        {
            // Need to use the machine->Translate so that page faults handled
            // properly, instead of the addrspace->Translate
            do 
            {
                exception = machine->Translate(virtAddr, &physAddr, size, FALSE);
                if (exception != NoException) 
                {
                    machine->RaiseException(exception, virtAddr);
                }
            } while (exception != NoException);

            numBytesFromPSLeft = PageSize - physAddr % PageSize;
            numBytesToCopy = (numBytesFromPSLeft < size) ? numBytesFromPSLeft : size;
            bcopy(machine->mainMemory + physAddr, buffer + numBytesCopied, numBytesToCopy);
            numBytesCopied += numBytesToCopy;
            size -= numBytesToCopy;
            virtAddr += numBytesToCopy;
        }
    }
    else {
        fprintf(stderr,"Invalid type passed in.\n");
        ASSERT(FALSE);
    }
    return numBytesCopied; 
}

//----------------------------------------------------------------------
// Write file system call implementation
//----------------------------------------------------------------------
void writeImpl()
{
    int writeAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int fileID = machine->ReadRegister(6);

    char* buffer = new char[size + 1];
    if (fileID == ConsoleOutput) {
        userReadWrite(writeAddr, buffer, size, USER_WRITE);
        buffer[size] = 0; // always terminate
        printf("%s", buffer);
    }
    else {
        buffer = new char[size];
        userReadWrite(writeAddr, buffer, size, USER_WRITE);
        UserOpenFile* userFile = currentThread->space->getPCB()->getFile(fileID);
        if (userFile == NULL) {
            return;
        } else {
            SysOpenFile* sysFile = 
                    fileManager->getFile(userFile->indexInSysOpenFileList);
            int numBytesWritten = 
                    sysFile->file->WriteAt(buffer, size, userFile->currOffsetInFile);
            userFile->currOffsetInFile += numBytesWritten;
        }
    }
    delete [] buffer;
}

//----------------------------------------------------------------------
// Read file system call implementation.
//----------------------------------------------------------------------

int readImpl()
{
    int readAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int fileID = machine->ReadRegister(6);
    char* buffer = new char[size + 1];
    int numActualBytesRead = size;

    if (fileID == ConsoleInput) {
        int bytesRead = 0;
        while (bytesRead < size) {
            buffer[bytesRead] = getchar();
            bytesRead++;
        }
    }
    else {
        UserOpenFile* userFile = currentThread->space->getPCB()->getFile(fileID);
        if (userFile == NULL) {
            return 0;
        } 
        SysOpenFile* sysFile = 
                fileManager->getFile(userFile->indexInSysOpenFileList);
        numActualBytesRead = 
                sysFile->file->ReadAt(buffer, size, userFile->currOffsetInFile);
        userFile->currOffsetInFile += numActualBytesRead;
    }
    userReadWrite(readAddr, buffer, numActualBytesRead, USER_READ);
    delete [] buffer;
    return numActualBytesRead;
}

//----------------------------------------------------------------------
// Close file system call implementation.
//----------------------------------------------------------------------

void closeImpl()
{
    int fileID = machine->ReadRegister(4);
    UserOpenFile* userFile = currentThread->space->getPCB()->getFile(fileID);
    if (userFile == NULL) {
        return;
    } else {
        SysOpenFile* sysFile = 
                fileManager->getFile(userFile->indexInSysOpenFileList);
        sysFile->closedBySingleProcess();
        currentThread->space->getPCB()->removeFile(fileID);
    }
}

//----------------------------------------------------------------------
// Page fault handler that loads requested page into memory for
// Project 3, which implements demand-paging.
//----------------------------------------------------------------------

void pageFaultHandler() 
{
    int faultingVirtAddr = machine->ReadRegister(BadVAddrReg);
    virtualMemoryManager->swapPageIn(faultingVirtAddr);

    int pid = currentThread->space->getPCB()->getPID();
    int virtPage = faultingVirtAddr / PageSize;
    int physPage = currentThread->space->getPageTableEntry(virtPage)->physicalPage;

    //fprintf(stderr,"L %d: %d -> %d\n", pid, virtPage, physPage);
    DEBUG('v',"L %d: %d -> %d\n", pid, virtPage, physPage);
}

//----------------------------------------------------------------------
// IncrementPC
//      Helper function used to increment the values of PCreg, NextPCreg,
//      and PrevPCreg after a syscall.
//----------------------------------------------------------------------

void IncrementPC(void)
{
    int currentReg = machine->ReadRegister(PCReg);
    int nextReg = machine->ReadRegister(NextPCReg);
    int prevReg = machine->ReadRegister(PrevPCReg);

    currentReg = currentReg + 4;
    nextReg = nextReg + 4;
    prevReg = prevReg + 4;

    machineLock->Acquire();
    machine->WriteRegister(PCReg, currentReg);
    machine->WriteRegister(NextPCReg, nextReg);
    machine->WriteRegister(PrevPCReg, prevReg);
    machineLock->Release();
}
