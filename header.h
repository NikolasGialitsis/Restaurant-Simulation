/* H E A D E R */

#ifndef HEADER_H
#define HEADER_H

	#define arrival_interval 5
	#define doorman_time 10
	#define customer_period 10
	#define max_order 200
	#define waiter_period 10

	typedef struct Table {
   		int id;
   		int space;
   		int occupied;
	}Table;


/*		S H A R E D   M E M O R Y	*/ 

	typedef struct Sh_Memory {
   		
   		Table* table ;
    	
    	sem_t queue; // entrance queue
    	sem_t entry_request;// by customer
    	sem_t entry_answer; //by doorman
    	
    	sem_t table_flag;//if there is a free table or not

    	sem_t order_request;//by customer
    	sem_t order_answer;//by waiter

    	sem_t pay_request; // by customer
    	sem_t pay_answer; //by waiter

    	sem_t sem_waiter;//for sync between waiters
    	

    	sem_t sem_customer_enter; //for sync between customers and doorman 
    	sem_t sem_customer_order; //for sync between customers and waiter
    	sem_t sem_customer_pay;//for sync between customers and waiter
    	sem_t sem_customer_exit;//for sync between customers and doorman
    	sem_t exit_bar; //for moving customers from bar to tables



    	sem_t exit_request;//by customer
    	sem_t exit_answer;//by doorman

    	int waiter_id;//unique identifier
    	int order_waiter;//id of the waiter currently taking an order
    	int pay_waiter;//id of the waiter that has to get the check

    	
    	int leaving_table;//id of the table that got recently available
    	/*BAR variables */
    	
    	sem_t leave_bar_request;//by customer
    	sem_t leave_bar_answer;//by doorman
    	int first_customer;//bar: customer ID with highest priority
    	
    	int leaving_bar_id;//ID of group leaving the bar
    	int leaving_bar_size;//size of the group leaving bar
		   
		int leaving_group;//ID of customer ID leaving table 	

    	int curr_id;//id of group of customers taking order or paying
    	int customer_id; //unique group identifier
    	
    	

    	int num_tables; //input num of tables
    	int bar_capacity;//input bar capacity
    	int group_size; //size of group entering restaurant
    	int total_capacity;//# of people currently sitting
    	int space_flag; 
    	
    	int table_id; //current table

    	int income;//total money received
    	int served;//number of groups served
    	int not_served;//number of groups not served
    	int groups_left;//total groups that have left the restaurant

    	int new_table_id;//for moving a group from bar to a new table


	
	}Sh_Memory;


	typedef struct node{
		struct node* next;
		int group_id;
	}node;


	node* Insert_Back(node*);
	node* Remove_Front(node*,int*);
	void Print_List(node*);
	void Delete_List(node*);
	void Delete_Node(node*,int);
#endif