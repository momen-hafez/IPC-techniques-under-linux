#ifndef __LOCAL_H__
#define __LOCAL_H__

/* Coloring Codes */

#define RED    "\033[0;31m"
#define GREEN  "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE   "\033[0;34m"
#define WHITE  "\033[0;37m"
#define CYAN  "\x1b[36m"

/* Required includes */
#include <semaphore.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include<time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "semaphore.h"
/* variables */

#define NUMBER_MESSAGE_QUEUES 2
#define TOTAL_NUMBER_OF_PASSENGERS 500
#define NUMPER_PASSENGERS 40
#define NUMBER_BUSSES 3
#define BUS_CAPACITY 5
#define NUMBER_OFFICERS 10
#define NUMBER_CROSSING_POINTS_P 5
#define NUMBER_CROSSING_POINTS_J 3
#define NUMBER_CROSSING_POINTS_F 2
#define PERCENTAGE_P 0.5
#define PERCENTAGE_J 0.4
#define PERCENTAGE_F 0.1
#define BUS_TIME_PERIOD 5
#define MAX_THRESHOLD 10
#define MIN_THRESHOLD 9
#define NUMBER_GRANTED 35
#define NUMBER_DENIED  20
#define NUMBER_IMPATIENT  20
#define IMPATIENCE_TIME 60 // time -> 60 seconds


struct PASSENGER_DATA{
  int id;
  int nationality;
  int wait_time;
  int passport_v;
  char expired_date[15];
  int pid;	
  int is_done;
};
struct BUS{
  int bus_id;
  int waiting_time;
  int is_available;
  int number_of_travels;
};
struct HALL {
  struct PASSENGER *P;
};
struct PASSENGER *head ;

struct PASSENGER {
  struct PASSENGER_DATA data;
  struct PASSENGER *next;
};



typedef struct  {
   long int mtype;          /* type of received/sent message */
   struct PASSENGER_DATA P;           /* text of the message */
}MESSAGE;



enum               {AVAIL_SLOTS, TO_CONSUME};
enum Nationalities { PALESTINIAN , JORDANIAN , FORIEGN };
enum Passport      { NOT_VALID , VALID };


#endif
