/*
 * sender.c
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
#include <errno.h>
#include "buff.h"

int main(int arc, char **argv){
	size_t len = 20;
	size_t bufferSize = sizeof(BUFFER) + len*sizeof(char);

	sem_t *writeSem = sem_open(WRITESEM, O_EXCL|O_CREAT, 0600, 0);
	if(writeSem == SEM_FAILED){
		perror("Create WRITESEM");
		exit(1);
	}

	sem_t *readSem = sem_open(READSEM, O_EXCL|O_CREAT, 0600, 0);
	if(readSem == SEM_FAILED){
		perror("Create READSEM");
		sem_close(writeSem);
		sem_unlink(WRITESEM);
		exit(1);
	}

	printf("before sleep\n");
	sleep(10);
	sem_post(readSem);
	printf("after post\n");
	sem_wait(writeSem);
	printf("after wait\n");

	sem_close(writeSem);
	sem_unlink(WRITESEM);
	sem_close(readSem);
	sem_unlink(READSEM);
}
