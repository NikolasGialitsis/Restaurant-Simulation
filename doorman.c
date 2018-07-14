/* D O O R M A N */

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
#include <time.h> // time()




int main(int argc,char* argv[]){

	if (argc < 5){
		perror("Not enough arguments given to function waiter \n");
		exit(1);
	}

	int i ;
	int max_time = 0;
	int shmid = 0 ;
	
	for ( i = 1 ; i < argc ; i++){
		if( strcmp(argv[i],"-d") ==0){
			max_time = atoi(argv[i+1]);
			i++; 
		}
		else if( strcmp(argv[i],"-s")== 0){
			shmid = atoi(argv[i+1]);
			i++;
		}
		else ;
	}

	Sh_Memory * shmem = (Sh_Memory*)shmat(shmid,0,0);
	if(shmem == (Sh_Memory*)(-1)){
		perror("Shared memory attachment");
	}
	Table* table= (Table *)((void*)shmem+sizeof(Sh_Memory));
	
	node* two_queue = NULL;
	node* four_queue = NULL;
	node* six_queue = NULL;
	node* eight_queue = NULL;

	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) { 
		perror("clock_gettime"); 
		exit(-1); 
	}

	ts.tv_sec += 5;




	srand(time(NULL) ^ (getpid()<<16));//for accurate seeding
	int service_time;
	int group_size;
	int group_id;
	int table_id;
	int table_size;
	int leaving_group;
	int bar_leaving_id;
	int bar_leaving_size;
	printf("Doorman summoned\n");
	while(shmem->groups_left != 0){


		if(sem_timedwait(&(shmem->exit_request),&ts)==0){
			ts.tv_sec +=5;
			table_id = shmem->leaving_table;
			leaving_group = shmem->leaving_group;
			table[table_id].occupied = 0;
			printf("Doorman:Table %d with capacity %d is now available,previously by group with ID = %d\n",table_id,table[table_id].space,leaving_group );
			
			/* 
				------------------------------------------------------------------------------
				Auto to kommati proorizotan gia tin metafora twn pelatwn ap to bar sta trapezia
				Kanontas remove front apo tin antistoixi oura pairnw to ID tou pelati me tin megaluteri
				proteraiotita.(first id)

				Antistoixa o customer elegxei ana takta diastimata an to ID tou sumfwnei me to first ID.
				An nai tote kserei oti kathetai sto analogo trapezi 

				To exw afisei se sxolia dioti oi diergasies pelati kai waiter den termatizoun pote
				-----------------------------------------------------------------------------


			int size = table[table_id].space;
			int first_id = -1;
			

			if(size <= 2){
				Remove_Front(two_queue,&first_id);
				table[table_id].occupied = 1;	
			}
			else if(size <= 4){
				Remove_Front(four_queue,&first_id);
				table[table_id].occupied = 1;
			}
			else if(size <= 6){
				Remove_Front(six_queue,&first_id);
				table[table_id].occupied = 1;
			}
			else if(size <= 8){
				Remove_Front(eight_queue,&first_id);
				table[table_id].occupied = 1;
			}
			else{
				perror("Unexpected Behaviour");
				exit(666);
			}
			shmem->first_customer = first_id;
			shmem->new_table_id = table_id;
			*/
			sem_post(&(shmem->sem_customer_exit));
			sem_post(&(shmem->exit_answer));

		}		
		ts.tv_sec +=5;
		
		if(sem_timedwait(&(shmem->leave_bar_request),&ts)==0){
			ts.tv_sec +=5;
			bar_leaving_size = shmem->leaving_bar_size;
			bar_leaving_id = shmem->leaving_bar_id;
			printf("Doorman:Sadly group of size %d with ID = %d  is leaving bar\n",bar_leaving_size,bar_leaving_id);
			/*
			-------------------------------------------------------------------------------------------
			 Auto to kommati proorizotan stin diagrafi twn group ID apo tin antistoixi oura tou bar
			Otan ena group feugei apo to bar tote kanei P(leave_bar_request) kai pernaei tis aparaitites
			plirofories , leaving_bar_size kai leaving_bar id ston doorman.Gia auto to logo xrisimopoiw 
			enan akomi semaforo ton exit_bar opou eksasfalizei mi tautoxroni prosvasi pelatwn stin koini mnimi
			(kai ara tropopoihsh twn pliroforiwn size kai id)

			To exw afisei se sxolia dioti oi diergasies doorman kai waiter den termatizoun pote
			-------------------------------------------------------------------------------------------------



			if(bar_leaving_size <= 2){
				Delete_Node(two_queue,bar_leaving_id);
			}
			else if(bar_leaving_size <= 4){
				Delete_Node(four_queue,bar_leaving_id);
			}
			else if(bar_leaving_size <= 6){
				Delete_Node(six_queue,bar_leaving_id);
			}
			else if(bar_leaving_size <= 8){
				Delete_Node(eight_queue,bar_leaving_id);
			}*/


			sem_post(&(shmem->exit_bar));
			sem_post(&(shmem->leave_bar_answer));
		}

		if(sem_timedwait(&(shmem->entry_request),&ts)== 0 ){
			ts.tv_sec+=5;
			group_size = shmem->group_size;
			group_id = shmem->customer_id;
			sem_post(&(shmem->sem_customer_enter));
				
			printf("Doorman is informed that the incoming group's size is %d, ID = %d\n",group_size,group_id);

			service_time = rand()%max_time + 1;
			sleep(service_time);

			/* Try finding a table for the group*/
			shmem->space_flag = 0;
			if(group_size <= shmem->total_capacity){
				int i;
				for( i = 0 ; i < shmem->num_tables ; i++){
					if((table[i].occupied == 0)&&((table[i].space) >= group_size)){
						printf("Doorman:please sit on table %d with %d seats, ID = %d\n",table[i].id,table[i].space,group_id);
						shmem->space_flag = 1;
						table[i].occupied = 1;
						shmem->table_id = i;
						break;
					}
				}
			}


			node* temp;

			if((shmem->space_flag) == 0){
				printf("Doorman:there is no available table\n");
				printf("bar capacity = %d\n",shmem->bar_capacity);
				if((group_size)<=(shmem->bar_capacity)){
					printf("Doorman:please wait at the bar\n");
					if (shmem->group_size <= 2){
						if( two_queue == NULL){
							two_queue = malloc(sizeof(node*));
							if(two_queue == NULL){
								perror("memory allocation error\n");
							}
							two_queue->group_id = group_id;
							two_queue->next = NULL;						
						}
						else{
							temp = Insert_Back(two_queue);
							temp->next = NULL;
							temp->group_id = group_id;
						}
						printf("Groups waiting for a size 2 table:");
						Print_List(two_queue);
					}
					else if(group_size <= 4){
						if( four_queue == NULL){
							four_queue = malloc(sizeof(node*));
							if(four_queue == NULL){
								perror("memory allocation error\n");
							}
							four_queue->group_id = group_id;
							four_queue->next = NULL;							
						}
						else{
							temp = Insert_Back(four_queue);
							temp->next = NULL;
							temp->group_id = group_id;
						}
						printf("Groups waiting for a size 4 table:");
						Print_List(four_queue);
					}
					else if(group_size <= 6){
						if( six_queue == NULL){
							six_queue = malloc(sizeof(node*));
							if(six_queue == NULL){
								perror("memory allocation error\n");
							}
							six_queue->group_id = group_id;
							six_queue->next = NULL;							
						}
						else{
							temp = Insert_Back(six_queue);
							temp->next = NULL;
							temp->group_id =  group_id;
							
						}
						printf("Groups waiting for a size 6 table:");
						Print_List(six_queue);
					}
					else if(group_size <= 8){
						if( eight_queue == NULL){
							eight_queue = malloc(sizeof(node*));
							if(eight_queue == NULL){
								perror("memory allocation error\n");
							}
							eight_queue->group_id = group_id;
							eight_queue->next = NULL;
						}
						else{
							temp = Insert_Back(eight_queue);
							temp->next = NULL;
							temp->group_id =  group_id;
							
						}
						printf("Groups waiting for a size 8 table:");
						Print_List(eight_queue);
					}

				}
				else{
					printf("Doorman:sorry restaurant is full\n");
					shmem->space_flag = -1;
				}
			}
			sem_post(&(shmem->entry_answer));		
		}
		ts.tv_sec +=5;
	}
	printf("Doorman has finished working\n");
	Delete_List(two_queue);
	Delete_List(four_queue);
	Delete_List(six_queue);
	Delete_List(eight_queue);
	exit(0);
}


node* Insert_Back(node* header){
	node* temp = header;
	while((temp ->next) != NULL){
		temp = temp->next;
	}
	temp->next =(node*)malloc(sizeof(node));
	if(temp == NULL){
		perror("Error allocating memory\n");
	} 
	return temp->next;
}

node* Remove_Front(node* header,int* id){
	node* temp = header;
	*id = temp->group_id; 
	header = header->next;
	free(temp);
	return header;
}

void Print_List(node* header){
	node* temp = header;
	while(temp != NULL){
		printf("id = %d , ",temp->group_id );
		temp = temp->next;
	}
	printf("\n");
}


void Delete_Node(node* header,int id){
	node* temp = header;
	node* next_ptr;
	int flag = 0;
	while(temp != NULL){
		if((temp->next)->group_id == id){
			
			next_ptr = (temp->next)->next;
			free(temp->next);
			temp->next = next_ptr;
			return;
		}
	}
	if(flag == 0){
		printf("I could not find node with id = %d\n",id);
	}
}


void Delete_List(node* header){
	node* temp = header;
	node* temp_del;
	while(temp != NULL){
		temp_del = temp;
		temp = temp->next;
		free(temp_del);
	}
}