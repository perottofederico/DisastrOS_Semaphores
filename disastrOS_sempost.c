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
	//Increase sem 
	sem->count++;
	printf("\n>>>>>>>Semaphore #%d has been incremented to %d by process %d\n\n", sem->id, sem->count, running->pid);

	if(sem->count <= 0){ //MOVE A PROCESS FROM WAITING TO READY
		
		SemDescriptorPtr* sem_descptr = (SemDescriptorPtr*)List_detach(&sem->waiting_descriptors,
			sem->waiting_descriptors.first);
		List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*)sem_descptr);

		PCB* pcb_to_wake = sem_descptr->descriptor->pcb;
		pcb_to_wake->status = Ready;

		List_detach((ListHead*)&waiting_list, (ListItem*)pcb_to_wake);
		List_insert((ListHead*)&ready_list, (ListItem*)ready_list.last, (ListItem*)pcb_to_wake);
		printf("Process #%d has been move to ready queue\n", pcb_to_wake->pid);

		disastrOS_printStatus();
	}

	//return 0 if succesfull
	running->syscall_retvalue = 0;
	return;
}
