#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"


void internal_semOpen(){
	//Get semaphore id and value from "registers"
	int sem_num = running->syscall_args[0];
	int sem_val = running->syscall_args[1];

	//Check if a semaphore with the same id already exists
	Semaphore* sem = SemaphoreList_byId(&semaphores_list, sem_num); //Need to add sempahore list to globals

	if(!sem){
		//If it is not found, create it
		printf("[Seamphore not in list. Creating...]\n")
		sem = Semaphore_alloc(sem_num, sem_val);
		if(!sem){
			printf("[Error in creating Semaphore]\n");
			running->syscall_retvalue = DSOS_ESEMOPEN; //New error code
			return;
		}
		printf("[Semaphore created]\n");
		//Insert new semaphore in semaphore list
		List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);
	}

	//Create the descriptor for the semaphore to add to the running pcb
	SemDescriptor* sem_desc = SemDescriptor_alloc(running->last_sem_fd, sem, running);
	if(!sem_desc){
		running->syscall_retvalue = DSOS_ESEMDESCCREATE; //New error code
		return;
	}
	printf("[Semaphore descriptor created]\n");
	//Increase the sem fd value for the next call
	(running->last_sem_fd)++;

	//Create a pointer to the descriptor above
	SemDescriptorPtr*  sem_descptr = SemDescriptorPtr_alloc(sem_desc);
	printf("[Pointer to the semaphore descriptor created\n]");
	sem_desc->ptr = sem_descptr;
	//Add the decriptor pointer to the descriptors list in the semaphore (sem->descriptors)
	List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) sem_descptr);
	
	//Add the descriptor to the list of semaphore descriptors of the process
	List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*)sem_desc); 

	//Return the fd of the semaphore 
	running->syscall_retvalue = sem_desc->fd;
	return;
}




