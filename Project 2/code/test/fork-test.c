/*
 * fork-test.c
 */

#include <syscall.h>

void print(char *s)
{
	int len = 0;

	while (*s++)
		++len;

	Write(s-len-1, len, 1);
}

proc3()
{
	print("proc3 halting the system...\n");
	Halt();
}

proc2()
{
	print("proc2 forking proc3...\n");
	Fork(proc3);
	Exit(0);
}

proc1()
{
	print("proc1 forking proc2...\n");
	Fork(proc2);
	Exit(0);
}

main()
{
	print("main forking proc1...\n");
	Fork(proc1);
}
