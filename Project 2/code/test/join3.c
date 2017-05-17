/*
 * join3.c
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
	print("proc3: Thanks for waiting fellas!\n");
	Exit(1);
}
