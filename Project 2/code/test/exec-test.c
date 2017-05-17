/*
 * exec-test.c
 */

#include <syscall.h>

void print(char *s)
{
	int len = 0;

	while (*s++)
		++len;

	Write(s-len-1, len, 1);
}

main()
{
	print("About to Exec() some stuff...\n");
	Exec("exec1", 0, 0, 0);
	Exec("exec2", 0, 0, 0);
	Exec("exec3", 0, 0, 0);
	print("Preparing to Halt()...\n");
	//Halt();
}
