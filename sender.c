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
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "buff.h"

int main(int argc, char *argv[]){
	int opt;
	char *options = "m:";
	char *error;

	size_t len;

	int result = 0;
	char input[5000];
	char *ch;

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

	sem_t *writeSem = sem_open(WRITESEM, O_EXCL|O_CREAT, 0600, len);
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

	//Create the sharded memory
	int shm = shm_open(SHMNAME, O_CREAT | O_EXCL| O_RDWR, 0600);
	if(shm == -1){
		perror("Create Shared Memory");
		sem_close(writeSem);
		sem_unlink(WRITESEM);
		sem_close(readSem);
		sem_unlink(READSEM);
		exit(1);
	}

	//Change the size of the shared memory
	while(ftruncate(shm, bufferSize) == -1){
		//continues if EINTR was the cause of the error
		if(errno != EINTR){
			perror("ftruncate");
			close(shm);
			shm_unlink(SHMNAME);
			sem_close(writeSem);
			sem_unlink(WRITESEM);
			sem_close(readSem);
			sem_unlink(READSEM);
			exit(1);
		}
	}

	BUFFER *sharedMem = mmap(NULL, bufferSize, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
	if(sharedMem == MAP_FAILED){
		perror("mmap");
		close(shm);
		shm_unlink(SHMNAME);
		sem_close(writeSem);
		sem_unlink(WRITESEM);
		sem_close(readSem);
		sem_unlink(READSEM);
		exit(1);
	}


	sharedMem->rIndex = 0;
	sharedMem->wIndex = 0;
	sem_post(readSem);

	printf("Start writing\n");

	while(1){
		result = scanf("%[^\n]%*c", input);

		if(result == EOF){
			while(sem_wait(writeSem) == -1){
				if(errno != EINTR){
					perror("Sem_wait");
					close(shm);
					shm_unlink(SHMNAME);
					sem_close(writeSem);
					sem_unlink(WRITESEM);
					sem_close(readSem);
					sem_unlink(READSEM);
					exit(1);
				}
			}
			sharedMem->data[sharedMem->wIndex] = EOF;
			sharedMem->wIndex++;
			sem_post(readSem);
			break;
		}

		for(ch = input; *ch != '\0'; ch++){
			while(sem_wait(writeSem) == -1){
				if(errno != EINTR){
					perror("Sem_wait");
					close(shm);
					shm_unlink(SHMNAME);
					sem_close(writeSem);
					sem_unlink(WRITESEM);
					sem_close(readSem);
					sem_unlink(READSEM);
					exit(1);
				}
			}
			//printf("%c %ld\n", *ch, writeSem->__align);
			sharedMem->data[sharedMem->wIndex] = *ch;
			sharedMem->wIndex = (sharedMem->wIndex + 1) % len;
			sem_post(readSem);
		}

		*ch = '\n';
		while(sem_wait(writeSem) == -1){
			if(errno != EINTR){
				perror("Sem_wait");
				close(shm);
				shm_unlink(SHMNAME);
				sem_close(writeSem);
				sem_unlink(WRITESEM);
				sem_close(readSem);
				sem_unlink(READSEM);
				exit(1);
			}
		}
		sharedMem->data[sharedMem->wIndex] = *ch;
		sharedMem->wIndex = (sharedMem->wIndex + 1) % len;
		sem_post(readSem);

	}

	//Release Semaphoore and Shared Memory
	if(munmap(sharedMem, bufferSize) != 0){
		perror("munmap");
		close(shm);
		shm_unlink(SHMNAME);
		sem_close(writeSem);
		sem_unlink(WRITESEM);
		sem_close(readSem);
		sem_unlink(READSEM);
		exit(1);
	}


	close(shm);
	sem_close(writeSem);
	sem_close(readSem);
}
