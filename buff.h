/*
 * buff.h
 *
 *  Created on: Jun 11, 2021
 *      Author: osboxes
 */

#ifndef BUFF_H_
#define BUFF_H_

#define READSEM "/readingSem"
#define WRITESEM "/writingSem"
#define SHMNAME "/ringbuffer"

typedef struct buffer{
	size_t len;
	size_t rIndex;
	size_t wIndex;
	char data[];
} BUFFER;

#endif /* BUFF_H_ */
