/*
 * create-test.c
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
	char readBuf[100], *writeBuf = "Indubitably";
	int writeOpenFileId, readOpenFileId;

	print("Creating new file...\n");
	Create("proj2-test-file.txt");

	print("Open said file...\n");
	writeOpenFileId = Open("proj2-test-file.txt");
	readOpenFileId = Open("proj2-test-file.txt");

	print("Writing said file...\n");
	Write(writeBuf, 11, writeOpenFileId);

	print("Reading said file...\n");
	Read(readBuf, 11, readOpenFileId);

	readBuf[11] = '\0';

	print("File contains: ");
	print(readBuf);

	Halt();
}
