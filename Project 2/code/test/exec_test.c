#include "syscall.h"

main()
{
	char *str = "Greetings from the parent!\n";

	Write(str, 28, 1);

	/*Exec("/home/dkudrow/src/cs170_s15/proj2/code/test/hello");*/
	Exec("../test/hello", 0, 0, 0);

	/* should not reach here... */
	Halt();
}
