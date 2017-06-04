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
	int pid1,pid2,pid3;

	print("About to Exec() some stuff...\n");
	pid1 = Exec("exec1");
	pid2 = Exec("exec2");
	pid3 = Exec("exec2");
	//pid1 = Exec("exec1", 0, 0, 0);
	//pid2 = Exec("exec2", 0, 0, 0);
	//pid3 = Exec("exec2", 0, 0, 0);
	print("About to join child processes...\n");
	Join(pid1);
	Join(pid2);
	Join(pid3);
	Exit(0);

}
