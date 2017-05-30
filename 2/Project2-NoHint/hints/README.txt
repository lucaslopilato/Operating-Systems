This directory contains sample solution files used for completing Project 2.
A small number of lines of code are removed in the following  four files 
and we replace them with a note "Implement me" and some comments on what to be done.


-rw-r--r-- 1 cs170 grad 12003 Apr 28 16:07 addrspace.hint.cc
-rw-r--r-- 1 cs170 grad 20069 Apr 28 16:07 exception.hints.cc
-rwxr--r-- 1 cs170 grad  2518 Apr 28 16:07 pcb.hint.cc*
-rwxr--r-- 1 cs170 grad  4591 Apr 28 16:07 processmanager.hint.cc*



** The following directory contains .cc and .h files that support the above 4 files.
No change has been made in these files.

drwxr-xr-x 2 cs170 grad  4096 Apr 28 16:07 NOCHANGE/


** Notice that during the Nachos initialization (threads/main.cc), the following objects are allocated (threads/system.cc), 

MemoryManager *memoryManager;       // memory manager for a set of free memory pages

ProcessManager *processManager;     // process manager for a set of free PIDs, PCBs,
	Each PCB also points to a set of openned files (UserOpenFile) and each of them points to a system wide
	SysOpenFile object (See below), plus a file offset. Make sure this offset moves during Read() and Write()

OpenFileManager *openFileManager;   // open file manager for a set of SysOpenFile objects, namely all opened file descriptors
char diskBuffer[PageSize];          // Use this disk buffer for  buffer all of your user-level file I/O. 
Lock *diskBufferLock;		   //  Use this Lock for accessing diskBuffer if needed. 
