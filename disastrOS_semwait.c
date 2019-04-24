#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){

	//Get fd
	int fd = running->syscall_args[0];

	//Check if the descriptor belongs to the running process
	SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	if(!sem_desc){
		running->syscall_retvalue = DSOS_ESEMDESC_NOT_IN_PROCESS;
		return;
	}

	//Get the semaphore associated with the descriptor
	Semaphore* sem = sem_desc->semaphore;

	//Store calling process to give it a return value
	//PCB* caller = running;

	//Decrease sem value
	sem->count--;
	printf("\nSemaphore #%d has been decreased to %d by process %d\n\n", sem->id, sem->count, running->pid);


	if(sem->count < 0){ //move tha calling process from ready to waiting queue

		//Move the descriptor (its pointer) from the list of descriptors ready to waiting
		List_detach(&sem->descriptors, (ListItem*)sem_desc->ptr);
		List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*)sem_desc->ptr);

		//Change the process status and insert it in the list of waiting processes
		running->status = Waiting;
		List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
		printf("\nProcess #%d has been moved to the waiting queue\n", running->pid);

		//Get a new process from the ready list and put it in running
		PCB* pcb = (PCB*)List_detach(&ready_list, (ListItem*)ready_list.first);
		running = pcb;
		printf("\nProcess #%d has been moved from the ready queue to running \n\n", pcb->pid);
	}

	//disastrOS_printStatus();

	//If successfull return 0
	//caller->syscall_retvalue = 0;
	running->syscall_retvalue=0;
	return;
}
