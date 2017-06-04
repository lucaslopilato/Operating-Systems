/*
 * VirtualMemoryManager implementation
 *
 * Used to facilitate demand paging through providing a means by which page
 * faults can be handled and pages loaded from and stored to disk.
*/

#include <stdlib.h>
#include <machine.h>
#include "virtualmemorymanager.h"
#include "system.h"

VirtualMemoryManager::VirtualMemoryManager()
{
    fileSystem->Create(SWAP_FILENAME, SWAP_SECTOR_SIZE * SWAP_SECTORS);
    swapFile = fileSystem->Open(SWAP_FILENAME);

    swapSectorMap = new BitMap(SWAP_SECTORS);
    physicalMemoryInfo = new FrameInfo[NumPhysPages];
    
    // Create empty space in physcial memory
    for(int i = 0; i < NumPhysPages; i++){
        physicalMemoryInfo[i].space = NULL;
    }

    //swapSpaceInfo = new SwapSectorInfo[SWAP_SECTORS];
    nextVictim = 0;
}

VirtualMemoryManager::~VirtualMemoryManager()
{
    fileSystem->Remove(SWAP_FILENAME);
    delete swapFile;
    delete [] physicalMemoryInfo;
    //delete [] swapSpaceInfo;
}

int VirtualMemoryManager::allocSwapSector()
{
    int location = swapSectorMap->Find() * PageSize; // also marks the bit
    return location;
}
/*
SwapSectorInfo * VirtualMemoryManager::getSwapSectorInfo(int index)
{
    return swapSpaceInfo + index;
    
}
*/
void VirtualMemoryManager::writeToSwap(char *page, int pageSize,
                                       int backStoreLoc)
{
    swapFile->WriteAt(page, pageSize, backStoreLoc);
}

/*
 * Page replacement with  the second chance algorithm
 */
void VirtualMemoryManager::swapPageIn(int virtAddr)
{

        TranslationEntry* currPageEntry;
        TranslationEntry* oldPageEntry;                             //added
        FrameInfo* physPageInfo = physicalMemoryInfo + nextVictim;  //added
        
        /*
        if(nextVictim>= NumPhysPages) {//no more space available
                fprintf(stderr, "Fatal error: No more space available\n");
                exit(1);
                return;
        }
        */

        // Find available spot to use with Clock Paging 
        while(  physPageInfo->space != NULL &&                  // condition: space not available
                getPageTableEntry(physPageInfo)->use == true)   // condition: space in use
        {
            //set these parameters so when you exit these are the pages conditions
            getPageTableEntry(physPageInfo)->use = false;       
            nextVictim = (nextVictim + 1) % NumPhysPages;
            physPageInfo = physicalMemoryInfo + nextVictim;
        }

        // Found an available spot nosee if there is an empty slot
        // found empty slot
        if(physPageInfo->space == NULL){
            *physPageInfo = FrameInfo();
            physPageInfo->space = currentThread->space;
            physPageInfo->pageTableIndex = virtAddr / PageSize;
            currPageEntry = getPageTableEntry(physPageInfo);
            currPageEntry->physicalPage = memoryManager->getPage();
            loadPageToCurrVictim(virtAddr);
        }

        //Not an empty slot so find the victim page
        else
        {
            oldPageEntry = getPageTableEntry(physPageInfo);
            if(oldPageEntry->dirty == true)
            {
                char* physMemLoc = machine->mainMemory + oldPageEntry->physicalPage * PageSize;
                writeToSwap(physMemLoc, PageSize, oldPageEntry->locationOnDisk);
                oldPageEntry->dirty = false;
            }

            //We assume this page is not occupied by any process space
            physPageInfo->space = currentThread->space;
            physPageInfo->pageTableIndex = virtAddr / PageSize;
            currPageEntry = getPageTableEntry(physPageInfo);
            currPageEntry->physicalPage = oldPageEntry->physicalPage; //memoryManager->getPage();
            loadPageToCurrVictim(virtAddr);
            oldPageEntry->valid = false;    //added
        }

        nextVictim = (nextVictim + 1) % NumPhysPages; //pageTableSize;

        /*

        //Begin Lucas' code
        //Perform 2nd Change algorithm to find next victim
        
        //Page Table Size
        int pageTableSize = currentThread->space->getNumPages();
        
        FrameInfo * physPageInfo = physicalMemoryInfo + nextVictim;
        currPageEntry = getPageTableEntry(physPageInfo);

        //Loop until an unused entry is found
        while(!currPageEntry->use){
            //Set to false
            currPageEntry->use = false;

            //Move to next potential victim
            nextVictim = (nextVictim + 1) % pageTableSize;

            //Update physPageInfo
            physPageInfo = physicalMemoryInfo + nextVictim;
            currPageEntry = getPageTableEntry(physPageInfo);
        }
        
        //If selected victim is dirty
        //Write to SWAP
        if(currPageEntry->dirty){
            //Get Page Start Pointer
            char* pg = machine->mainMemory + 
                       currPageEntry->physicalPage * PageSize ;
            writeToSwap(pg, PageSize, currPageEntry->locationOnDisk);
        }
        
        //End Lucas' code

        */


        //We assume this page is not occupied by any process space
        //physPageInfo->space = currentThread->space;
        //physPageInfo->pageTableIndex = virtAddr / PageSize;
        //currPageEntry = getPageTableEntry(physPageInfo);
        //currPageEntry->physicalPage = oldPageEntry->physicalPage; //memoryManager->getPage();
        //loadPageToCurrVictim(virtAddr);
        //oldPageEntry->valid = false;    //added

        //Changed to loop
        //nextVictim = nextVictim + 1;
        //nextVictim = (nextVictim + 1) % NumPhysPages; //pageTableSize;
}


/*
 * Cleanup the physical memory allocated to a given address space after its 
 * destructor invokes.
*/
void VirtualMemoryManager::releasePages(AddrSpace* space)
{
    for (int i = 0; i < space->getNumPages(); i++)
    {
        TranslationEntry* currPage = space->getPageTableEntry(i);
    //  int swapSpaceIndex = (currPage->locationOnDisk) / PageSize;
    //  SwapSectorInfo * swapPageInfo = swapSpaceInfo + swapSpaceIndex;
    //  swapPageInfo->removePage(currPage);
    //  swapPageInfo->pageTableEntry = NULL;

        if (currPage->valid == TRUE)
        {
            //int currPID = currPage->space->getPCB()->getPID();
            int currPID = space->getPCB()->getPID();
            DEBUG('v', "E %d: %d\n", currPID, currPage->virtualPage);
            memoryManager->clearPage(currPage->physicalPage);
            physicalMemoryInfo[currPage->physicalPage].space = NULL; 
        }
        swapSectorMap->Clear((currPage->locationOnDisk) / PageSize);
    }
}

/*
 * After selecting a slot of physical memory as a victim and taking care of
 * synchronizing the data if needed, we load the faulting page into memory.
*/
void VirtualMemoryManager::loadPageToCurrVictim(int virtAddr)
{
    int pageTableIndex = virtAddr / PageSize;
    TranslationEntry* page = currentThread->space->getPageTableEntry(pageTableIndex);
    char* physMemLoc = machine->mainMemory + page->physicalPage * PageSize;
    int swapSpaceLoc = page->locationOnDisk;
    swapFile->ReadAt(physMemLoc, PageSize, swapSpaceLoc);

  //  int swapSpaceIndex = swapSpaceLoc / PageSize;
 //   SwapSectorInfo * swapPageInfo = swapSpaceInfo + swapSpaceIndex;
    page->valid = TRUE;
//    swapPageInfo->setValidBit(TRUE);
//    swapPageInfo->setPhysMemPageNum(page->physicalPage);
}

/*
 * Helper function for the second chance page replacement that retrieves the physical page
 * which corresponds to the given physical memory page information that the
 * VirtualMemoryManager maintains.
 * This return page table entry corresponding to a physical page
 */
TranslationEntry* VirtualMemoryManager::getPageTableEntry(FrameInfo * physPageInfo)
{
    TranslationEntry* page = physPageInfo->space->getPageTableEntry(physPageInfo->pageTableIndex);
    return page;
}

void VirtualMemoryManager::copySwapSector(int to, int from)
{
    char sectorBuf[SectorSize];
    swapFile->ReadAt(sectorBuf, SWAP_SECTOR_SIZE, from);
    swapFile->WriteAt(sectorBuf, SWAP_SECTOR_SIZE, to);
}
