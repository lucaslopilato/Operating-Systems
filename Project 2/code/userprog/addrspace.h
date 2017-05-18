// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
#include "copyright.h"
#include "system.h"
#include "noff.h"
#include "sysopenfile.h"
#include "pcb.h"
#include "machine.h" //definition of PageSize
#ifdef HOST_SPARC
#include <strings.h>
#endif

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#define UserStackSize 1024              // increase this as necessary!

class AddrSpace {

public:
    AddrSpace(OpenFile* executable, PCB *pcb);	// Create an address space,
                                        // initializing it with the program
                                        // stored in the file "executable"
    AddrSpace(const AddrSpace* other, PCB* pcb); //Copy Constructor

    ~AddrSpace();			            // De-allocate an address space

    void InitRegisters();               // Initialize user-level CPU registers,
                                        // before jumping to user code

    void SaveState();			        // Save/restore address space-specific
    void RestoreState();		        // info on a context switch

    int Translate(int virtualAddr);     // Return the physical memory address
                                        // mapped by a virtual address

    int ReadFile(int virtAddr, OpenFile* file, int size, int fileAddr); //Loads code
                                        //And data segments into translated memory
    PCB* getPCB();                      //Returns associated PCB for address space
    bool isValid();                     //Checks if new space successfully allocated

private:
    TranslationEntry *pageTable;	    // Assume linear page table translation
                                        // for now!
    unsigned int numPages;		        // Number of pages in the virtual
                                        // address space
};

#endif // ADDRSPACE_H
