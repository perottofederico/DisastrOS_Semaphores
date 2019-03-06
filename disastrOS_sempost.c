#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){

	//Get fd 
	int fd = running->syscall_args[0];

	//Check if the descriptors belongs to the running process
	SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	if(!sem_desc){
		running->syscall_retvalue = DSOS_ESEMDESC_NOT_IN_PROCESS;
		return;
	}

	//Get the semaphore
	Semaphore* sem = sem_desc->semaphore;
	//Increase sem value
	sem->count++;

	if(sem->count < 0){
		//Insert running process in list of ready processes
		List_insert(&ready_list, ready_list.last, (ListItem*) running);
		
		//Move the descriptor (its pointer) from the list of waiting descriptors to in use
		SemDescriptorPtr* sem_descptr = (SemDescriptorPtr*) List_detach(&sem->waiting_descriptors, 
			(ListItem*) sem->waiting_descriptors.first);
		List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*)sem_descptr);

		//Remove a process from the waiting list
		List_detach(&waiting_list, (ListItem*) sem_descptr->descriptor->pcb);

		//Change running process' state
		running->status = Ready;
		//Set new process as running
		running = sem_descptr->descriptor->pcb;
	}

	//return 0 if succesfull
	running->syscall_retvalue = 0;
	return;
}
