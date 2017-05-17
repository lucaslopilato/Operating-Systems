// sysopenfile.h
//

#ifndef SYSOPENFILE_H
#define SYSOPENFILE_H

#include "filesys.h"

struct SysOpenFile {
    OpenFile* openFile; // This file's OpenFile object in fileSystem
    int fileId;         // This file's ID
    char *filename;     // This file's name
};


#endif // SYSOPENFILE_H
