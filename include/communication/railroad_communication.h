#ifndef RAILROAD_COMMUNICATION_H
#define RAILROAD_COMMUNICATION_H

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>
#include "types.h"

#include "telegram/locomotive.h"
#include "telegram/magnetic.h"
#define BIT_1_TIME 58000  /* 58 microseconds*/
#define BIT_0_TIME 100000 /* 100 microsecdons*/
#define LPT1 0x378        /*Pin of parallelport*/
#define LOC_MSQ_SIZE 3
#define MAG_MSQ_SIZE 4

static SEM bit_mutex;
static SEM loc_sem[LOC_MSQ_SIZE];
static SEM mag_sem[MAG_MSQ_SIZE];

RT_TASK loco_tasks[LOC_MSQ_SIZE];
RT_TASK* magnetic_task = NULL;

unsigned long long message = 0xFFFC066230C00000; // 0x5555555555555555;
int length = 42;
const int locomotive_count = 3;
int magnetic_msg_count = 0;
LocomotiveData locomotive_msg_queue[LOC_MSQ_SIZE] = {}; // 0b111111111111110000000110011111110011111001<<22 | {.address = 00000011, .light = 1, .direction = 1, .speed = 110}
MagneticData magnetic_msg_queue[MAG_MSQ_SIZE] = {};
static int loco_sent_count = 0; // Zählt gesendete Lokomotiv-Telegramme

static void send_bit_task(unsigned long long message, int length);

static void send_magnetic_msg_task(long arg);

static void send_loco_msg_task(long i);

#endif