/*
 * IndirectBlock implementation
 *
 * This class provides a way for a file header to contain not just direct
 * sectors, but indirect blocks that each have their individual collection of
 * sectors.  This allows the file size to exceed the usual 4K.
 */

#include "IndirectBlock.h"
#include "system.h" // synchDisk

//-----------------------------------------------------------------------------
// IndirectBlock::IndirectBlock
//     Constructor
//-----------------------------------------------------------------------------

IndirectBlock::IndirectBlock()
{
    numSectors = 0;
    for (int i = 0; i < DataSectorsPerIndirect; i++)
    {
        dataSectors[i] = -1;
    }
}

//-----------------------------------------------------------------------------
// IndirectBlock::WriteBack
//     Writes the modified object back to the specified sector on disk.
//-----------------------------------------------------------------------------

void IndirectBlock::WriteBack(int sectorNum)
{
    synchDisk->WriteSector(sectorNum, (char*) this);
}

//-----------------------------------------------------------------------------
// IndirectBlock::FetchFrom
//     Retrieves the serialized object from disk.
//-----------------------------------------------------------------------------

void IndirectBlock::FetchFrom(int sectorNum)
{
    synchDisk->ReadSector(sectorNum, (char*) this);
}

//-----------------------------------------------------------------------------
// IndirectBlock::PutSector
//     Adds a newly allocated data sector from the file header to this IB's 
//     list of allocated sectors.
//-----------------------------------------------------------------------------

void IndirectBlock::PutSector(int sectorNum)
{
    int indexOfNewSector = numSectors;
    dataSectors[indexOfNewSector] = sectorNum;
    numSectors++;
}

//-----------------------------------------------------------------------------
// IndirectBlock::Deallocate
//     Removes the sectors allocated to this particular IB from the global map
//     of allocated data sectors.  Essentially removes the data held by this
//     IPB.
//-----------------------------------------------------------------------------
void IndirectBlock::Deallocate(BitMap* allocSectorsMap)
{
    for (int i = 0; i < numSectors; i++)
    {
        allocSectorsMap->Clear(dataSectors[i]);
    }
    numSectors = 0;
}

//-----------------------------------------------------------------------------
// IndirectBlock::ByteToSector
//     Returns the disk sector that is corresponding the specified byte at
//     offset from this IB.
//
//     e.g. If SectorSize is 5, and we are looking for the sector holding offset
//          8, we see that 8 / 5 = 1, and conclude that the sector containing
//          byte 8 is sector 1.
//-----------------------------------------------------------------------------

int IndirectBlock::ByteToSector(int offsetFromStartOfIB)
{
    int currIBSectorIndex = offsetFromStartOfIB / SectorSize;
    int resultDataSector = dataSectors[currIBSectorIndex];

    if (resultDataSector <= 0)
    {
        return -1;
    }
    else
    {
        return resultDataSector;
    }
}

//-----------------------------------------------------------------------------
// IndirectBlock::GetNumSectorsAlloc
//     Self-explanatory.
//-----------------------------------------------------------------------------

int IndirectBlock::GetNumSectorsAlloc()
{
    return numSectors;
}
