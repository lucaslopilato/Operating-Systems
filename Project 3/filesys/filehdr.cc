// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    // Ensure file will actually fit on disk before starting
    if (fileSize > MaxFileSize)
    {
        printf("Unable to save file of size %d to disk, max size is %d.\n", fileSize, MaxFileSize);
        return FALSE;
    }

    numBytes = fileSize;
    numSectors = divRoundUp(fileSize, SectorSize);
    numIndirectBlocks = 0;

    if (freeMap->NumClear() < numSectors)
    {
        printf("Unable to save file of size %d, not enough disk space.\n", fileSize);
	return FALSE;
    }

    int sectorsLeftToAllocate = numSectors;
    int currIBIndex = 0;

    while (sectorsLeftToAllocate > 0)
    {
        // Alloc a new indirect block
        indirectBlocks[currIBIndex] = new IndirectBlock();
        numIndirectBlocks++;
        sectorsForIndirectBlocks[currIBIndex] = freeMap->Find();

        // Find out how many sectors needed for this IB
        int sectorsForThisIB = sectorsLeftToAllocate > DataSectorsPerIndirect
                               ? DataSectorsPerIndirect
                               : sectorsLeftToAllocate;

        // Allocate the needed sectors
        for (int i = 0; i < sectorsForThisIB; i++)
        {
            int currDirectSectorIndex = freeMap->Find();
            indirectBlocks[currIBIndex]->PutSector(currDirectSectorIndex);
            sectorsLeftToAllocate--;
        }
        currIBIndex++;
    }
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    int numIndirects = divRoundUp(numSectors, DataSectorsPerIndirect);
    for (int i = 0; i < numIndirects; i++)
    {
        indirectBlocks[i]->Deallocate(freeMap);
        freeMap->Clear(sectorsForIndirectBlocks[i]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    // 1. Read in this FileHeader from its location on disk.
    char* buffer = new char[SectorSize];
    bzero(buffer, SectorSize);
    synchDisk->ReadSector(sector, buffer);
    bcopy(buffer, (char*) this, HeaderSize);

    // 2. Load in all associated indirect blocks from disk.
    int numIndirects = divRoundUp(numSectors, DataSectorsPerIndirect);
    for (int i = 0; i < numIndirects; i++)
    {
        int storedIBSectorNum = sectorsForIndirectBlocks[i];
        ASSERT(storedIBSectorNum >= 2);
        ASSERT(storedIBSectorNum < (NumIndirectBlocks * DataSectorsPerIndirect + 2));

        indirectBlocks[i] = new IndirectBlock();
        indirectBlocks[i]->FetchFrom(storedIBSectorNum);
    }
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    // 1. Write this FileHeader to specified location on disk.
    char* buffer = new char[SectorSize];
    bzero(buffer, SectorSize);
    ASSERT(HeaderSize == SectorSize);
    bcopy((char*) this, buffer, HeaderSize);
    synchDisk->WriteSector(sector, buffer);

    // 2. Write all associated indirect blocks to disk.
    int numIndirects = divRoundUp(numSectors, DataSectorsPerIndirect);
    for (int i = 0; i < numIndirects; i++)
    {
        int storedIBSectorNum = sectorsForIndirectBlocks[i];
        ASSERT(storedIBSectorNum >= 2);
        ASSERT(storedIBSectorNum < (NumIndirectBlocks * DataSectorsPerIndirect + 2));

        indirectBlocks[i]->WriteBack(storedIBSectorNum);
    }
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    // 1. Find out what indirect block contains the byte
    int indirectBlockIndex = divRoundDown(offset, BytesPerIB);
    int offsetWithinIB = (offset >= BytesPerIB) ? (offset % BytesPerIB) : offset;

    // 2. Query that indirect block for what sector contains the byte
    int resultingSector = indirectBlocks[indirectBlockIndex]->ByteToSector(offsetWithinIB);
    ASSERT(resultingSector >= 2); // Non-negative, can't be 0 or 1 b/c directory & freemap
    return resultingSector;
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//
//	*** TEMPORARILY COMMENTED OUT UNTIL FIX FOR PRINTING WITH INDIRECTION ***
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    /*
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
    */
}

//----------------------------------------------------------------------
// FileHeader::SetNumBytes
//     Pretty self-explanatory.
//----------------------------------------------------------------------

void FileHeader::SetNumBytes(int bytes)
{
    numBytes = bytes;
}

//----------------------------------------------------------------------
// FileHeader::ExtendFile
//     Allows the file size to be extended by numSectors, therefore if a 
//     user wants to "grow" a file after its creation, they can.
//----------------------------------------------------------------------

bool FileHeader::ExtendFile(int numSectors)
{
    int sectorsLeftToAllocate = numSectors;
    ASSERT(sectorsLeftToAllocate > 0);

    OpenFile* freeMapFile = new OpenFile(FreeMapSector);
    BitMap* freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    if (freeMap->NumClear() < sectorsLeftToAllocate)
    {
        return FALSE;
    }

    int currIBIndex = numIndirectBlocks - 1;

    while (sectorsLeftToAllocate > 0)
    {
        int numDataSectorsFreeInCurrIB = 0;
        int sectorsForThisIB = 0;

        if (currIBIndex != -1) {
            numDataSectorsFreeInCurrIB = DataSectorsPerIndirect - indirectBlocks[currIBIndex]->GetNumSectorsAlloc();
        }
        
        if ((numDataSectorsFreeInCurrIB <= 0) || (currIBIndex == -1))
        {
            if (freeMap->NumClear() < sectorsLeftToAllocate + 1) // add 1 for IB's direct sector
            {
                return FALSE;
            }
            currIBIndex++;
            numIndirectBlocks++;
            indirectBlocks[currIBIndex] = new IndirectBlock();
            sectorsForIndirectBlocks[currIBIndex] = freeMap->Find();

            sectorsForThisIB = sectorsLeftToAllocate > DataSectorsPerIndirect
                             ? DataSectorsPerIndirect
                             : sectorsLeftToAllocate;
        }
        else // numDataSectorsFreeInCurrIB > 0 // Add more direct sectors to current indirect block
        {
            if (freeMap->NumClear() < sectorsLeftToAllocate)
            {
                return FALSE;
            }
            sectorsForThisIB = sectorsLeftToAllocate > numDataSectorsFreeInCurrIB
                             ? numDataSectorsFreeInCurrIB
                             : sectorsLeftToAllocate;
        }


        ASSERT(sectorsForThisIB > 0);
        for (int i = 0; i < sectorsForThisIB; i++)
        {
            int currDirectSectorIndex = freeMap->Find();
            indirectBlocks[currIBIndex]->PutSector(currDirectSectorIndex);
            sectorsLeftToAllocate--;
            numSectors++;
            numBytes += SectorSize;
        }
    }
    freeMap->WriteBack(freeMapFile);
    delete freeMap;
    delete freeMapFile;
    return TRUE;
}
