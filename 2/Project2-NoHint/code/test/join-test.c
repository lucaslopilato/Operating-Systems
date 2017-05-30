/*
 * join-test.c
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
	int pid;

	print("main Execing proc1\n");
	pid = Exec("join1", 0, 0, 0);

	print("main waiting proc1\n");
	Join(pid);

	print("Preparing to Halt()...\n");
	Halt();
}
