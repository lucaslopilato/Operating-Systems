/*
 * join1.c
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

	print("proc1 Execing proc2\n");
	pid = Exec("join2", 0, 0, 0);

	print("proc1 waiting for proc2\n");
	Join(pid);

	print("proc1 Exiting\n");
	Exit(1);
}
