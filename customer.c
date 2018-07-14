/* C U S T O M E R */

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

	if (argc < 5){
		perror("Not enough arguments given to function waiter \n");
		exit(1);
	}

	int i ;
	int period = 0;
	int shmid = 0 ;
	int people = 0;

	for ( i = 1 ; i < argc ; i++){
		if( strcmp(argv[i],"-d") ==0){
			period = atoi(argv[i+1]);
			i++; 
		}
		else if( strcmp(argv[i],"-s")== 0){
			shmid = atoi(argv[i+1]);
			i++;
		}
		else if( strcmp(argv[i],"-n")==0){
			people = atoi(argv[i+1]);
			i++;
		}
		else ;
	}

	int coinflip;
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) { 
		perror("clock_gettime"); 
		exit(-1); 
	}

	ts.tv_sec += 5;
	Sh_Memory * shmem = (Sh_Memory*)shmat(shmid,0,0);
	if(shmem == (Sh_Memory*)(-1)){
		perror("Shared memory attachment");
	}
	Table* table= (Table *)((void*)shmem+sizeof(Sh_Memory));

	int group_id;
	int group_waiter;
	int time_staying = 0;
	int table_id = -1;
	srand(time(NULL) ^ (getpid()<<16));//for accurate seeding

	
	printf("Group of %d customers arrived at the entrance\n",people);
	sem_wait(&(shmem->queue));
			
		sem_wait(&(shmem->sem_customer_enter));	
		shmem->group_size = people;
		(shmem->customer_id)++;
		group_id = shmem->customer_id;

		printf("Group of %d customers entering restaurant,ID = %d \n",people,group_id);
		
		sem_post(&(shmem->entry_request));
		sem_wait(&(shmem->entry_answer));
		


		if ( (shmem->space_flag) == 1 ){
			table_id = shmem->table_id;
			sem_post(&(shmem->queue));
			printf("\t\tGroup of %d sat on table %d, ID = %d\n",people,table_id,group_id);
			shmem->total_capacity -= people;
			sem_wait(&(shmem->sem_customer_order));
			shmem->curr_id = group_id;
			
			sem_post(&(shmem->order_request));	
			sem_wait(&(shmem->order_answer));
			
			group_waiter = shmem->order_waiter;
			sem_post(&(shmem->sem_waiter));
			
			printf("\t\tGroup of %d ordered to waiter %d: , ID = %d \n",people,group_waiter,group_id);
			
			time_staying = rand()%period + 1;
			sleep(time_staying);

			printf("\t\tGroup of %d wants to pay waiter %d: , ID = %d \n",people,group_waiter,group_id);

			sem_wait(&(shmem->sem_customer_pay));
			shmem->curr_id = group_id;
			shmem->pay_waiter = group_waiter;
			
			sem_post(&(shmem->pay_request));
			sem_wait(&(shmem->pay_answer));

			printf("\t\tGroup of %d is pleased and is leaving now , ID = %d \n",people,group_id);


			sem_wait(&(shmem->sem_customer_exit));
			shmem->leaving_table = table_id;
			shmem->leaving_group = group_id;
			sem_post(&(shmem->exit_request));
			sem_wait(&(shmem->exit_answer));
			
			(shmem->served)++;
			shmem->total_capacity += people;
			(shmem->groups_left)--;
			
			exit(0);

		}
		else if((shmem->space_flag) == 0){
			shmem->bar_capacity -= people;
			printf("\tGroup of %d waiting at the bar , ID = %d \n",people,group_id);
			sem_post(&(shmem->queue));
			while(group_id != shmem->first_customer){
				sem_timedwait(&(shmem->table_flag),&ts);
				ts.tv_sec += 5;
				if(group_id != shmem->first_customer){
					srand(time(NULL) ^ (getpid()<<16));//for accurate seeding
					coinflip = rand()%2;
					if(coinflip){
						printf("\tGroup of %d is leaving bar,they couldnt wait anymore , ID = %d\n",people,group_id);
						
						sem_wait(&(shmem->exit_bar));
						shmem->leaving_bar_id = group_id;
						shmem->leaving_bar_size = people;
						shmem->bar_capacity += people;

						sem_post(&(shmem->leave_bar_request));
						sem_wait(&(shmem->leave_bar_answer));
						printf("\tGroup of %d just left the bar, ID = %d\n",people,group_id);
						(shmem->groups_left)--;
						(shmem->not_served)++;


						exit(0);
					}
				}
			}
			
			table_id = shmem->new_table_id;
			printf("\tGroup of %d moved from bar to table %d, ID = %d \n",people,table_id,group_id);
			shmem->total_capacity -= people;
			
			sem_post(&(shmem->queue));

			shmem->bar_capacity += people;			
		
			sem_wait(&(shmem->sem_customer_order));
			shmem->curr_id = group_id;
			
			sem_post(&(shmem->order_request));	
			sem_wait(&(shmem->order_answer));
			
			group_waiter = shmem->order_waiter;
			sem_post(&(shmem->sem_waiter));
			
			printf("\t\tGroup of %d ordered to waiter %d: , ID = %d \n",people,group_waiter,group_id);
			
			time_staying = rand()%period + 1;
			sleep(time_staying);

			printf("\t\tGroup of %d wants to pay waiter %d: , ID = %d \n",people,group_waiter,group_id);


			sem_wait(&(shmem->sem_customer_pay));
			shmem->curr_id = group_id;
			shmem->pay_waiter = group_waiter;
			
			sem_post(&(shmem->pay_request));
			sem_wait(&(shmem->pay_answer));

			printf("\t\tGroup of %d is pleased and is leaving now , ID = %d \n",people,group_id);

			sem_wait(&(shmem->sem_customer_exit));
			shmem->leaving_table = table_id;
			shmem->leaving_group = group_id;
			sem_post(&(shmem->exit_request));
			sem_wait(&(shmem->exit_answer));

			(shmem->served)++;
			shmem->total_capacity += people;
			(shmem->groups_left)--;
			exit(0);
		}
		else if((shmem->space_flag) == -1){
			printf("Group of %d leaving,restaurant is full, ID = %d\n",people,group_id);
			sem_post(&(shmem->queue));
			(shmem->groups_left)--;
			(shmem->not_served)++;

			exit(0);
		}
		else{
			perror("~~ Unexpected Behaviour ~~\n");
			exit(666);
		}


	(shmem->groups_left)--;
	exit(0);
}


