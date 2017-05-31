/*
 * IndirectBlock header
 *
 * This class provides a way for a file header to contain not just direct
 * sectors, but indirect blocks that each have their individual collection of
 * sectors.  This allows the file size to exceed the usual 4K.
 */

#ifndef INDIRECT_POINTER_BLOCK_H
#define INDIRECT_BLOCK_H

#include "disk.h"
#include "bitmap.h"

// IndirectBlock needs to fit in one sector.  Leave space for
// "numSectors" and the rest of the space is "dataSectors"
// e.g. if SectorSize is 128, then this evaluates to 31 data sectors
#define DataSectorsPerIndirect ((SectorSize - sizeof(int)) / sizeof(int))

class IndirectBlock
{
    public:
        IndirectBlock();
        void WriteBack(int sectorNum);              // writes the object to the sector of disk
        void FetchFrom(int sectorNum);              // retrieve this object from sector of disk
        void PutSector(int sectorNum);              // add newly alloc sector to dataSectors
        void Deallocate(BitMap* allocSectorsMap);   // removes this object's alloc sectors from map
        int  ByteToSector(int sectorNum);           // returns the sector num for byte offset
                                                    // relative to beginning of the IPB
        int  GetNumSectorsAlloc();                  // returns numSectors

    private:
        int numSectors;                             // curr num of sectors allocated
        int dataSectors[DataSectorsPerIndirect]; // disk sector indexes for this IPB
};

#endif // INDIRECT_BLOCK_H
