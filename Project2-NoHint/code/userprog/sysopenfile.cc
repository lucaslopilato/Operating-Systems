// sysopenfile.cc
//

#include "sysopenfile.h"

/* Implement This*/

SysOpenFile::SysOpenFile(OpenFile* file, int fileID, char* fileName)
{
    this->openFile = file;
    this->fileId = fileId;
    this->filename = fileName;
    this->numProcessesOpen = 1;
}

void SysOpenFile::reduceOpenProcesses(){
    this->numProcessesOpen--;
}

SysOpenFile::~SysOpenFile(){
    if(numProcessesOpen == 0)
        delete openFile;
}
