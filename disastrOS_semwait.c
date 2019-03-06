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
	//Decrease sem value
	sem->count--;

	//If the value is less than 0
	if(sem->count < 0){
		//Move the descriptor (its pointer) from the list of descriptors ready to waiting
		List_detach(&sem->sem_descriptors, (ListItem*)sem_desc->ptr);
		List_insert(&sem->waiting_descritpors, sem->waiting_descritpors.last, (ListItem*)sem_desc->ptr);

		//Change the process status and insert it in the list of waiting processes
		running->status = waiting;
		List_insert(&waiting_list, waiting_list.last, (ListItem*) running);

		//Get a new process from the ready list and put it in running
		PCB* pcb = (PCB*)List_detach(&ready_list, (ListItem*)ready_list.first);
		running = pcb;
	}

	//If successfull return 0
	running->syscall_retvalue = 0;
	return;
}
