#include "openfilemanager.h"
#include <string.h> // strcmp
//-----------------------------------------------------------------------------
// OpenFileManager::OpenFileManager
//
//     Constructor
//-----------------------------------------------------------------------------

OpenFileManager::OpenFileManager() : sysOpenFilesMap(MAX_SYS_OPEN_FILES) {
    consoleWriteLock = new Lock("consoleWriteLock");
}

//-----------------------------------------------------------------------------
// OpenFileManager::~OpenFileManager
//
//     Destructor
//-----------------------------------------------------------------------------

OpenFileManager::~OpenFileManager() {}

//-----------------------------------------------------------------------------
// OpenFileManager::addFile
//
//     Adds a file to the list of currently open system files.
//-----------------------------------------------------------------------------

int OpenFileManager::addFile(SysOpenFile file) {

    int fileIndex = sysOpenFilesMap.Find();
    if (fileIndex == -1) {
        printf("No more room for any more open system files.\n");
        return -1;
    } else {
        sysOpenFileList[fileIndex] = file;
        return fileIndex;
    }
}

//-----------------------------------------------------------------------------
// OpenFileManager::getFile
//
//     Returns the system open file of that filename and its index.
//-----------------------------------------------------------------------------

SysOpenFile* OpenFileManager::getFile(char* filename, int& index) {

    for (int i = 2; i < MAX_SYS_OPEN_FILES; i++) {
        
        if (sysOpenFilesMap.Test(i)) {
            if (!strcmp(filename, sysOpenFileList[i].filename)) {
                index = i;
                return &sysOpenFileList[index];
            }
        }
    }
    return NULL; // couldn't find a file by that name
}

//-----------------------------------------------------------------------------
// OpenFileManager::getFile
//
//     Same as above, but use the index into array if we know that information.
//-----------------------------------------------------------------------------

SysOpenFile* OpenFileManager::getFile(int index) {

    if (sysOpenFilesMap.Test(index)) {
        return sysOpenFileList + index;
    } else {
        return NULL;
    }
}
