/*
 * join2.c
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

	print("proc2 Execing proc3\n");
	pid = Exec("join3", 0, 0, 0);

	print("proc2 waiting for proc3\n");
	Join(pid);

	print("proc2 Exiting\n");
	Exit(1);
}
