/*
 * receiver.c
 *
 *  Created on: Jun 10, 2021
 *      Author: osboxes
 */


#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "buff.h"

int main(int argc, char **argv) {
	size_t len = 20;
	size_t bufferSize = sizeof(BUFFER) + len*sizeof(char);

	sem_t *writeSem = sem_open(WRITESEM, 0);
	if(writeSem == SEM_FAILED){
		exit(1);
	}

	sem_t *readSem = sem_open(READSEM, 0);
	if(readSem == SEM_FAILED){
		sem_close(writeSem);
		exit(1);
	}

	sem_wait(readSem);

	int shm = shm_open(SHMNAME, O_RDWR, 0600);
	if(shm == -1){
		perror("Open Shared Memory");
		sem_close(writeSem);
		sem_close(readSem);
		exit(1);
	}


	BUFFER *sharedMem = mmap(NULL, bufferSize, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
	if(sharedMem == MAP_FAILED){
		perror("mmap");
		close(shm);
		sem_close(writeSem);
		sem_close(readSem);
		exit(1);
	}

	printf("Start reading\n");
	sem_post(writeSem);

	//Reading part of the program
	sem_wait(readSem);
	printf("%c", sharedMem->data[sharedMem->rIndex]);
	sharedMem->rIndex++;
	sem_post(writeSem);

	//Release Semaphoore and Shared Memory
	if(munmap(sharedMem, bufferSize) != 0){
		perror("munmap");
		close(shm);
		sem_close(writeSem);
		sem_close(readSem);
		exit(1);
	}

	close(shm);
	sem_close(readSem);
	sem_close(writeSem);
}

