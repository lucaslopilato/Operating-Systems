/**
 *Test program for concurrent hashtable
 **/

#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <cstdlib>
#include <string>
//#include <cstdio>
using namespace std;


#ifdef HASHCHAIN
#include "hashchain.h"
#else
#include "rwlock.h"
#include "phash.h"
#endif



#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
       return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}

//misc. workload
int test1(HashMap *htable, int n, int k, int w){
  int i, errorflag=0;
  
  for (i=0; i<n; i++)
    (*htable).get(i);
  for (i=k-w; i<k+w; i++)
    if(i<n && i>=0) (*htable).put(i,i);
  if((*htable).get(k)==-1)
      errorflag=1;
  (*htable).remove(k);
  for (i=0; i<n; i++)
    (*htable).get(i);
  (*htable).put(k,k);
  for (i=0; i<n; i++)
    (*htable).get(i);
  if((*htable).get(k)==-1)
      errorflag=1;
  if(errorflag==1)
    cout << "Failed to find  some elements"<<endl;
  return errorflag;
}

//readonly workload
int test_get(HashMap *htable, int n, int k, int w){
  int i, err=0;
  for (i=k-w; i<k+w; i++)
    if(i<n && i>=0) err |= (*htable).get(i) != i;

  if(err)
    cout << "Failed to find  some elements or value was incorrect"<<endl;
  return err;
}
//
//put-only workload
int test_put(HashMap *htable, int n, int k, int w){
  int i;
  for (i=k-w; i<k+w; i++)
    if(i<n && i>=0) (*htable).put(i,0);
  for (i=k-w; i<k+w; i++)
    if(i<n && i>=0) (*htable).put(i,i);
  return 0;
}

//put-get workload -- note, I inserted extra "remove" because there is a bug in hashchain
int test_put_get(HashMap *htable, int n, int k, int w){
  int i, err=0;
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).remove(i);
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).put(i,0);
  err |= (*htable).get(k) != 0;
  if (err) cout << "get(k) after put("<<k<<",0)) = " << (*htable).get(k) << endl;
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).remove(i);
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).put(i,1);
  err |= (*htable).get(k) != 1;
  if (err) cout << "get(k) after put("<<k<<",1)) = " << (*htable).get(k) << endl;
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).remove(i);
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).put(i,i+1);
  err |= (*htable).get(k) != k+1;
  if (err) cout << "get(k) after put("<<k<<",i+1)) = " << (*htable).get(k) << endl;
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).remove(i);
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).put(i,k+3);
  err |= (*htable).get(k) != k+3;
  if (err) cout << "get(k) after put("<<k<<",k+3)) = " << (*htable).get(k) << endl;
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).remove(i);
  for (i=k-w; i<k+w+1; i++)
    if(i>=0) (*htable).put(i,i);
  err |= (*htable).get(k) != k;
  if (err) cout << "get(k) after put("<<k<<",i)) = " << (*htable).get(k) << endl;
  if(err)
    cout << "Failed to find  some elements or value was incorrect"<<endl;
  return err;
}

//do delete then put sequence around k
int test_del_put(HashMap *htable, int n, int k, int w){
  int i;
  for (i=k-w; i<k+w; i++)
    if(i<n && i>=0) (*htable).remove(i);
  for (i=k-w; i<k+w; i++)
    if(i<n && i>=0) (*htable).put(i,i);
  return 0;
}

HashMap hash;
int NumKeys=100;
int NumberofThread = 1;
int NumIterations=1;
int testNumber = 0;

void *tfunc1(void *arg){
  long k= (long) arg; //thread ID
  int b=NumKeys/NumberofThread;
  int i;
  if( NumKeys % NumberofThread!=0)
    b=b+1;
  // cout << "Thread " <<i;
  for (i= b *k; i< b*k+b && i<NumKeys; i++){
     //cout << "Thread " <<k <<" does test1 "<<i<<endl;
    // printf(" Thread %d does test1 %d\n",  k, i);
    if(0 != test1(&hash, NumKeys, i,2)) return (void*)-1;
  }
  return 0;
} 

void *tfunc2(void *arg){
  long k= (long) arg; //thread ID
  int center_key=(k*NumKeys)/NumberofThread;
  int i;
  for (i=0; i<10; i++){
    if (0 != test_get(&hash, NumKeys, center_key, 10)) return (void*)-1;
  }
  return 0;
} 
  
void *tfunc3(void *arg){
  long k= (long) arg; //thread ID
  int center_key=(k*NumKeys)/NumberofThread;
  int i;
  for (i=0; i<10; ++i) {
    if (0 != test_put(&hash, NumKeys, center_key,10)) return (void*)-1;
  }
  return 0;
} 

void *tfunc4(void *arg){
  long k= (long) arg; //thread ID
  int center_key=(k*NumKeys)/NumberofThread;
  int i;
  for (i=0; i<10; ++i) {
    if (0 != test_del_put(&hash, NumKeys, center_key,10)) return (void*)-1;
  }
  return 0;
} 

void *tfunc5(void *arg){
  long k= (long) arg; //thread ID
  int center_key=k*5;
  int i;
  for (i=0; i<10; ++i) {
    if (0 != test_put_get(&hash, NumKeys, center_key,3)) return (void*)-1;
  }
  return 0;
} 

void test(){
  long i, errorflag=0;
 
  pthread_t *thr = (pthread_t *)malloc(sizeof(pthread_t) * NumberofThread); 
  //  Start Timers
  double wall0 = get_wall_time();
  double cpu0  = get_cpu_time();

  void* (*tfunc)(void *);
  for (i=0; i<NumKeys; i++)
    hash.put(i,i);

  switch(testNumber) {
    case 1: tfunc = tfunc1; break;
    case 2: tfunc = tfunc2; break;
    case 3: tfunc = tfunc3; break;
    case 4: tfunc = tfunc4; break;
    case 5: tfunc = tfunc5; break;
    default: tfunc = NULL; break;
  }

  if (!tfunc) {
    printf("Invalid testNumber\n");
    exit(1);
  }

  cout << "Running tests in " << NumberofThread << " threads" << endl;

  int err=0;
  void* retval = 0;
  int j;
  for(j=0; j<NumIterations && !err; ++j) {
    if(NumberofThread>1){
      for(i=0;i<NumberofThread;i++) 
        pthread_create(&thr[i],NULL, tfunc, (void *)  i);
      for(i=0;i<NumberofThread;i++) {
        pthread_join(thr[i],&retval);
        err |= (0 != retval);
      }
      cout << "Finished iteration " << j << endl;
    } else {
      i=0;
      retval = tfunc ((void *)  i);
    } 
  }
  
  if(retval)
    cout << "One or more tests failed"<<endl;

  for (i=0; i<NumKeys; ++i) {
    int val=hash.get(i);
    err |= val != i;
    cout << " " << val;
    if (0 == ((i+1)%10)) cout << endl;
  }
  if (i%10) cout << endl;
  if(err)
    cout << "Failed to find  some elements or value was incorrect"<<endl;

    
}


int main(int argc, char*argv[])
{
    int key, value;
    int choice;
    if(argc != 5) {
      printf("\n Usage eg ./phashcoarse <number of threads> <number of keys> <testnumber> <num iterations>\n ");
      return 1;

    }  

    NumberofThread = atoi(argv[1]);
    NumKeys = atoi(argv[2]);
    testNumber = atoi(argv[3]);
    NumIterations = atoi(argv[4]);

    if (NumKeys < 1 || testNumber < 1) {
      printf("Invalid test number or number of keys\n");
      printf("\n Usage eg ./phashcoarse <number of threads> <number of keys> <testnumber>\n ");
      return 1;
    }
    test();
/*
    while (1)
    {
        cout<<"1. Insert element into the table"<<endl;
        cout<<"2. Search element from the key"<<endl;
        cout<<"3. Delete element at a key"<<endl;
        cout<<"4. Repeat performance test"<<endl;
        cout<<"5. Exit"<<endl;
        cout<<"Enter your choice: ";
        cin>>choice;
  if(cin.fail()){
    cin.clear();
          cout<<"Invalid choice "<<endl;
     cin.ignore(80, '\n');
  } else {
        switch(choice)
        {
        case 1:
            cout<<"Enter element to be inserted: ";
            cin>>value;
            cout<<"Enter key at which element to be inserted: ";
            cin>>key;
            hash.put(key, value);
            break;
        case 2:
            cout<<"Enter key of the element to be searched: ";
            cin>>key;
            cout<<"Element at key "<<key<<" : ";
            value= hash.get(key); 
            if (value == -1)
            {
          cout<<"No element found at key "<<key<<endl;
      } else
          cout<<value<<endl;
            break;
        case 3:
            cout<<"Enter key of the element to be deleted: ";
            cin>>key;
            hash.remove(key);
            break;
        case 4:
          test();
            break;
        case 5:
            //exit(1);
            return 0;
        default:
           cout<<"\nEnter correct option\n";
       }}
    }
    */
    return 0;
}
