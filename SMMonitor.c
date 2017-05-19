/**
 * @file main.c
 * @date May 18, 2017
 * @author Jacob Calvert <jcalvert@jacobncalvert.com>
 * @brief
 *
 */


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#define SMSZ     65

#define SREG	0
#define DREG 	1

#define BSY			(1<<0)
#define RDY			(1<<1)
#define LEN_MASK	0xFC;

uint8_t *shm;

#define IS_BUSY()	(shm[SREG] & BSY)
#define IS_RDY()	(shm[SREG] & RDY)
#define SET_BUSY()	(shm[SREG] |= BSY)
#define SET_RDY()	(shm[SREG] |= RDY)
#define CLR_BUSY()	(shm[SREG] &= ~BSY)
#define CLR_RDY()	(shm[SREG] &= ~RDY)

#define SET_ADDR(addr)	(shm[DREG] = addr)
#define SET_LEN(len)	(shm[SREG] |= (len<<2))

int main(int argc, char **argv)
{
	int shmid;
	key_t key;


	/*
	 * We'll name our shared memory segment
	 * "5678".
	 */
	key = 5678;

	/*
	 * Create the segment.
	 */
	if ((shmid = shmget(key, SMSZ, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
	}

	/*
	 * Now we attach the segment to our data space.
	 */
	if ((shm = shmat(shmid, NULL, 0)) == (uint8_t *) 0xFF) {
		perror("shmat");
	}
	uint8_t cache = 0;
	uint8_t rdycnt = 0;
	while(1)
	{
		usleep(1000);
		if(IS_RDY())
		{
			rdycnt++;

		}
		if(rdycnt > 254)
		{
			printf("INFO: apparent bus hang, clearing bus\r\n");
			shm[SREG] = 0;
			rdycnt = 0;
		}
		if(shm[SREG] != cache)
		{
			printf("SREG: Busy (%c) | Ready (%c)\r\n", IS_BUSY()?'*':' ', IS_RDY()?'*':' ');
			cache = shm[SREG];
			rdycnt = 0;

		}


	}

	return 0;
}
