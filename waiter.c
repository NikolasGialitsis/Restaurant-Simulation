/* W A I T E R */ 

#include <stdio.h>   //printf
#include <stdlib.h> //malloc
#include <sys/types.h> //pid_t sem_t 
#include <sys/ipc.h> // mode_t
#include <sys/shm.h> // shmat shmid_ds 
#include <semaphore.h> //sem_destroy 
#include <errno.h> //errno
#include <assert.h> //assert
#include <sys/stat.h> //S_IRUSR S_IWUSR
#include <fcntl.h> // O_CREAT O_EXCL
#include <string.h> //strcmp strtok
#include <unistd.h> //fork
#include <sys/wait.h> // wait
#include "header.h"
#include <time.h>

int main(int argc,char* argv[]){

	if (argc < 7){
		perror("Not enough arguments given to function waiter \n");
		exit(1);
	}

	int i ;
	int period = 0;
	int shmid = 0 ;
	int moneyamount = 0;
	for ( i = 1 ; i < argc ; i++){
		if( strcmp(argv[i],"-d") ==0){
			period = atoi(argv[i+1]);
			i++; 
		}
		else if( strcmp(argv[i],"-s")== 0){
			shmid = atoi(argv[i+1]);
			i++;
		}
		else if(strcmp ( argv[i],"-m")== 0){
			moneyamount = atoi(argv[i+1]);
			i++;
		}
		else ;
	}

	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) { 
		perror("clock_gettime"); 
		exit(-1); 
	}

	ts.tv_sec += 5;

	/*Shared memory attachment*/
	Sh_Memory * shmem = (Sh_Memory*)shmat(shmid,0,0);
	if(shmem == (Sh_Memory*)(-1)){
		perror("Shared memory attachment");
	}
	Table* table= (Table *)((void*)shmem+sizeof(Sh_Memory));
	
	int id;
	sem_wait(&(shmem->sem_waiter));
		(shmem->waiter_id)++;
		id = shmem->waiter_id;
		printf("Waiter %d summoned\n",id);
	sem_post(&(shmem->sem_waiter));
	
	int service_time  = 0;

	int group_id;
	int pay_id;

	int money;
	srand(time(NULL) ^ (getpid()<<16));//for accurate seeding

	while(shmem->groups_left != 0){

		if(sem_timedwait(&(shmem->order_request),&ts) == 0){
			ts.tv_sec += 5;
			group_id = shmem->curr_id;
			printf("\t\tWaiter %d:started taking orders from group with ID = %d \n",id,group_id);
			sem_post(&(shmem->sem_customer_order));
			service_time = rand()%period + 1;
			sleep(service_time);
			
			sem_wait(&(shmem->sem_waiter));
			shmem->order_waiter = id;
			printf("\t\tWaiter %d:finished taking orders from group with ID = %d \n",id,group_id);
			sem_post(&(shmem->order_answer));			
		}
		ts.tv_sec += 5;
		
		if(sem_timedwait(&(shmem->pay_request),&ts) == 0){	
			ts.tv_sec += 5;
			pay_id = shmem->pay_waiter;
			group_id = shmem->curr_id;
			
			if(pay_id == id){
				money = rand()%moneyamount+1;
				printf("\t\tWaiter %d:The total cost will be %d for group with ID = %d \n",id,money,group_id);	
				service_time = rand()%period + 1;
				sleep(service_time);
				printf("\t\tWaiter %d:got payed %d by group with ID = %d \n",id,money,group_id);
				shmem->income += money;
				sem_post(&(shmem->sem_customer_pay));
				sem_post(&(shmem->pay_answer));

			}
			else{
				sem_post(&(shmem->pay_request));	
			}
		}
		ts.tv_sec += 5;
	
	}
	printf("Waiter %d has finished working\n",id);
	return 0;
}