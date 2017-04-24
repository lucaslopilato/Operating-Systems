
Implement each of the following four options to synchronize the concurrent access to the hashtable from multiple threads.

1. Use a mutex only to synchronize for the entire hash table. Call this approach ``coarse-grain'' synchronization. That corresponds to compiler flag FLAGS1 in the Makefile.
  - phash.cc (solution for coarse grain locking with mutex only and with your RWLock compiled using -DRWLOCK)
  - phash.h (revised from the given hashchain.h)

2. Implement and use a read-write lock for the entire hashtable using Pthread mutex and condition variable following the RWLock design in the text book. That corresponds to compiler flag FLAGS2 defined in the Makefile.

3. Allocate an array of mutex locks and use each of them for each hash table entry. Namely synchronize the access to the linked list associated with each element array table[] in the sample hash table program. Call this approach ``fine-grain'' synchronization. That corresponds to compiler flag FLAGS3 in the Makefile.
  - phash1.cc (solution for fine grain locking with mutex only and with your RWLock compiled using -DRWLOCK) 

4. Allocate an array of RWLocks for fine-grain synchronization of concurrent access to each hash table entry. That corresponds to compiler flag FLAGS4 defined in the Makefile.

You will need to use #ifdef, #ifndef, #else, or #endif preprocessor directives so that the compilation is controlled by the preprocessor symbols FINEGRAIN (meaning fine-grain synchronization control) and RWLOCK (meaning to use your read-write lock), which are passed by the above Makefile flags FLAGS1, FALGS2, FLAGS3, and FLAGS4.
