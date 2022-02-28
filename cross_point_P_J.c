#include "local.h"

int check_expiration(char *);   /* mcheck expiration */

MESSAGE msg;                    /* msg to send to the passengers */

struct msqid_ds buf;            /* message queue structure */
  
int main(int argc, char *argv[ ]){
  /* storing the message queues IDs information */
  int mid_P , n , mid_P1;
  int mid_P_J[NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J];

  MESSAGE msg;
  if(argc != 3){
     fprintf(stderr, "Usage: %s msq_id &\n", argv[0]);
     return 1;
  }
  mid_P  = atoi(argv[1]); 
  mid_P1 = atoi(argv[2]);
  mid_P_J[0] = mid_P;
  for(int i = 1 ; i < NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J ; i++){
    mid_P_J[i] = mid_P_J[i - 1] + 1; 
  }
  while(1){
     for(int j = 0 ; j < NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J ; j++){
         /* recieve messages from the Passengers */    
        if((n = msgrcv(mid_P_J[j], &msg, sizeof(MESSAGE), 1, 0)) == -1 ) {
          perror("msgrcv | ");
          return 40;
        }else if(n == 0){
      	  break;
         }
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
           if (msgsnd(mid_P1, &msg, sizeof(msg), 0) == -1 ) {
	     perror("msgsend");
             return 41; 
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
  
