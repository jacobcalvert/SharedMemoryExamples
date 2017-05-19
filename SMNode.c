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

uint32_t IAM;

bool _read(uint8_t *buffer, uint8_t *_len)
{
	while(!IS_RDY());
	if(!IS_BUSY())
	{
		if(shm[DREG] == IAM)
		{
			SET_BUSY();
			uint8_t len = shm[SREG] >> 2;
			uint8_t i = 0;
			*_len = len;
			while(i < len)
			{
				buffer[i] = shm[DREG + i + 1];
				++i;
			}
			CLR_RDY();
			CLR_BUSY();
			return true;
		}
		return false;

	}
	return false;
}

void _write(uint8_t who, uint8_t *buffer, uint8_t len)
{

	while(IS_BUSY() || IS_RDY())
	{
		usleep(10000);
		printf(".");
	}
		SET_BUSY();
		SET_ADDR(who);
		SET_LEN(len);
		uint8_t i = 0;

		while(i < len)
		{
			shm[DREG + i + 1] = buffer[i];
			++i;
		}
		SET_RDY();
		CLR_BUSY();
		printf("\r\n");

}


int main(int argc, char **argv)
{
	int shmid;
	key_t key;


	printf("You are? ");
	scanf("%u", &IAM);
	printf("\r\nYou are 0x%02x\r\n", IAM);

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
	uint8_t t = 0;
	while(t < SMSZ)
	{
		shm[t++] = 0;
	}
	char rw;
	while(1)
	{
		printf("Read or Write? ");
		scanf(" %c", &rw);
		printf("\r\nyou chose to %s\r\n", (rw == 'w')?"write":"read");
		while(rw == 'r')
		{
			uint8_t buffer[64];
			uint8_t len;
			if(_read(buffer, &len))
			{
				printf("we got a packet ]> ");
				uint8_t i = 0;
				while(i < len)
				{
					printf("0x%02x ", buffer[i]);
					i++;
				}
				printf("\r\n");
			}
		}
		while(rw == 'w')
		{
			uint8_t buffer[64], i =0;
			uint32_t who, len;
			printf("to who: ");
			scanf("%u", &who);
			printf("\r\nto 0x%02x\r\n",who);
			printf("enter the length: ");
			scanf("%u", &len);
			printf("\r\nenter %u numbers\r\n", len);
			while(i < len)
			{
				printf("enter a number: ");
				scanf("%hhu", &buffer[i]);
				i++;
			}
			_write(who, buffer, len);


		}
	}

	return 0;
}
