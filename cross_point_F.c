#include "local.h"

MESSAGE msg;                  /* msg to send to the passengers */


struct msqid_ds buf;          /* message queue structure */

int  check_expiration(char *);/* check expration function */


int main(int argc, char *argv[ ]){
  /* storing the message queues IDs information */
  int mid_f , n = 0 , mid_F1;
  int mid_F[NUMBER_CROSSING_POINTS_F];
  if(argc != 3){
     fprintf(stderr, "Usage: %s msq_id &\n", argv[0]);
     return 38;
  }
  mid_f  = atoi(argv[1]);
  mid_F1 = atoi(argv[2]);
  
  /* store the message queue IDs for all foreign cross points */
  mid_F[0] = mid_f;
  for(int i = 1 ; i < NUMBER_CROSSING_POINTS_F ; i++){
    mid_F[i] = mid_F[i - 1] + 1; 
  }

  while(1){
     for(int j = 0 ; j < NUMBER_CROSSING_POINTS_F ; j++){
        /* recieve messages from the Passengers */
        if((n = msgrcv(mid_F[j], &msg, sizeof(MESSAGE), 1 , 0)) == -1 ) {
           perror("msgrcv | ");
           return 2;
        }else if(n == 0){
      	  break;
        }
       /* if we have a message */
       if(n == 48){
          if( msg.P.passport_v == 1) msg.P.is_done = 1;
          else msg.P.is_done = 0;
          msg.mtype = 1;
          char temp[15];
          strcpy(temp , msg.P.expired_date);
          /* here we check the expiration */ 
          int x = check_expiration(temp);
          if (x == 0)  strcpy(msg.P.expired_date , "-");
          /* send feedback message to the passenger */
          if (msgsnd(mid_F1, &msg, sizeof(msg), 0) == -1 ) {
	    perror("msgsend");
            return 39; 
          }
         }
      }
    }
  }
int check_expiration(char *date){
   int  year ,     day ,     month;
   char *year_s , *day_s , *month_s;
   char *token;
   int i = 0;
   token = strtok(date,"-");
   while( token != NULL ) {
      if(i == 0) year = atoi(token);
      else if (i == 1) month = atoi(token);
      else day = atoi(token);
      i++;
      token = strtok(NULL, "-");
   }
   return (year < 2021 || (year == 2021 && month < 12) || (year == 2021 && month == 12 && day < 30)) ?  0 :  1;
}

