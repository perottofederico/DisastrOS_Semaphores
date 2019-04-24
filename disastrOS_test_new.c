#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include "disastrOS.h"
#include "disastrOS_globals.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#define BUFFER_SIZE 16 //With a smaller buffer (eg: 5) we can see the program behaviour when the buffer is full
#define CYCLES 10

int buffer[BUFFER_SIZE]={0};
int in_idx=0;
int out_idx=0;

void printBuffer(int* b, int pid){
  printf("\n[Process #%d] Buffer Status: [", pid);
  for(int i=0; i<BUFFER_SIZE; i++)
    printf(" %d",b[i]);
  printf(" ]\n");
}

void producer(int* buffer, int fd_mutex, int fd_items, int fd_free){
  
  for(int i=0; i<CYCLES; i++){
    disastrOS_semWait(fd_free);
    disastrOS_semWait(fd_mutex);

    buffer[in_idx] = i+1;
    in_idx = (in_idx+1)%BUFFER_SIZE;
    disastrOS_sleep(disastrOS_getpid()); //Need this to let other processes do their thing before the semPost


    disastrOS_semPost(fd_mutex);
    disastrOS_semPost(fd_items);

    printBuffer(buffer, disastrOS_getpid());

  }
}

void consumer(int* buffer, int fd_mutex, int fd_items, int fd_free){

  for(int i=0; i<CYCLES; i++){
    disastrOS_semWait(fd_items);
    disastrOS_semWait(fd_mutex);

    buffer[out_idx] = 0;  //Let's pretend this "consumes" the item
    out_idx = (out_idx+1)%BUFFER_SIZE;
    disastrOS_sleep(disastrOS_getpid());

    disastrOS_semPost(fd_mutex);
    disastrOS_semPost(fd_free);

    printBuffer(buffer, disastrOS_getpid());

  }
}

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  printf("PID: %d, terminating\n", disastrOS_getpid());

  int mutex = disastrOS_semOpen(1, 1); //Mutex
  int items = disastrOS_semOpen(2, 0); //Items in buffer
  int free = disastrOS_semOpen(3, BUFFER_SIZE); //Free spaces

  
  disastrOS_sleep(20-disastrOS_getpid());
  //printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);

  if (disastrOS_getpid()%2==0){
    printf("Process #%d has been assigned producer function\n", running->pid);
    producer(buffer, mutex, items, free);
  }
  else{
    printf("Process #%d has been assigned consumer function\n", running->pid);
    consumer(buffer, mutex, items, free);
  }

  disastrOS_semClose(mutex);
  disastrOS_semClose(items);
  disastrOS_semClose(free);

  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<10; ++i) {
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  printf("shutdown!\n");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
