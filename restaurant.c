/* R E S T A U R A N T */
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


int main(int argc , char* argv[]){
	
	int i = 0;
	char* config_name = NULL;
	int rest_time = 0;
	int customers = 0;

	if(argc < 7){
		perror("not enough arguments to function restaurant\n");
		exit(1);
	}


	for ( i = 1 ; i < argc ; i++){

		if (strcmp(argv[i],"-l") == 0){
			config_name = argv[i+1];
			printf("configuration file is %s\n",config_name);
			i++;
		}
		else if(strcmp(argv[i],"-n")==0){
			customers = atoi(argv[i+1]);
			printf("groups of customers expected = %d \n",customers);
			i++;
		}
		else if(strcmp(argv[i],"-d")==0){
			rest_time = atoi(argv[i+1]);
			printf("time for Statistics = %d\n",rest_time);
			i++;
		}
		else{
			perror("arguments given do not agree to the current format \n");
			exit(1);
		}
	}

	FILE* fp;
	fp = fopen(config_name,"r");//open configuration file with read-only privilages
	if(fp == NULL){
		perror("Unable to open configuration file\n");
		exit(1);
	}

	/*I N I T I A L I A Z A T I O N */

	int num_waiters = 0;
	int bar_capacity = 0;
	int num_tables = 0;
	int* table_capacities = NULL;

    /*Buffer for parsing the configuration file*/
	char* config_data = malloc(100*sizeof(char));
	if (config_data == NULL){
		perror("Memory allocation error\n");
		exit(2);
	}


	config_data = fgets(config_data,100,fp);
	if(config_data == NULL){
		perror("fgets error:string empty\n");
		exit(3);
	}
	num_waiters = atoi(config_data);

	config_data = fgets(config_data,100,fp);
	if(config_data == NULL){
		perror("fgets error:string empty\n");
		exit(3);
	}
	bar_capacity = atoi(config_data);

	config_data = fgets(config_data,100,fp);
	if(config_data == NULL){
		perror("fgets error:string empty\n");
		exit(3);
	}
	num_tables = atoi(config_data);

	printf("num_waiters = %d\n",num_waiters);
	printf("bar_capacity = %d\n",bar_capacity );
	printf("num_tables = %d\n",num_tables );

	table_capacities = malloc(num_tables*sizeof(int));
	if(table_capacities == NULL){
		perror("Memory allocation error\n");
		exit(2);
	}

	config_data = fgets(config_data,100,fp);
	if(config_data == NULL){
		perror("fgets error:string empty\n");
		exit(3);
	}

	/*Split line into integers */
	i = 0;
	char* token = strtok(config_data," ");
	while ( token != NULL && i < num_tables){
		table_capacities[i++] = atoi(token);
		token = strtok(NULL," ");
	}

	int total_capacity = 0;
	for ( i = 0 ; i < num_tables ; i++){
		total_capacity += table_capacities[i];
	}

	fclose(fp);

	int shmid;
	int err; 


	/*Create Shared Memory Segment*/

	shmid = shmget(IPC_PRIVATE, sizeof(Sh_Memory) + (num_tables*sizeof(Table)) , 0666);
	if (shmid == -1)
		perror("shmget error");
	else
		;//printf("New Shared memory's id is %d\n",shmid);

	/*Shared memory Attachment*/

	Sh_Memory * shmem = (Sh_Memory*)shmat(shmid,0,0);
	if(shmem == (Sh_Memory*)(-1)){
		perror("Shared memory attachment");
	}
	Table* table= (Table *)((void*)shmem+sizeof(Sh_Memory));

	/* For Displaying results after specific number of seconds */




	/*Initialize shared segment and semaphores */

	shmem->num_tables = num_tables;
	shmem->total_capacity = total_capacity;
	shmem->space_flag = 0;
	shmem->bar_capacity = bar_capacity;
	shmem->customer_id = -1;
	shmem->leaving_group = -1;
	shmem->waiter_id = -1;
	shmem->groups_left = customers;
	shmem->order_waiter = -1;
	shmem->group_size = -1;
	shmem->curr_id = -1;
	shmem->income = 0;
	shmem->served = 0;
	shmem->not_served = 0;
	shmem->leaving_table = -1;
	shmem->leaving_bar_id = -1;
	shmem->leaving_bar_size = -1;
	shmem->first_customer = -1;
	shmem->new_table_id = -1;
	

	for(i = 0 ; i < num_tables ; i++){
		table[i].id = i;
		table[i].space = table_capacities[i];
		table[i].occupied = 0;
		printf("table[%d].space = %d\n",i,table[i].space ); 
	}


	err = sem_init(&(shmem->entry_request),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->entry_answer),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->queue),1,1);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->table_flag),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->order_request),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->order_answer),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->sem_waiter),1,1);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->sem_customer_order),1,1);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->sem_customer_pay),1,1);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->sem_customer_enter),1,1);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->pay_request),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->pay_answer),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->sem_customer_exit),1,1);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->exit_request),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->exit_answer),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->leave_bar_request),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->leave_bar_answer),1,0);
	if(err == -1){
		perror("sem init failed\n");
	}
	err = sem_init(&(shmem->exit_bar),1,1);
	if(err == -1){
		perror("sem init failed\n");
	}

	pid_t pid;
	
	pid = fork();
	if(pid == 0){
		sleep(rest_time);
		printf("\n------- Restaurant Statistics -------\n");
		printf("Waiters = %d , number of tables = %d , bar capacity = %d\n",num_waiters,num_tables,bar_capacity);
		printf("Total number of people sitting in tables = %d:\n",total_capacity-shmem->total_capacity);
		printf("Total number of people in bar = %d\n",bar_capacity - shmem->bar_capacity);
		printf("Total Income:%d\n",shmem->income);
		printf("Groups served:%d\n",shmem->served );
		printf("Groups not served:%d\n",shmem->not_served);
		printf("---------------------------------------\n");
		exit(0);

	}

	srand(time(NULL) ^ (getpid()<<16));//for accurate seeding
	/* C R E A T E   P R O C E S S E S */

	


	/*Create Doorman */
	pid = fork();
	if (pid == 0){

		free(table_capacities);
		free(config_data);

		char str_shmid[15];
		char str_time[15];
		sprintf(str_shmid, "%d", shmid);
		sprintf(str_time , "%d", doorman_time);
		char* args[] = {"./doorman","-d",str_time,"-s",str_shmid,NULL};
		execv(args[0],args);
		
		perror("return not expected :exec error\n");
		exit(-1);

	}
	
	/*Create Waiters*/

	for( i = 0 ; i < num_waiters ; i++){
		pid = fork();
		if(pid == 0){
			free(table_capacities);
			free(config_data);
			
			char str_shmid[15];
			char str_period[15];
			char str_money[15];
			sprintf(str_shmid, "%d", shmid);
			sprintf(str_period , "%d", waiter_period);
			sprintf(str_money , "%d",max_order);


			char* args[] = {"./waiter","-m",str_money,"-d",str_period,"-s",str_shmid,NULL};
			execv(args[0],args);
			
			perror("return not expected :exec error\n");
			exit(-1);

		}		
	}

	/* Create groups of customers */ 


	int nsize = 0;
	for( i = 0 ; i < customers ; i++){
		sleep(rand()%arrival_interval + 1);//the rate at which the groups will be arriving at the entrance	
		pid = fork();
		if(pid == 0){
			free(table_capacities);
			free(config_data);
			srand(time(NULL) ^ (getpid()<<16));//for accurate seeding
			nsize = rand()%8 + 1;
			char str_shmid[15];
			char str_period[15];
			char str_group_size[15];
			sprintf(str_shmid, "%d", shmid);
			sprintf(str_period , "%d", customer_period);
			sprintf(str_group_size , "%d",nsize);


			char* args[] = {"./customer","-n",str_group_size,"-d",str_period,"-s",str_shmid,NULL};
			execv(args[0],args);
			
			perror("return not expected :exec error\n");
			exit(-1);

		}		
	}



	/* D E L E T I O N */
	while (wait(NULL) > 0 );

	free(table_capacities);
	free(config_data);

	err = shmctl(shmid, IPC_RMID , 0); //mark the segment to be destroyed when everything detaches
	if (err == -1){
		perror("Error deleting shared memory segment");
	}
	else{
		printf("Removed shared memory with id %d\n",shmid);
	}

	err = sem_destroy(&(shmem->entry_request));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->entry_answer));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->queue));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->table_flag));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->order_request));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->order_answer));
	if(err == -1){
		perror("sem_destroyfailed\n");
	}
	err = sem_destroy(&(shmem->sem_waiter));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->sem_customer_order));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->sem_customer_pay));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->sem_customer_enter));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->pay_request));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->pay_answer));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->sem_customer_exit));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->exit_request));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->exit_answer));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->leave_bar_answer));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->leave_bar_request));
	if(err == -1){
		perror("sem_destroy failed\n");
	}
	err = sem_destroy(&(shmem->exit_bar));
	if(err == -1){
		perror("sem_destroy failed\n");
	}

	printf("Restaurant terminated\n");
		
	printf("------- Restaurant Statistics -------\n");
	printf("Waiters = %d , number of tables = %d , bar capacity = %d\n",num_waiters,num_tables,bar_capacity);
	printf("Total number of people sitting in tables = %d:\n",total_capacity-shmem->total_capacity);
	printf("Total number of people in bar = %d\n",bar_capacity - shmem->bar_capacity);
	printf("Total Income:%d\n",shmem->income);
	printf("Groups served:%d\n",shmem->served );
	printf("Groups not served:%d\n",shmem->not_served);
	printf("---------------------------------------\n");
	exit(0);

}
