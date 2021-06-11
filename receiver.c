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
	sem_t *writeSem = sem_open(WRITESEM, 0);
	if(writeSem == SEM_FAILED){
		exit(1);
	}

	sem_t *readSem = sem_open(READSEM, 0);
	if(readSem == SEM_FAILED){
		exit(1);
	}


	sem_wait(readSem);
	printf("read\n");
	sleep(5);
	sem_post(writeSem);

	sem_close(readSem);
	sem_close(writeSem);
}

