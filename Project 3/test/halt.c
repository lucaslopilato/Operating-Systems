#include "syscall.h"

char buffer[512];

int main()
{
    int id;
    int i;
    Create("bigf2");
    id = Open("bigf2");

    for (i = 0; i < 512; i++) 
        buffer[i] = 'a' + (i % 26);

    for (i = 0; i < 80; i++) 
        Write(buffer, 512, id);

    Close(id);
    Exit(10);
}
