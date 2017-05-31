#include <syscall.h>

#define PageSize 128
#define NumPhysPages 100

char very_large_array[PageSize * NumPhysPages * 2];

void print(char *s)
{
	int len = 0;

	while (*s++)
		++len;

	Write(s-len-1, len, 1);
}

main()
{
	int i;

	for (i = 0; i < NumPhysPages * 2; ++i) {
		*(int *)(&very_large_array[i * PageSize]) = i;
	}

	print("Attempted to write to 200 pages.\n");

	for (i = 0; i < NumPhysPages * 2; ++i) {
		if (*(int *)(&very_large_array[i * PageSize]) != i) {
			print("Paging failed! Read wrong value from swapped page!\n");
		}
	}

	print("Attempted to read from 200 pages.\n");

	Halt();
}
