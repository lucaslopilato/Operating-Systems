#include "syscall.h"

main()
{
	char *hello_str = "Hello world!\n";

	Write(hello_str, 14, 1);

	Halt();
}
