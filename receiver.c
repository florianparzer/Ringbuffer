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
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include "buff.h"
#include <errno.h>

int main(int argc, char **argv) {
	int opt;
	char *options = "m:";
	char *error;

	size_t len;

	if(argc <=2){
		fprintf(stderr, "Option m is missing\n");
		return 1;
	}

	while((opt = getopt(argc, argv, options)) != -1){
		switch(opt){
			case 'm':
				len = strtol(optarg, &error, 10);
				if(*optarg == *error){
					fprintf(stderr, "Please enter a valid number\n");
					return -1;
				}
				if(len >= UINT_MAX){
					fprintf(stderr, "Entered number is too large\n");
					return -1;
				}
				break;
			default:
				printf("Unidentified Option\n");
				return 0;
		}
	}

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
	char ch;
	while(1){
		while(sem_wait(readSem) == -1){
			if(errno != EINTR){
				perror("sem_wait");
				close(shm);
				shm_unlink(SHMNAME);
				sem_close(writeSem);
				sem_unlink(WRITESEM);
				sem_close(readSem);
				sem_unlink(READSEM);
				exit(1);
			}
		}
		ch = sharedMem->data[sharedMem->rIndex];
		if(ch == EOF){
			break;
		}
		printf("%c", ch);
		fflush(stdout);
		sharedMem->rIndex = (sharedMem->rIndex + 1)% len;
		sem_post(writeSem);
	}

	//Release Semaphoore and Shared Memory
	if(munmap(sharedMem, bufferSize) != 0){
		perror("munmap");
		close(shm);
		sem_close(writeSem);
		sem_close(readSem);
		exit(1);
	}

	close(shm);
	shm_unlink(SHMNAME);
	sem_close(readSem);
	sem_unlink(WRITESEM);
	sem_close(readSem);
	sem_unlink(READSEM);
}

