/*
 * open-test.c
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
	char readBuf1[100], readBuf2[100], readBuf3[100];
	int openFileId1, openFileId2, openFileId3;

	openFileId1 = Open("open1.txt");
	openFileId2 = Open("open2.txt");
	openFileId3 = Open("open3.txt");

	Read(readBuf1, 10, openFileId1);
	Read(readBuf2, 10, openFileId2);
	Read(readBuf3, 10, openFileId3);

	readBuf1[10] = '\0';
	readBuf2[10] = '\0';
	readBuf3[10] = '\0';

	print("open1.txt: ");
	print(readBuf1);
	print("\nopen2.txt: ");
	print(readBuf2);
	print("\nopen3.txt: ");
	print(readBuf3);

	Halt();
}
