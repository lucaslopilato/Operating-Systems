// openfilemanager.cc
//

#include "openfilemanager.h"

//----------------------------------------------------------------------
// OpenFileManager::OpenFileManager
//  Construct and initialize the open file manager.
//----------------------------------------------------------------------

OpenFileManager::OpenFileManager()
{
   consoleWriteLock = new Lock("consoleWriteLock");
}

//----------------------------------------------------------------------
// OpenFileManager::~OpenFileManager
//----------------------------------------------------------------------

OpenFileManager::~OpenFileManager()
{
    delete consoleWriteLock;
}

//----------------------------------------------------------------------
// OpenFileManager::addOpenFile
//  Adds an on open file to the system file table.
//----------------------------------------------------------------------

int OpenFileManager::addOpenFile(SysOpenFile openFile)
{
    return 0;
}

//----------------------------------------------------------------------
// OpenFileManager::getOpenFile
//  Retrieves the system file table entry from the file table.
//----------------------------------------------------------------------

SysOpenFile *OpenFileManager::getOpenFile(int index)
{
    return 0;
}
