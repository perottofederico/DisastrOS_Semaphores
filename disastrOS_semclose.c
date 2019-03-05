#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){

	//Get fd of sem to close
	int fd = running->syscall_Args[0];

	//Find the sem desriptor using said fd
	SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	if(!sem_desc){
		running->syscall_retvalue = DSOS_ESEMDESC_NOT_IN_PROCESS; //New error code
		return;
	}

	//Remove sem_desc from the process list of sem descriptors
	sem_desc = (SemDescriptor*) List_detach(&running->sem_descriptors, (ListItem*)sem_desc);
	assert(sem_desc);

	//Get the semaphore associated to the descriptor
	Semaphore* sem = sem_desc->semaphore;

	//Remove the descriptor pointer from the list
	SemDescriptorPtr* sem_descptr = (SemDescriptorPtr*) List_detach(&sem->descriptors, (ListItem*) sem_desc->ptr);
	assert(sem_descptr);

	//Free the descriptor and its pointer
	SemDescriptor_free(sem_desc);
	SemDescriptorPtr_free(sem_descptr);

	//Make sure the semaphore isn't in use somewhere
	if(sem->descriptors.size){
		running->syscall_retvalue = DSOS_ESEMINUSE; //New error code
		return;
	}

	//Free the semaphore
	sem = (Semaphore*) List_detach(&semaphores_list, (ListItem*) sem);
	assert(sem);
	Semaphore_free(sem);

	running->syscall_retvalue = 0;
}
