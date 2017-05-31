#include "syscall.h"


char buffer[1024*9];
char buffer2[1024*9];


int
main()
{
    int id2;
    int i;
    for (i = 0; i < 1024*9; i++) {
      buffer[i] = 'a' + (i % 26);
    }
    Yield();
    for (i = 0; i < 1024*9; i++) {
      buffer2[i] = buffer[i];
    }
    if(buffer2[0]=='a')
	Write("Done\n", 5, ConsoleOutput);
    else
	Write("Failed\n", 7, ConsoleOutput);
   Exit(0);
}


