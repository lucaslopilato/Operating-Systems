// openfilemanager.h
//

#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include "sysopenfile.h"
#include "synch.h"

#define OPEN_FILE_TABLE_SIZE 32
#define ERR_FAIL_FIND -1
#define ERR_FAIL_ADD -1

class OpenFileManager {

public:
    OpenFileManager();
    ~OpenFileManager();

    int addOpenFile(OpenFile* openFile, char* fileName);
    SysOpenFile *getOpenFile(int index);

    /* Added to original code */ 
    void reduceOpenFiles(int index);
    /* End of changes */

    Lock *consoleWriteLock;
    
private:
    SysOpenFile* openFileTable[OPEN_FILE_TABLE_SIZE];

    /* Added to original code */
    int getFileIndex(OpenFile* file);
    int usedFileSpace;
    /* End of changes */

};

#endif // OPENFILEMANAGER_H
