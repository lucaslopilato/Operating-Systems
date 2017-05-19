/*
 * close-test.c
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
	char readBuf[100];
	int openFileId;

	print("Opening file...\n");
	openFileId = Open("close1.txt");

	Read(readBuf, 10, openFileId);
	readBuf[10] = '\0';

	print("Read first 10 bytes: ");
	print(readBuf);

	Read(readBuf, 10, openFileId);
	readBuf[10] = '\0';

	print("\nRead next 10 bytes: ");
	print(readBuf);

	print("\nClosing file...\n");
	Close(openFileId);

	print("Opening file again...\n");
	openFileId = Open("close1.txt");

	Read(readBuf, 10, openFileId);
	readBuf[10] = '\0';

	print("Reading first 10 bytes again: ");
	print(readBuf);
	print("\n");

	Halt();
}
