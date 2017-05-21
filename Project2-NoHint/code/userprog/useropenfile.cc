// useropenfile.cc
//

#include "useropenfile.h"
 
/* Implement This */

UserOpenFile::UserOpenFile(char* fileName, int ftIndex, int currPosition)
{
    this->filename = fileName;
    this->fileTableIndex = ftIndex;
    this->currentPosition = currPosition;
}

UserOpenFile::UserOpenFile(const UserOpenFile* other){
    this->filename = other->filename;
    this->fileTableIndex = other->fileTableIndex;
    this->currentPosition = other->currentPosition;
}