#include "syscall.h"


char buffer[1024*9];
char buffer2[1024*9];


int
main()
{
    int id2;
    int i;
    Create("aaaa");
    id2 = Open("aaaa");

    for (i = 0; i < 1024*9; i++) {
      buffer[i] = 'a' + (i % 26);
    }
    for (i = 0; i < 1024*9; i++) {
      buffer2[i] = buffer[i];
    }
    Write(buffer2, 4, id2);
    Write("Done\n", 5, ConsoleOutput);
    Close(id2);
}


