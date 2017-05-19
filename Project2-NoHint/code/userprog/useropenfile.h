// useropenfile.h
//

#ifndef USEROPENFILE_H
#define USEROPENFILE_H

struct UserOpenFile {
    char *filename;         // This file's name
    int fileTableIndex;     // This file's index in the system open file list
    int currentPosition;    // User's current read/write position in the file
};

#endif //USEROPENFILE_H
