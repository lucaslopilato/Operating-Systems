/*
 * VirtualMemoryManager header
 *
 * Used to facilitate demand paging through providing a means by which page
 * faults can be handled and pages loaded from and stored to disk.
*/

#ifndef VIRTUAL_MEMORY_MANAGER_H
#define VIRTUAL_MEMORY_MANAGER_H

#include "bitmap.h"

class AddrSpace;
class TranslationEntry;

#define SWAP_SECTORS 512
#define SWAP_SECTOR_SIZE PageSize
#define SWAP_FILENAME "SWAP"

struct FrameInfo //This structure is assocated with each physical page
{
    AddrSpace* space; // Process space currently owrns this particular physical page
    int pageTableIndex; // virtual page number of that process corresponding to this physical page.
};
class VirtualMemoryManager
{
    public:
        VirtualMemoryManager();
        ~VirtualMemoryManager();

        int allocSwapSector();
        void writeToSwap(char *page, int pageSize, int backStoreLoc);
        void swapPageIn(int virtAddr);
        void releasePages(AddrSpace* space);
        void copySwapSector(int to, int from);

        void loadPageToCurrVictim(int virtAddr);
        TranslationEntry* getPageTableEntry(FrameInfo * pageInfo);

    private:
        BitMap *swapSectorMap;
        OpenFile *swapFile;
        FrameInfo *physicalMemoryInfo;
        int nextVictim; // current physical page number to be inspected for page replacement
};

#endif
