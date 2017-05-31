// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "bitmap.h"
#include "IndirectBlock.h"

#define FreeMapSector       0
#define DirectorySector     1
#define FreeMapSize         sizeof(int)
#define DirectorySize       sizeof(int)
#define NumIndirectBlocks   (SectorSize - FreeMapSize - DirectorySize) / sizeof(int)
#define HeaderSize          FreeMapSize + DirectorySize + (NumIndirectBlocks * sizeof(int))
#define MaxFileSize         NumIndirectBlocks * DataSectorsPerIndirect * SectorSize
#define BytesPerIB          (DataSectorsPerIndirect * SectorSize)

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
    bool Allocate(BitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.
    bool ExtendFile(int numSectors);    // extend the file by numSectors
    void SetNumBytes(int numBytes);     // Allows the file to be extended

  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    int numIndirectBlocks;              // Number of indirect blocks alloc for this

    // E.g. the 0th indirect block is stored in data sector sectorsForIndirectBlocks[0].
    //
    int sectorsForIndirectBlocks[NumIndirectBlocks];  // stores phys location of indirect blocks
    IndirectBlock* indirectBlocks[NumIndirectBlocks]; // stores a list of indirect blocks
};

#endif // FILEHDR_H
