#ifndef RAILROAD_COMMUNICATION_H
#define RAILROAD_COMMUNCIATION_H

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>

#define BIT_1_TIME 58000  /* 58 microseconds*/
#define BIT_0_TIME 100000 /* 100 microsecdons*/
#define LPT1 0x378 /*Pin of parallelport*/

SEM bit_mutex;

RT_TASK loco_tasks[3];
RT_TASK magnetic_task;

uint64_t message = 0xFFFC066230C00000; // 0x5555555555555555;
int length = 42;
int locomotive_count = 3;
int magnetic_msg_count = 0;
LocomotiveData locomotive_msg_queue[locomotive_count] = {}; // 0b111111111111110000000110011111110011111001<<22 | {.address = 00000011, .light = 1, .direction = 1, .speed = 110}
MagneticData magnetic_msg_queue[4] = {}


static void send_bit_task(uint64_t message, int length);

static void send_magnetic_msg_task(long arg);

static void send_loco_msg_task(long i)


#endif