// threadtest-submit.cc
//        Set of test cases for Project 1B
//
//  NOTES:
//    - uses testnum (parsed in main.cc as -q flag) to determine which test(s) to run
//      - -1 means run all tests
//      - 1-n are the individual tests (where there are n tests defined, see TestNumber below)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.


#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "hashchain.h"
#include <iostream>
#include "rwlock.h"
using namespace std;

#define YIELD() currentThread->Yield()


enum TestNumber {
  ALL_TESTS=-1,
  INVALID_TEST=0,
  TEST_READ_K, //single get to center key for each thread
  TEST_PUT_K, //single put to center key for each thread
  TEST_LOCK, //test whether we can acquire/release lock
  TEST_SIGNAL, //test whether we can signal a thread waiting on a condition variable
  TEST_BCAST, //test whether we can broadcast to all waiting threads
  TEST_WAIT, //test whether we actually wait when we call Condition::Wait()
  TEST_RW_WEXLCUDE, //test whether RWLock prevents multiple writers at once
  TEST_RW_RWEXLCUDE, //test whether RWLock prevents readers and writers at the same time
  TEST_RW_MULTIPLE_R, //test whether RWLock allows multiple readers at the same time
  TEST_SEQUENCE, //basic sequence of gets, puts, and deletes
  TEST_READ, //simple read-only sequence
  TEST_PUT, //sequence of put statements (first put writes 0, second put writes correct value i)
  TEST_DELPUT, //sequence of deletes and puts
  TEST_PUTGET, //sequence of puts and gets. set up so there are write conflicts, not not on k, so we should always be the last person to write to key k
  TEST_KEYHIT, //sequences of puts/deletes for the same key
  TEST_RWHIT, //sequence of modifications that should all land in the same bucket (to hit the rwlock harder).
  TEST_INC, //sequence of increments. If we didn't miss anything, the final value of key n should be the number of threads * the number of iterations * 100
};


// testnum is set in main.cc
int testnum = TEST_SEQUENCE;

const char* get_test_name(enum TestNumber n) {
  switch(n) {
    case ALL_TESTS: return "All Tests";
    case TEST_READ_K: return "HashMap::get()";
    case TEST_PUT_K: return "HashMap::put()";
    case TEST_LOCK: return "Lock mutual exclusion";
    case TEST_SIGNAL: return "Condition::Signal()";
    case TEST_BCAST: return "Condition::Broadcast()";
    case TEST_WAIT: return "Condition::Wait()";
    case TEST_RW_WEXLCUDE: return "RWLock W mutual exclusion()";
    case TEST_RW_RWEXLCUDE: return "RWLock R+W mutual exclusion()";
    case TEST_RW_MULTIPLE_R: return "RWLock multiple readers";
    case TEST_SEQUENCE: return "simple HashMap sequence";
    case TEST_READ: return "HashMap read sequence";
    case TEST_PUT: return "HashMap put sequence";
    case TEST_DELPUT: return "HashMap del+put sequence";
    case TEST_PUTGET: return "HashMap put+get sequence";
    case TEST_KEYHIT: return "HashMap single key hitting sequence";
    case TEST_RWHIT: return "HashMap bucket hitting sequence";
    case TEST_INC: return "HashMap increment";
    default: return NULL;
  }
  return NULL; //just for complainy compilers
}

HashMap m;
Lock lock((char*)"test Lock");
volatile int lock_count=0;


Lock wait_lock((char*)"Condition locker");

Condition sig_cond((char*)"test condition for signal");
Semaphore sig_sem((char*)"signal barrier",0);

Condition bcast_cond((char*)"test condition for bcast");
Semaphore bcast_sem((char*)"broadcast barrier",0);

Condition wait_cond((char*)"test condition for wait");
Semaphore wait_sem((char*)"wait barrier",0);
volatile int wait_count=0;

RWLock rwlock;
int rw_w_count=0; //W mutual exclusion test counter
int rw_rw_count=0; //R+W mutual exclusion test counter
int rw_r_count=0; //R mutual exclusion test counter (for when not in P1_RWLOCK)


#if defined(CHANGED) && defined(THREADS)

/* Test operations on hash table */
const int NumThreads = 100; //number of threads used in testing.
const int NumKeys=500; //number of keys used in testing. note that for some of the predefined tests NumKeys must be >=5*NumThreads for correct testing
const int NumIterations=3; //number of times to cycle through the active tests
int return_codes[NumThreads];

const int TABLE_SIZE=128; //this should match the table size from hashchain.h/cc (used for test 6)



//simple multiple-use barrier (used to line-up or keep separate some tests)
//  - implemented using only semaphores so it doesn't rely on project 1B code
class Barrier {
  private:
    Semaphore mutex; //lock over barrier state
    Semaphore sem; //semaphore for releasing threads
    volatile int count; //counter on how many threads have entered barrier
    volatile int rcount; //counter on how many threads still need to exit barrier
  public:
    Barrier() : count(0), rcount(0), mutex((char*)"barrier mutex",1), sem((char*)"barrier semaphore",0) {}

    void wait(int N) { //barrier that waits for N threads to enter before continuing (your responsibility to make sure all threads use same N for same barrier)
      mutex.P(); //lock
      while(rcount) { //if other threads are still waiting to get out of the previous barrier, yield to them
        mutex.V();
        YIELD();
        mutex.P();
      }
      ++count; //increment barrier count

      if (count>=N) { //if we are the last one in we get to release everyone else
        rcount=count-1;
        mutex.V(); //unlock

        //now release everyone else
        for(--count;count;--count) { //first decrement is for us because we don't wait on barrier_sem
          sem.V(); //trigger waiting thread
        }
      } else { //else wait to be triggered by the last one in
        mutex.V(); //unlock
        sem.P(); //wait for trigger
        mutex.P(); --rcount; mutex.V(); //decrement rcount to set up the next barrier
      }
    }
};

Barrier test_barrier; //for synchronizing tests
Barrier iter_barrier; //for synchronizing test iterations


/* Each thread will use this function to access the hashtable. You can use
 * this to test your locking mechanism used to protect the hash table */
void tfunc(int id){
  //testNumber = 1;
  //test();
  //cout << "Testing htable\n";
  int i;
  int n=NumKeys; //number of keys used
  int k=(id * n) / NumThreads; //personal key for this thread (for tests that use a per-thread key)
  int w=4; //width of current test
  int err=0;
  int t; //temp var
  switch(testnum) {
    case ALL_TESTS: //run through all tests
    case TEST_READ_K: //single get to center key for each thread
      if (m.get(k) != k) err=1;
      if (testnum != ALL_TESTS) break;
    case TEST_PUT_K: //single put to center key for each thread
      m.put(k,k);
      if (testnum != ALL_TESTS) break;
    case TEST_LOCK: //test whether lock enforces mutual exclusion
      test_barrier.wait(NumThreads);
      lock.Acquire();
      t=lock_count;
      YIELD(); //yield to increase chances of failing if we don't have proper locking
      lock_count=t+1;
      lock.Release();
      if (testnum != ALL_TESTS) break;
    case TEST_SIGNAL: //test whether we can signal a thread waiting on a condition variable
      if (id == 0) { //thread zero does the signalling (signals once for each thread)
        for(int i=1; i<NumThreads;++i) {
          sig_sem.P();
          wait_lock.Acquire();
          sig_cond.Signal(&wait_lock);
          wait_lock.Release();
        }
      } else { //remaining threads lock,inc semaphore, wait for signal, and unlock
        wait_lock.Acquire();
        sig_sem.V();
        sig_cond.Wait(&wait_lock);
        wait_lock.Release();
      }
      if (testnum != ALL_TESTS) break;
    case TEST_BCAST: //test whether we can broadcast to all waiting threads
      if (id == 0) { //thread zero does the signalling (waits for all threads to start waiting, then broadcasts)
        for(int i=1; i<NumThreads;++i) {
          bcast_sem.P();
        }
        wait_lock.Acquire();
        bcast_cond.Broadcast(&wait_lock);
        wait_lock.Release();
      } else { //remaining threads lock,inc semaphore, wait for signal, and unlock
        wait_lock.Acquire();
        bcast_sem.V();
        bcast_cond.Wait(&wait_lock);
        wait_lock.Release();
      }
      if (testnum != ALL_TESTS) break;
    case TEST_WAIT: //test whether we actually wait in Condition::Wait
      if (id == 0) { //thread zero does the signalling (waits for all threads to start waiting, then verifies wait and broadcasts)
        for(int i=1; i<NumThreads;++i) { //wait for all threads to get into wait state
          wait_sem.P();
        }
        wait_lock.Acquire();
        if (wait_count != 0) {
          err |= 1;
          cout << "Condition::Wait test failed\n";
        }
        wait_cond.Broadcast(&wait_lock);
        wait_lock.Release();
      } else { //remaining threads lock,inc semaphore, wait for signal, and unlock
        wait_lock.Acquire();
        wait_sem.V();
        wait_cond.Wait(&wait_lock);
        wait_count++;
        wait_lock.Release();
      }
      if (testnum != ALL_TESTS) break;
    case TEST_RW_WEXLCUDE: //test whether RWLock prevents multiple writers at once
      test_barrier.wait(NumThreads); //barrier to increase chances of collision
      rwlock.startWrite();
      t=rw_w_count;
      YIELD();
      rw_w_count=t+1;
      rwlock.doneWrite();
      if (testnum != ALL_TESTS) break;
    case TEST_RW_RWEXLCUDE: //test whether RWLock prevents readers and writers at the same time
      test_barrier.wait(NumThreads); //barrier to increase chances of collision
      if(id==0) { //thread 0 grabs read lock once for each thread and increments
        for(i=0;i<NumThreads;++i) {
          rwlock.startRead();
          t=rw_rw_count;
          YIELD();
          rw_rw_count=t+1;
          rwlock.doneRead();
        }
      }
      //each thread grabs write lock and increments once (including thread 0)
      rwlock.startWrite();
      t=rw_rw_count;
      YIELD();
      rw_rw_count=t+1;
      rwlock.doneWrite();
      if (testnum != ALL_TESTS) break;

    case TEST_RW_MULTIPLE_R: //test whether RWLock allows multiple readers at the same time
      test_barrier.wait(NumThreads); //this must be the only test running to make sure other rwlock operations don't conflict
      rwlock.startRead();
#ifdef P1_RWLOCK
      //for P1_RWLOCK RWLock MUST allow multiple readers (so we barrier inside the read lock)
      test_barrier.wait(NumThreads);
#else
      //outside of P1_RWLOCK RWLock MUST NOT allow multiple readers (so we increment a counter in an unsafe way inside the read lock)
      t=rw_r_count;
      YIELD();
      rw_r_count=t+1;
#endif
      rwlock.doneRead();
      if (testnum != ALL_TESTS) break;
    case TEST_SEQUENCE: //basic sequence of gets, puts, and deletes
      for (i=0; i<n; i++) { m.get(i); }
      for (i=k-w; i<=k+w; i++) { if(i<n && i>=0) m.put(i,i); }
      if(m.get(k)==-1) err=1;
      m.remove(k);
      for (i=0; i<n; i++) { m.get(i); }
      m.put(k,k);
      for (i=0; i<n; i++) { m.get(i); }
      if(m.get(k)==-1) err=1;
      if (testnum != ALL_TESTS) break;
    case TEST_READ: //simple read-only sequence
      for (i=k-w; i<=k+w; i++) { if(i<n && i>=0) m.get(i) != i; }
      if (testnum != ALL_TESTS) break;
    case TEST_PUT: //sequence of put statements (first put writes 0, second put writes correct value i)
      for (i=k-w; i<=k+w; i++) { if(i<n && i>=0) m.put(i,0); }
      for (i=k-w; i<=k+w; i++) { if(i<n && i>=0) m.put(i,i); }
      if (testnum != ALL_TESTS) break;
    case TEST_DELPUT: //sequence of deletes and puts
      for (i=k-w; i<=k+w; i++)
        if(i<n && i>=0) m.remove(i);
      for (i=k-w; i<=k+w; i++)
        if(i<n && i>=0) m.put(i,i);
      if (testnum != ALL_TESTS) break;
    case TEST_PUTGET: //sequence of puts and gets. set up so there are write conflicts, not not on k, so we should always be the last person to write to key k
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,0);
      err |= m.get(k) != 0;
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,1);
      err |= m.get(k) != 1;
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,i+1);
      err |= m.get(k) != k+1;
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,k+3);
      err |= m.get(k) != k+3;
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,i);
      err |= m.get(k) != k;
      if (testnum != ALL_TESTS) break;
    case TEST_KEYHIT: //sequences of puts/deletes for the same key
      for(i=0;i<100;++i) {
        m.put(n+1,0);
        m.remove(n+1);
        m.put(n+1,1);
        m.remove(n+1);
      }
      if (testnum != ALL_TESTS) break;
    case TEST_RWHIT: //sequence of modifications that should all land in the same bucket (to hit the rwlock harder).
      for(i=0,t=n+1;i<100;++i,t+=TABLE_SIZE) { m.put(t,t); }
      for(i=0,t=n+1;i<100;++i,t+=TABLE_SIZE) { m.remove(t); }
      for(i=0,t=n+1;i<100;++i,t+=TABLE_SIZE) { m.put(t,t); }
      for(i=0,t=n+1;i<100;++i,t+=TABLE_SIZE) { m.remove(t); }
      if (testnum != ALL_TESTS) break;
    case TEST_INC: //sequence of increments. If we didn't miss anything, the final value of key n should be the number of threads * the number of iterations * 100
      for (i=0;i<100;++i) { m.increment(n,1); }
      break;
      //TODO: you can define additional tests you want to run here
    default:
      err=-1;
  }

  return_codes[id] = err;
  iter_barrier.wait(NumThreads+1);
  if (NumThreads > 0) currentThread->Finish();
}

void ThreadTester(int childThreadNum)
{
  DEBUG('t', (char*)"Entering ThreadTest1");
  // Prepare a new thread to be forked
  Thread *t = new Thread((char*)"forked thread");
  // 'Fork' the new thread, putting it on the ready queue, about to run tfunc
  t->Fork(tfunc, childThreadNum);
}


void ThreadTest(void)
{
  if(NumThreads < 0) {
    cout << "Num threads must be >= 0\n";
    //return -1;
    return;
  } else if (NumKeys < 5*NumThreads) {
    cout << "Num threads must be >= 5*num iterations for all tests to pass\n";
    //return -1;
    return;
  } else if (NumIterations < 1) {
    cout << "Num iterations must be >= 1\n";
    //return -1;
    return;
  }

  const char *testname = get_test_name((enum TestNumber)testnum);
  if (testname) {
    cout << "Running Test: '" << testname << "' in " << NumThreads << " threads for " << NumIterations << " iterations with " << NumKeys << " Keys\n";
  } else {
    cout << "Invalid Test Number\n";
    return;
  }

  int err=0;
  int errflag=0;
  int k;
  int val;

  // initialize some keys
  for (int i=0; i<NumKeys; i++)
    m.put(i,i);

  m.put(NumKeys,0); //counter for increment()

  //loop through the test NumIterations times
  for(int iter = 0; iter < NumIterations; ++iter) {
    //loop initialization
    wait_count=0;
    

    // Spawn N child threads
    for(int i = 0; i < NumThreads; i++) {
      ThreadTester(i); // Pass the child thread's id as arg
    }
    //wait for N child threads to finish (the +1 is for us)
    iter_barrier.wait(NumThreads+1);
    //check return code from each thread (they should all be zero)
    for(int i = 0; i < NumThreads; i++) {
      if (return_codes[i]) err++;
    }
    if (errflag) {
      cout << err << " tests returned failure codes for iteration " << iter << endl;
      errflag|=err;
      err=0;
      break; //once we fail, don't keep going (but still print the final key values)
    }

    for (k=0; k<NumKeys; ++k) { //verify key-values
      errflag |= m.get(k) != k;
    }
    if(errflag) {
      cout << "Failed to find  some elements or value was incorrect after iteration " << iter <<endl;
      break;
    }
    errflag |= err;
    err=0;
  }

  //all tests leave every key in a well defined state (get(i) == i) so we can check if anything broke here
  for (k=0; k<NumKeys; ++k) {
    val=m.get(k);
    err |= val != k;
    cout << " " << val;
    if (0==((k+1)%25) || k==(NumKeys-1)) cout << endl;
  }
  if(err)
    cout << "Failed to find  some elements or value was incorrect"<<endl;
  errflag |= err;
  err=0;

  if (testnum == TEST_INC || testnum == ALL_TESTS) {
    const int correct_sum = NumIterations*NumThreads*100;
    val=m.get(NumKeys);
    if (val != correct_sum) {
      errflag |= 1;
      cout << "HashMap increment resulted in wrong value. sum should be " << correct_sum << " but instead got " << val << endl;
    } else {
      cout << "HashMap increment resulted in correct value of " << correct_sum << endl;
    }
  }

  if (testnum == TEST_LOCK || testnum == ALL_TESTS) {
    const int correct_sum = NumIterations*NumThreads;
    if (lock_count != correct_sum) {
      errflag |= 1;
      cout << "Lock did not enforce mutual exclusion. sum should be " << correct_sum << " but instead got " << lock_count << endl;
    } else {
      cout << "Lock increment resulted in correct value of " << correct_sum << endl;
    }
  }

  if (testnum == TEST_RW_WEXLCUDE || testnum == ALL_TESTS) {
    const int correct_sum = NumIterations*NumThreads;
    if (rw_w_count != correct_sum) {
      errflag |= 1;
      cout << "RWLock did not enforce mutual exclusion between writers. sum should be " << correct_sum << " but instead got " << rw_w_count << endl;
    } else {
      cout << "RWLock W increment resulted in correct value of " << correct_sum << endl;
    }
  }
  if (testnum == TEST_RW_RWEXLCUDE || testnum == ALL_TESTS) {
    const int correct_sum = NumIterations*NumThreads*2;
    if (rw_rw_count != correct_sum) {
      errflag |= 1;
      cout << "RWLock did not enforce mutual exclusion between readers and writers. sum should be " << correct_sum << " but instead got " << rw_rw_count << endl;
    } else {
      cout << "RWLock RW increment resulted in correct value of " << correct_sum << endl;
    }
  }
#ifndef P1_RWLOCK
  if (testnum == TEST_RW_MULTIPLE_R || testnum == ALL_TESTS) {
    const int correct_sum = NumIterations*NumThreads;
    if (rw_r_count != correct_sum) {
      errflag |= 1;
      cout << "RWLock did not enforce mutual exclusion between readers and we aren't in P1_RWLOCK. sum should be " << correct_sum << " but instead got " << rw_r_count << endl;
    } else {
      cout << "RWLock R increment resulted in correct value of " << correct_sum << endl;
    }
  }
#endif

  if (!errflag) //if no error flags were set
    cout << "All tests passed. This doesn't mean everything is correct, just that no bugs were uncovered during automated testing"<<endl;
  else
    cout << "One or more tests failed. See output above for hints"<<endl;
  //return err;
}
#endif