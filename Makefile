CC=gcc
OBJ=restaurant	waiter 	restaurant	doorman	customer
CFLAGS=-g3	-pthread


restaurant:	waiter 	doorman	restaurant.c 	customer
	$(CC)	$(CFLAGS)	restaurant.c 	-o 	restaurant

waiter:	waiter.c
	$(CC)	$(CFLAGS)	waiter.c 	-o 	waiter

doorman:	doorman.c
	$(CC)	$(CFLAGS)	doorman.c 	-o 	doorman

customer:	customer.c
	$(CC)	$(CFLAGS)	customer.c 	-o 	customer

clean:
	rm	$(OBJ)