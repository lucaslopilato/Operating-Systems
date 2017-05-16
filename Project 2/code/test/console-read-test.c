/*
 * console-read-test.c
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
	char c;
	int i, rc;

	for (i = 0; i < 10; ++i) {
		rc = Read(&c, 1, 0);
		Write(&c, 1, 1);
	}
	Halt();
}
