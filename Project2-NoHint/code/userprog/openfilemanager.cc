// openfilemanager.cc
//

#include "openfilemanager.h"

//----------------------------------------------------------------------
// OpenFileManager::OpenFileManager
//  Construct and initialize the open file manager.
//----------------------------------------------------------------------

OpenFileManager::OpenFileManager()
{
   openFileTable = new SysOpenFile*[OPEN_FILE_TABLE_SIZE];
   for(int i=0; i<OPEN_FILE_TABLE_SIZE; i++)
    openFileTable[i] = NULL;
   consoleWriteLock = new Lock("consoleWriteLock");
   usedFileSpace = 0;    // added
}

//----------------------------------------------------------------------
// OpenFileManager::~OpenFileManager
// ** Implement this **
//----------------------------------------------------------------------

OpenFileManager::~OpenFileManager()
{
    for(int i=0; i<OPEN_FILE_TABLE_SIZE; i++){
        if(openFileTable[i] != NULL){
            delete openFileTable[i];
        }
    }
    delete[] openFileTable;
    delete consoleWriteLock;
}

//----------------------------------------------------------------------
// OpenFileManager::addOpenFile
//  Adds an on open file to the system file table.
// ** Implement this **
//----------------------------------------------------------------------

int OpenFileManager::addOpenFile(OpenFile* openFile, char* fileName)
{
    int index = getFileIndex(openFile);
    if(index == ERR_FAIL_FIND){
        if(usedFileSpace == OPEN_FILE_TABLE_SIZE)
            return ERR_FAIL_ADD;
        for(int i=2; i < OPEN_FILE_TABLE_SIZE; i++){
            if(openFileTable[i] == NULL){
                openFileTable[i] = new SysOpenFile(openFile, i, fileName);
                usedFileSpace++;
                return i;
            }
        }
    }
    else{
        openFileTable[index]->numProcessesOpen++;
        return index;
    }
    return ERR_FAIL_ADD;    
}

//----------------------------------------------------------------------
// OpenFileManager::getOpenFile
//  Retrieves the system file table entry from the file table.
//  ** Implement this **
//----------------------------------------------------------------------

SysOpenFile *OpenFileManager::getOpenFile(int index)
{
    if(openFileTable[index] == NULL)
        return NULL;

    return openFileTable[index];
}


//----------------------------------------------------------------------
// OpenFileManager::getFileIndex(OpenFile* file)
//  Retrieves the index of the file specified
//  ** Implement this **
//----------------------------------------------------------------------

int OpenFileManager::getFileIndex(OpenFile* file)
{
    for(int i=0; i< OPEN_FILE_TABLE_SIZE; i++){
        if(openFileTable[i] != NULL && openFileTable[i]->openFile == file)
                return i;
    }
    return ERR_FAIL_FIND;
}

//----------------------------------------------------------------------
// OpenFileManager::reduceOpenFiles(int index)
//  Reduce the number of processes using an open file
//  ** Implement this **
//----------------------------------------------------------------------

void OpenFileManager::reduceOpenFiles(int index)
{
    openFileTable[index]->reduceOpenProcesses();
    if(openFileTable[index]->numProcessesOpen == 0){
        delete openFileTable[index];
        usedFileSpace--;
        openFileTable[index] = NULL;
    }
} 

SysOpenFile* OpenFileManager::getFile(char* filename, int& index) {

    for (int i = 2; i < OPEN_FILE_TABLE_SIZE; i++) {
        if(openFileTable[i] != NULL && openFileTable[i]->filename != NULL){
            if (!strcmp(filename, openFileTable[i]->filename)) {
                index = i;
                return openFileTable[index];
            }
        }
    }
    return NULL; // couldn't find a file by that name
}