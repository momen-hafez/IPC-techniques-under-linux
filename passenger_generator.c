#include "local.h"
/* Recommended font size for displaying the code is 9 */


int Pg = 0;                                             /* Number of granted Passengers */
int Pd = 0;                                             /* Number of denied Passengers */
int Pu = 0;                                             /* Number of deleted passengers due to get impatient of waiting*/
int idx;                                                /* index variable used in Bus Process */
int continue_generating  = 0;                           /* this variable is used to be set if the number of passengers in hall is more than threshold Hmax*/
int number_of_passengers = 0;                           /* storing the number of passengers */

int passengers_process_id  [TOTAL_NUMBER_OF_PASSENGERS];/* array for saving the passenger's process IDs */
int mid                    [NUMBER_MESSAGE_QUEUES];     /* saving the message queue mid, which used to send back the information from the check points*/
int mid1                   [NUMBER_MESSAGE_QUEUES];     /* saving the message queue mid, which used to send back the information from the check points*/


struct BUS busses[NUMBER_BUSSES];                       /* storing the busses into array of structures */
int buss_pid     [NUMBER_BUSSES];                       /* storing the busses' process IDs */
int number_of_busses = 0;   
int number_of_passengers_in_the_hall = 0;               /* Current number of passenger in the hall */
struct PASSENGER *bus[BUS_CAPACITY];                    /* implement the bus as an array of Passenger's structures */


/* set of required functions */
int  number_in_halls();                               /* count the number of passengers in the hall (shared memory) */
void createPassenger(int , int , int , int , char *); /* create the passenger and insert it to the head of the linked list */
void generateDate(char *);                            /* generate date for the passenger */
void generatePassenger();                             /* generate passenger */
void createPassengerProcesses();                      /* create the passenger process */
void deleteNode_1(int id);                            /* delete node from the shared memory */
void checking_waiting_time_after_each_time_slice(int);/* updating the remaining time for each passenger after time slice, to check the impatiency level */
void is_done_processing_the_passenger(int);           /* signal handler function to make require operations after processing the passengers*/
void is_done(int);                                    /* check if the processing is done or not */
void mark_as_done(int , int);                         /* mark the passenger as done and transpose him to the hall */
void deleteNode(int , int);                           /* delete node from the passengers linked list */
void createBussProcess();                             /* create the busses processes */
void remove_all();                                    /* Remove every used IPCs */
void generateBusses();                                /* generate busses */
void display_the_hall();                              /* display the hall */ 

int i = 0;
int j = 0;

MESSAGE msg;                                         /* msg of type Passenger data to be sent through the message queue */
int counter = NUMPER_PASSENGERS;                     /* storing the number of passengers into a counter */

char     f_char = 'A';                               /* used to generate a keys using ftok() */
int      P_J_pid , F_pid;                            /**/
char     P_J_mid[10] , F_mid[10];                    /* These arraye are used to store the message queues ID to sent it to the Officesrs files*/
char     P_J_mid1[10] , F_mid1[10];                  /**/ 
int      process_nationality;                        /* to store the passenger nationality */
int      n;                                          /* used in msgrcv */
int      fd[2];                                      /* file descriptor for creating a pipe */
char     id_to_update[60];                           /* storing the ID of passenger who needs to update his information */
char     id[10];                                     /* storeing the passenger ID as a string */

char date_to_update[15];                             /* string to update the expiration date */
char validity[10];                                   /* string to store the validity of a passenger information */

int   pid_P_J      [NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J];        /* message queues ID for palestinian and jordanian check points*/
int   count_P_J    [NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J];        /* count of messages in each palestinian or jordanian message queue */
char  pid_P_J_str  [NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J + 1][10];/* store message id a array of string */
int   pid_F        [NUMBER_CROSSING_POINTS_F];                                   /**/
int   count_F      [NUMBER_CROSSING_POINTS_F];                                   /* Same variables but for Forign passengers*/
char  pid_F_str    [NUMBER_CROSSING_POINTS_F + 1][10];                           /**/ 


int min_number_of_passengers_p_j = 1e5;                                          /* these two variables are used for the passenger to determine */
int min_number_of_passengers_f   = 1e5;                                          /* which chick point has least number of passengers to choose  */
int which_queue_p_j;                                                             /**/
int which_queue_f;                                                               /**/

struct msqid_ds buf;                                                             /* Structure of message queue information to use with msgctl */

/* Shared memory requred variables */
int shmid;                                        /* shared memory ID */
struct HALL *hallptr;                             /* shared memory itself */
struct PASSENGER *P;                              /* Passenger structure */
struct PASSENGER *ptr;                            /* Passenger structure */



/* semaphore set --> we used the POSIX semaphore */
sem_t *sem1;                                     /* semaphore used to access the shared memory by the generator and the passenger */
sem_t *sem_waiting;                              /* semaphore used to update the number of Passengers getting Impatient and leave back */
sem_t *sem_granted;                              /* semaphore to store the number of granted passengers */
sem_t *sem_denied;                               /* semaphore to store the number of denied passengers */



  
int main(){
   printf("\n\n\n%s====>=====>====>======>=====>=====>======>=====>=====>======>======>====>\n",CYAN);
   printf("%s--> Enlarge your screen to see better output, Regards... :-)\n",CYAN);
   printf("%s====>=====>====>======>=====>=====>======>=====>=====>======>======>====>\n",CYAN);   
   printf("\n\n\n");
   printf("%s                              <=====<======<======<=========>======>======>=====>\n",GREEN);   
   printf("%s                             <<========<======<================>=======>========>>\n",GREEN);
   printf("%s                           <<<-----<-------- %sOur Travel is STARTING%s---------->-->>>\n",GREEN,YELLOW,GREEN);
   printf("%s                             <<========<======<================>=======>========>>\n",GREEN);
   printf("%s                              <=====<======<=======================>======>=====>\n\n\n",GREEN);   
   key_t key;
   srand((unsigned)(getpid()));  /* for generating random integer at each iteration */
   /* 
   * Creating the message queues which used by the Officers to return back the satus of each Passenger to him 
   *  we have two of them, one for the Palestinian and Jordanian passengers and the other for the Foreign passengers 
   */
   for (int i = 0 ; i < NUMBER_MESSAGE_QUEUES ; ++i, ++f_char) {
    key = ftok(".", f_char);
    if ( (mid1[i] = msgget(key, IPC_CREAT | 0660)) == -1 ) {
      perror("Message Queue create");
      exit(1);
     }
    }
   /*
   * Message queues for the Palestinian and Jordanian Passengers
   */
   for(int i = 0 ; i < NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J ; i++ , ++f_char){
      key = ftok(".", f_char);
      if ( (pid_P_J[i] = msgget(key, IPC_CREAT | 0660)) == -1 ) {
        perror("Message Queue create");
        exit(2);
     }
   }
   /*
   * changing the message queues ID's to strings
   */
   for(int i = 0 ; i < NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J ; i++){
      sprintf(pid_P_J_str[i] , "%d" , pid_P_J[i]); 
   }

  /*
   * Message queues for the Foreign Passengers
   */
   for(int i = 0 ; i < NUMBER_CROSSING_POINTS_F ; i++ , ++f_char){
      key = ftok(".", f_char);
      if ( (pid_F[i] = msgget(key, IPC_CREAT | 0660)) == -1 ) {
        perror("Message Queue create");
        exit(3);
     }
   }
   for(int i = 0 ; i < NUMBER_CROSSING_POINTS_F ; i++){
      sprintf(pid_F_str[i] , "%d" , pid_F[i]); 
   }
   /*
   * creation a Pipe used to send the status of information for the passenger to update after 
   * getting back from the check points
   */
   if(pipe(fd) < 0){
      perror("Pipe");
      return -14;
    }
    /*
    * Creation of the shared memory, which implemented as an linked list of passengers
    * this shared memory is access by the gengeneratorerater who arrange the passenger processes on it
    * and by the Busses which take the passengers to the jordanian side
    */
    if ( (shmid = shmget(ftok(".", 'Z'), 4096, IPC_CREAT | 0644)) < 0 ) {
      perror("shmget fail");
      exit(5);
    }
    /*
    * attach the generator process to the shared memory 
    */
    if ( (P = (struct PASSENGER *) shmat(shmid, 0, 0)) == (char *) -1 ) {
    perror("shmat: parent");
    exit(6);
    }
    hallptr = (struct PASSENGER *) P; /* Pointer to the shared Memory for accessing it */
    /*
    * This signal is used to ask the parent process to update the information for the passenger and check if the 
    * passenger access is denied or not
    */
    if (sigset(SIGUSR1, is_done) == SIG_ERR ) {
         perror("Sigset can not set SIGUSR1");
         exit(7);
    }
    /*
    * This signal is used basically for alarming the parent process to check the waiting time
    * for each passenger and determining which passenger is tending to leave and getting 
    * impatient of waiting
    */
    if (sigset(SIGALRM, checking_waiting_time_after_each_time_slice) == SIG_ERR ) {
         perror("Sigset can not set SIGALRM");
        exit(8);
    }
   /*
   * creation two children, representing the officers, these two childs handle all types of Passengers, 
   * and representing the required message queue for each officer.
   */
   switch(P_J_pid = fork()){
     case -1:
       perror("Fork");
       return 9;
    case 0:
       /* 
       * the mid of the queue used to return back the passenger info to him, as well as 
       * the message queue for the first check point of type Palestinian-Jordanian
       */
       sprintf(P_J_mid1 , "%d" , mid1[0]);                                                  
       execlp("./cross_point_P_J", "cross_point_P_J", pid_P_J_str[0], P_J_mid1 , (char *)0);
       perror("Execlp");
       return 10;
     } 
    
   switch(F_pid = fork()){
     case -1:
       perror("Fork");
       return 11;
     case 0:
       /* 
       * the mid of the queue used to return back the passenger info to him, as well as 
       * the message queue for the first check point of type forign
       */
      sprintf(F_mid1 , "%d" , mid1[1]);
      execlp("./cross_point_F", "cross_point_F", pid_F_str[0], F_mid1 , (char *)0);
      perror("Execlp");
      return 12;
    }   
    /*
    * Generating the Busses information
    */
    for(int i = 0 ; i < NUMBER_BUSSES ; i++){
      generateBusses();
    }

   /* =================================== semaphores ============================
    * when initialization we specify if the semaphore used for multi processing 
    * system or multi threading, and the initial values of the semaphore
   /* semaphore for accessing the shared memory between the generator and busses */
   sem1 = (sem_t *) malloc(sizeof(sem_t));
   if(sem_init (sem1, 1, 1) == -1){
     perror("semaphore - shared memory");
     return 13;
   }
   /* semaphore for updating the number of Passengers who got impatient and return back */
   sem_waiting = (sem_t *) malloc(sizeof(sem_t));
   if(sem_init (sem_waiting, 1, 1) == -1){
     perror("semaphore - updating number of passengers return back due to the impatiency level");
     return 14;
   }
   /* semaphore for updating the number of granted Passengers */
   sem_granted = (sem_t *) malloc(sizeof(sem_t));
   if(sem_init (sem_granted, 1, 1) == -1){
     perror("semaphore - updating number of passengers whoses access is granted");
     return 15;
   }
   /* semaphore for updating the number of denied Passengers */
   sem_denied  = (sem_t *) malloc(sizeof(sem_t));
   if(sem_init (sem_denied, 1, 1) == -1){
     perror("semaphore - updating number of passengers whoses access is denied");
     return 16;
   }
   /*================================End semaphores===============================*/
   
   /* start Running Our Program 
   *  according to the project description, the simulation will end if the number of granted passengers is becomr more than
   *  a specific value or the number of denied or getting impatient passengers, so we start by generating the passenger information, and 
   *  then we create the passenger process to start the process of traveling as we will se later ...... 
   *  NOTE: this is a simulation code, so the numbers and waiting times is chosed to be small in case of observing the output ... 
   */
   while(Pu < NUMBER_IMPATIENT && Pg < NUMBER_GRANTED && Pd < NUMBER_DENIED){
      /*
      * first we set the alarm, to call the signal SIGALARM after each time slot value, and then we 
      * decrement the impatiency lever for each passenger and determining the passengers who decided to get back.
      */
      alarm(3);
      /* Generating the passenger, and insert it ro the head of the linked list */
      generatePassenger();
      /* Creating the passenger process */
      createPassengerProcesses();
      /* this sleep is to monitoring the output */
      sleep(3);
      /* 
      *  according to the project description we need to stop generating passengers if the number of passenger
      *  in the hall is more than a specific threshold, so we find the current number of passenger in the shared memory
      */
      number_of_passengers_in_the_hall = number_in_halls();
      /*
      * if the number of passengers in the hall > MAX_THRESHOLD, stop generating and ask the busses
      * to start transposing the passengers, when the number became below MIN_THRESHOLD, begin generating again
      */
      if(number_of_passengers_in_the_hall > MAX_THRESHOLD){ /* stop generating */
         continue_generating = 1;
         while(continue_generating){
            sleep(1);
            printf("\n\n%s=================================================================================\n",YELLOW);
            printf("%s<======== %sWARNING The Hall is FULL, STOP generating Passengers !!!%s========>\n",YELLOW , RED , YELLOW); 
            printf("%s=================================================================================\n",YELLOW);
            createBussProcess();   
            int keep_count = number_in_halls();      
            if(keep_count < MIN_THRESHOLD){
               continue_generating = 0; /* keep generating */
            }
         }
      }
      /*
      * start transposing with Busses if the number of passenger in the halls is more than a certain value.
      */
      if(number_of_passengers_in_the_hall >= NUMBER_BUSSES * BUS_CAPACITY){
         createBussProcess();
         sleep(1);
         number_of_passengers_in_the_hall = number_in_halls();
      }
      /* 
      * this Counter is related to the number of created passengers, but the semulation has a specific conditions to end
      * so we will keep it commented
      */
      //counter--;  

   }
   /* sleep for observng the output */
   sleep(3);
   remove_all();
   printf("\n\n");
   printf("%s !=========!=======!========!=========!==========!========!========!\n",GREEN);
   printf("%s -----> Number of Granted Passengers            = %d.\n",YELLOW,Pg);
   printf("%s -----> Number of Denied Passengers             = %d.\n",YELLOW,Pd);
   printf("%s -----> Number of Passengers who get impatient  = %d.\n",YELLOW,Pu);
   printf("%s !=========!=======!========!=========!==========!========!========!\n",GREEN);
   printf("\n\n\n");
   printf("%s                              <=====<======<======<=========>======>======>=====>\n",YELLOW);   
   printf("%s                             <<========<======<================>=======>========>>\n",YELLOW);
   printf("%s                           <<<-----<-------- %sOur Travel is END  %s---------->-->>>\n",YELLOW,RED,YELLOW);
   printf("%s                             <<========<======<================>=======>========>>\n",YELLOW);
   printf("%s                              <=====<======<=======================>======>=====>\n\n\n",YELLOW);
 //  display();
  

}
/* Create Passenger Process */
void createPassengerProcesses(){
  /*
  * creation the Passenger Process
  */
  passengers_process_id[number_of_passengers - 1] = fork();
  int pid;
  int temp = passengers_process_id[number_of_passengers - 1];
  /*
  * storing the Process if or the passenger in the linked list 
  */
  head->data.pid = temp;
  if(passengers_process_id[number_of_passengers - 1] == -1){
      perror("fork");
      exit(17);
  }else if(passengers_process_id[number_of_passengers - 1] > 0){
     /* Parent Code */
     head->data.pid  = passengers_process_id[number_of_passengers - 1]; 
  }else{
       /* Passenger Code */
       head->data.pid = getpid();
       process_nationality = head->data.nationality;
       /* The passenger store his information to send it to the Officers */
       msg.P = head->data;
       /* To determinr the Officer of the least number of passenger */
       min_number_of_passengers_p_j = 1e5;
       min_number_of_passengers_f   = 1e5;
       /* If the Passenger is Palestinian or Jordanian */
       if(process_nationality == PALESTINIAN || process_nationality == JORDANIAN){
          /* if its information still un checked */
          if(msg.P.is_done == 0){
             msg.mtype = 1;            /* determining the message type, it's fixed, as there's no need for grouping the messages in different types */
             msg.P     = head->data;   /* storing the Passenger data in the message structure */
             /* 
             * after packeging the data by the Passenger, now he want to determine which check point to choose,
             * he will count the number of passengers waiting for being manipulated at the check points, and he 
             * will choose the check point with the least number of passengers
             */
             for(int j = 0 ; j < NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J ; j++){
                msgctl(pid_P_J[j], IPC_STAT, &buf);
                count_P_J[j] = buf.msg_qnum;
                if(count_P_J[j] <= min_number_of_passengers_p_j){
                   min_number_of_passengers_p_j = count_P_J[j];
                 }
              }
             for(int j = 0 ; j < NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J ; j++){
                 if(min_number_of_passengers_p_j == count_P_J[j]){
                 which_queue_p_j = j + pid_P_J[0];
                 }
              }
             /* this sleep is  for observing the output */ 
             sleep(1);
             if(msg.P.nationality == PALESTINIAN){
                 printf("\n%s    ========================<<<<<<<< Arriving to the check points STAGE >>>>>>>>========================\n",WHITE);
                 printf("%s    Passenger %d has arrived, he is PALESTINIAN, and he choose check point %d\n",YELLOW,msg.P.id , which_queue_p_j - pid_P_J[0]);
             }else{
                 printf("\n%s    ========================<<<<<<<< Arriving to the check points STAGE >>>>>>>>========================\n",WHITE);
                 printf("%s    Passenger %d has arrived, he is JORDANIAN, and he choose check point %d\n",BLUE,msg.P.id , which_queue_p_j - pid_P_J[0]);
             }
             printf("%s    ========================<<<<<<<< ---------------------------------- >>>>>>>>========================\n\n",WHITE);
             /* Sending the data to the officer in the check point with the least number of passengers */
             if(msgsnd(which_queue_p_j, &msg, sizeof(msg), 0) == -1 ) {
	       perror("msgsend");
               exit(18);
              }
             sleep(1);
             /* here, the passenger received his data from the officer */
             if((n = msgrcv(mid1[0], &msg, sizeof(MESSAGE), 1,0)) == -1 ) {
               perror("msgrcv");
               exit (19);
             } 
             /*
             * Manipulating the data to read the status from the officer 
             */
             strcpy(id_to_update , "");
             sprintf(id_to_update , "%d" , msg.P.id);
             strcat(id_to_update , " ");
             strcat(id_to_update , msg.P.expired_date);
             strcat(id_to_update , " ");
             sprintf(validity , "%d", msg.P.passport_v); 
             strcat(id_to_update , validity);
             strcpy(validity , "");   
             /*
             * now we will write the status data through a pipe to the generator process to send the passenger
             * to the hall, or denied him
             */
             if(write(fd[1] , id_to_update , strlen(id_to_update)) == -1){
                perror("write");
                exit (20);
             }     
             /* calling the signal for reading the pipe from the parent */       
             kill(getppid() , SIGUSR1);
          }
       }else{
        /* This code is for the forign Passengers, the same as the previous code */
         if(msg.P.is_done == 0){
            msg.mtype = 1;
            msg.P     = head->data;
            for(int i = 0 ; i < NUMBER_CROSSING_POINTS_F ; i++){
               msgctl(pid_F[i], IPC_STAT, &buf);
               count_F[i] = buf.msg_qnum;
               if(count_F[i] < min_number_of_passengers_f){
                   min_number_of_passengers_f = count_F[i];
               }
             }
            for(int i = 0 ; i < NUMBER_CROSSING_POINTS_F ; i++){
                if(min_number_of_passengers_f == count_F[i]){
                     which_queue_f = i + pid_F[0];
                 }
             }
            sleep(1);
            printf("\n");
            printf("%s     ========================<<<<<<<< Arriving to the check points STAGE >>>>>>>>========================\n",WHITE);
            printf("%s     Passenger %d has arrived, he is FORIGN, and he choose check point %d\n",RED,msg.P.id , which_queue_f - pid_F[0]);
            printf("%s     ========================<<<<<<<< ---------------------------------- >>>>>>>>========================\n\n",WHITE);
            if (msgsnd(which_queue_f, &msg, sizeof(msg), 0) == -1 ) {
	       perror("msgsend");
               exit (21);
	    }

	   if((n = msgrcv(mid1[1], &msg, sizeof(MESSAGE), 1, 0)) == -1 ) {
              perror("msgrcv");
              exit (22);
            }
            strcpy(id_to_update , "");
            sprintf(id_to_update , "%d" , msg.P.id);
            strcat(id_to_update , " ");
            strcat(id_to_update , msg.P.expired_date);
            strcat(id_to_update , " ");
            sprintf(validity , "%d", msg.P.passport_v); 
            strcat(id_to_update , validity);
            strcpy(validity , "");
            if(write(fd[1] , id_to_update , 50) == -1){
              perror("write");
              exit (23);
            }
            kill(getppid() , SIGUSR1);
           }
         }
      exit(0);
      }
   }
/*
* Generate Passenger Data
*/   
void generatePassenger(){
  
  int determinant = rand() % 10 + 1 ;        /* Determinant of the passenger nationality */
  int nationality;                           /* storeing the nationality for the passenger */
  int wait_time;                             /* storing the wait time for the passenger ot the impatiency level */
  int determinant_validity;                  /* determining if the passenger passport is done or not */
  int validity;                              /* storing the validity status of each passenger's passport */
  char date[15];                             /* store the date of each passenger */
  /* determining the nationalities according to the given percentages */
  if(determinant >= 1 && determinant < 10 * PERCENTAGE_P){
     nationality = PALESTINIAN;
  }else if(determinant >= 10 * PERCENTAGE_P && determinant < 10 * PERCENTAGE_P + 10 * PERCENTAGE_J){
     nationality = JORDANIAN;
  }
  else if(determinant >= 10 * PERCENTAGE_P + 10 * PERCENTAGE_J && determinant <= 10 * PERCENTAGE_P + 10 * PERCENTAGE_J + 10 * PERCENTAGE_F){
     nationality = FORIEGN;
  }
  /* storing the wait time for each signal */
  wait_time = (rand() % 4) + 12;
  determinant_validity = rand() % 10;
  if(determinant_validity >= 1 && determinant_validity < 9){
     validity = VALID;
  }else{
    validity = NOT_VALID;
  }
  generateDate(date);
  /* inserting the passenger to the linked list */
  createPassenger(number_of_passengers , wait_time , nationality , validity , date);
  number_of_passengers++;
}
/* generate date function */
void generateDate(char *date){
  int day, year , month;
  char day_s[10] , month_s[10] , year_s[10];
  day   = rand() % 30 + 1;
  month = rand() % 12 + 1;
  year  = rand() % 3  + 2020; 
  sprintf(day_s   , "%d" , day  );
  sprintf(month_s , "%d" , month);
  sprintf(year_s  , "%d" , year );
  strcpy(date , year_s );
  strcat(date , "-"    );
  strcat(date , month_s);
  strcat(date , "-"    );
  strcat(date , day_s  );
}

void checking_waiting_time_after_each_time_slice(int the_sig){
  /* 
  this function is sensetive to SIGALARM to determin which passenger decided to get back
  and updating the impatiency level of each passenger after each time slot.
  */
  struct PASSENGER *ptr;
  if(head==NULL) {
        printf("nList is empty:n\n");
        return;
   }
   else{
       ptr=head;
       while(ptr!=NULL){
           if(ptr->data.is_done == 0 && ptr->data.passport_v == 1){
              ptr->data.wait_time--;
              if(ptr->data.wait_time <= 0){
              /* we used semaphore sem_waiting to update the number of passengers who get impatient */
                if(sem_wait(sem_waiting) == -1){
                  perror("sem wait");
                  exit (24);
                }
                Pu++;
                if(sem_post(sem_waiting) == -1){
                  perror("sem wait");
                  exit (25);
                }
                /* delete the passenger from the linked list */
                deleteNode(ptr->data.id , 0);
                /* kill the passenger process */
                kill(ptr->data.pid , SIGKILL);
              }
            }
           ptr=ptr->next ;
         }
      }

}

/* mark the passenger with OK information as done and send him to the hall */
void mark_as_done(int id , int state){
  struct PASSENGER *ptr;
  if(head==NULL) {
        printf("nList is empty:n");
        return;
   }
   else{
       ptr=head;
       while(ptr!=NULL){
           if(ptr->data.id == id){
              ptr->data.is_done = state;
              if(state != 0 && strcmp("",ptr->data.expired_date) != 0){
                printf("\n");
                printf("%s    ===>===>===>===>===>===>===>===>===>=== Going to the HALL stage ===>===>===>===>===>===>===>===>\n",GREEN);
                printf("%s    Passenger %d, Has finished his Checking, now he will move to the hall...\n",YELLOW,ptr->data.id); 
                printf("%s    ===>===>===>===>===>===>===>===>===>===>======>====>===>===>===>===>===>===>===>===>===>===>===>\n",GREEN);
                /* updating the number of granted passengers */
                if(sem_wait(sem_granted) == -1){
                  perror("sem wait");
                  exit (26);
                }
                Pg++;
                if(sem_post(sem_granted) == -1){
                  perror("sem wait");
                  exit (27);
                }
                /* accessing the shared mimory to insert the passenger to the hall using sem1 semapore*/
                 if(sem_wait(sem1) == -1){
                   perror("sem wait");
                   exit (28);
                 }
                 struct PASSENGER *link = (struct PASSENGER*) malloc(sizeof(struct PASSENGER)); 
                 link->data = ptr->data;
		 link -> next = hallptr->P;
		 hallptr->P = link;
		 if(sem_post(sem1) == -1){
		  perror("sem post");
                   exit (29);
		 }
		 display_the_hall();
		 sleep(1);
              }else{
                /* delete the node with denied status */
                deleteNode(ptr->data.id , 1);
                /* Killing the denied passenger process */
                kill(ptr->data.pid , SIGKILL);
              }
              break;
           }
           ptr=ptr->next ;
         }
      }
   }
/* 
* check if the status of the passenger is OK and can be go to the hall 
* this function is sensiteve to the signal SIGUSR1
*/ 

void is_done(int the_sig){
  char id_s[10] , date[15];
  int i = 0;
  int iid;
  int is_valid;
  if(read(fd[0] , id , 50) != -1){
       char *token;
       token = strtok(id," ");
       i = 0;
       while( token != NULL ) {
          if(i == 0) iid = atoi(token);
          else if (i == 1) strcpy(date , token);
          else is_valid = token[0] - '0';
          i++;
          token = strtok(NULL, " ");
         }
       if(strcmp(date , "-") == 0 || is_valid == 0){
          if(sem_wait(sem_denied) == -1){
              perror("sem wait");
              exit (30);
          }
          Pd++;
          if(sem_post(sem_denied) == -1){
             perror("sem wait");
             exit (31);
          }
          mark_as_done(iid , 0);
      }else{
        mark_as_done(iid , 1);
      }
       strcpy(date , "");
   }
}
/* inserting the passenger to the linked list function */
void createPassenger(int id , int wait_time , int nationality , int valid , char *date){
  struct PASSENGER_DATA P;
  struct PASSENGER *link = (struct PASSENGER*) malloc(sizeof(struct PASSENGER));
  P.id = id;
  P.nationality = nationality;
  P.passport_v = valid;
  P.wait_time = wait_time;
  P.is_done   = 0;
  strcpy(P.expired_date , date);
  link->data = P;
  link -> next = head;
  head = link;

}
/* delete node from the linked list */
void deleteNode(int key , int status)
{
      //temp is used to freeing the memory
       struct PASSENGER *temp;
       struct PASSENGER *prev;
       temp = head;
       if(temp != NULL && temp->data.id == key){
         head = temp -> next;
         free(temp);
         return;
       }
       while(temp != NULL && temp->data.id != key){
          prev = temp;
          temp = temp -> next;
       }
       if(temp == NULL){
          return;
       }
       prev -> next = temp -> next;
       free(temp);
       printf("\n");
       printf("%s    ==!====!=====!=====!=====!=====!=====!======!====!=====!=====!====!====!\n",YELLOW);
      if(status == 0){ /*T he passenger get impatient */
        printf("%s    Passenger %d Can't wait, and he desided to return back!\n",RED,key);
      }else{          /* The passenger is denied */
        printf("%s    Passenger %d, has been denied !\n",RED,key);
      }
      printf("%s    ==!====!=====!=====!=====!=====!=====!======!====!=====!=====!====!====!\n\n",YELLOW);
}

/* delete node from the shared memory */
void deleteNode_1(int key){
    struct PASSENGER *temp;
    struct PASSENGER *prev;
    temp = hallptr->P;
    if(temp != NULL && temp->data.id == key){
       hallptr->P = temp -> next;
       free(temp);
       return;
    }
       while(temp != NULL && temp->data.id != key){
          prev = temp;
          temp = temp -> next;
       }
       if(temp == NULL){
          return;
       }
       prev -> next = temp -> next;
       free(temp);
}
/* generate Buss */
void generateBusses(){
  int id = number_of_busses;                            /* storing the id of the buss */
  int waiting_time = rand() % BUS_TIME_PERIOD + 2;      /* storing the value of TBh   */
  struct BUS bus ;                                      /* creating a bus structure  */
  bus.bus_id = id;
  bus.waiting_time = waiting_time;
  bus.is_available = 1;                                 /* storing the status of the bus if it's available or not */
  bus.number_of_travels = 0;                            /* storing the number of travels for each buss */
  busses[number_of_busses++] = bus;
}

/* Display the Hall function */
void display_the_hall(){
  struct PASSENGER *ptr;
  printf("\n");
  printf("%s    !===!=====!======!======!======!======!======!====== The Hall ======!=====!======!======!======!======!======!====!\n",YELLOW);
  if(P==NULL) {
        printf("nList is empty:n");
        return;
   }
   else{
       ptr=hallptr->P;
       while(ptr!=NULL && ptr->data.pid != 0){
           if(ptr->data.nationality == PALESTINIAN){
                printf("%s    Passenger: %d -> PALESTINIAN -> PASSPORT VALIDITY: %d -> IMPATIENCY LEVEL: %d - IS DONE: %d -> EXPIRED DATE: %s\n",GREEN, ptr->data.id , ptr->data.passport_v , ptr->data.wait_time ,ptr->data.is_done , ptr->data.expired_date);
              }else if(ptr->data.nationality == JORDANIAN){  
                printf("%s    Passenger: %d ->   JORDANIAN -> PASSPORT VALIDITY: %d -> IMPATIENCY LEVEL: %d - IS DONE: %d -> EXPIRED DATE: %s\n",GREEN, ptr->data.id , ptr->data.passport_v , ptr->data.wait_time ,ptr->data.is_done , ptr->data.expired_date);
              }else{
                printf("%s    Passenger: %d ->     FOREIGN -> PASSPORT VALIDITY: %d -> IMPATIENCY LEVEL: %d - IS DONE: %d -> EXPIRED DATE: %s\n",GREEN, ptr->data.id , ptr->data.passport_v , ptr->data.wait_time ,ptr->data.is_done , ptr->data.expired_date);
              }
           ptr=ptr->next ;
           }
      }
printf("%s    !=====!======!=======!======!========!=====!======!======!=======!=======!========!=====!=====!=====!=======!=====!\n\n",YELLOW);      
}
/* Number of passengers in the hall */
int number_in_halls(){
  struct PASSENGER *ptr;
  int count = 0;
  if(sem_wait(sem1) == -1){
      perror("sem wait");
      exit (32);
   }
  if(P==NULL) {
        printf("nList is empty:n");
        return -1;
   }
   else{
       ptr=hallptr->P;
       while(ptr!=NULL){
           count++;
           ptr=ptr->next ;
         }
      }
  if(sem_post(sem1) == -1){
      perror("sem wait");
      exit (33);
   }    
   return count;   
}

/* Create Bus Process */
void createBussProcess(){
        /* first we determine the currently available bus with the least number of travels */
        int min = 1e5;
        for(int j = 0 ; j < NUMBER_BUSSES ; j++){
          if(busses[j].is_available == 1 && busses[j].number_of_travels < min){
             min = busses[j].number_of_travels ;
             i = j;
           }
        }
        /* we make it un available */
        busses[i].is_available = 0;
        /* now we will create it */
        switch(buss_pid[i] = fork()){
           case -1:
              perror("fork");
              exit(34);
           case 0 :
              /* the buss process attach to our shared memory */
              if ( (P = (struct PASSENGER *) shmat(shmid, 0, 0)) == (char *) -1 ) {
                 perror("shmat: parent");
                  exit(35);
               }
              hallptr = (struct PASSENGER *) P;
              /* using the semaphore sem1 to access the shared memory */
              if(sem_wait(sem1) == -1){
                  perror("sem wait");
                  exit (36);
              } 
              /* now the bus will start transposing the passengers */
              printf("\n");
              printf("%s    ===========================================================================\n",BLUE);
              printf("%s    Now, Bus %d will start transposing the passengers to the jordanian side ...\n",BLUE,busses[i].bus_id); 
              printf("%s    ===========================================================================\n\n",BLUE);
              idx = 0;
              ptr = hallptr->P; 
              ptr = ptr-> next; 
              
              while(ptr != NULL){
                 /* storing the passenger in the bus */
                 if(ptr->data.pid != 0){
                     bus[idx++] = ptr;
                 }
                ptr = ptr -> next;
              }
             if(idx >= BUS_CAPACITY - 1){
                /* deleting the passengers information and kill them */
                while(idx && hallptr->P -> next != NULL){
                   idx--;
                   deleteNode_1(hallptr->P->data.id);
                   kill(hallptr->P->data.pid , SIGKILL);
                   hallptr->P = hallptr->P -> next;
                 }
              }

            printf("\n");
            printf("%s    ==>===>====>====>===>====>====>=====>=====>===>=====>===>=====>=====>====>===\n",YELLOW);
            printf("%s    Now, Bus %d  start moving and it will take about %d seconds to return back ...\n",YELLOW,busses[i].bus_id,busses[i].waiting_time); 
            printf("%s    ==>===>====>====>===>====>====>=====>=====>===>=====>===>=====>=====>====>===\n",YELLOW);
            /* the bus process sleep until it come back from the travel */
            sleep(busses[i].waiting_time);
            
            printf("\n");
            printf("%s    ====<===<===<===<====<====<====<===<====<====<====<====<====<===<====<======\n",GREEN);
            printf("%s    Now, Bus %d  has been returned back back ...\n",GREEN,busses[i].bus_id); 
            printf("%s    ====<===<===<===<====<====<====<===<====<====<====<====<====<===<====<======\n",GREEN);

             if(sem_post(sem1) == -1){
		  perror("sem post");
                   exit (37);
		 }            
            exit(0); 
        }
       /* now our bus is availabe, and we increased its number of travels */
       busses[i].is_available = 1;  
       busses[i].number_of_travels++;

}
/* Removing the message queues Code */
void remove_all(){
  for(int i = 0 ; i < NUMBER_CROSSING_POINTS_F ; i++){
      msgctl(pid_F[i], IPC_RMID, &buf);
   }
   for(int j = 0 ; j < NUMBER_CROSSING_POINTS_P + NUMBER_CROSSING_POINTS_J ; j++){
      msgctl(pid_P_J[j], IPC_RMID, &buf);
   }
 for(int j = 0 ; j < 2 ; j++){
     msgctl(mid1[j], IPC_STAT, &buf);
  }
}
