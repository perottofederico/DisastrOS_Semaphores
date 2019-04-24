#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){

	//Get fd of sem to close
	int fd = running->syscall_args[0];

	//Find the sem desriptor using said fd
	SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	if(!sem_desc){
		running->syscall_retvalue = DSOS_ESEMDESC_NOT_IN_PROCESS; //New error code
		return;
	}

	//Get the semaphore associated to the descriptor
	Semaphore* sem = sem_desc->semaphore;

	//Remove sem_desc from the process' list of sem descriptors
	sem_desc = (SemDescriptor*) List_detach(&running->sem_descriptors, (ListItem*)sem_desc);
	assert(sem_desc);

	//Remove the descriptors of the semaphores from the lists
	SemDescriptorPtr* sem_descptr = (SemDescriptorPtr*) List_detach(&sem->descriptors, (ListItem*) sem_desc->ptr);
	assert(sem_descptr);

	//Free the descriptor and its pointer
	SemDescriptorPtr_free(sem_descptr);
	SemDescriptor_free(sem_desc);
	assert(sem_descptr || sem_desc);

	//Make sure the semaphore isn't in use somewhere
	if(sem->descriptors.size == 0 && sem->waiting_descriptors.size == 0){
		printf("Semaphore %d will be deleted\n", sem->id);
		//Free the semaphore
		sem = (Semaphore*) List_detach(&semaphores_list, (ListItem*) sem);
		assert(sem);
		Semaphore_free(sem);
	}

	//Return 0 if succesfull
	running->syscall_retvalue = 0;
	return;
}
