/* Main data header file - this contains all the necessary information 
   for elevator_module.c, *insert other files as created* 
   Author: Ian Sutton
   FSUID: iss13
   Team mates: Yilin Wang, Ibrahim Atiya, Sai Gunasegaran 
*/

#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sutton");
MODULE_DESCRIPTION("Data file that contains elevator information");

//Define restrictions: total number of floors is 10, 
//max passengers is 8, max weight is 8

#define NUM_FLOORS 10
#define MAX_PASSENGERS 8
#define MAX_LOAD 8

/* 
   Enumeration for the current elevator state.
   The elevator can be IDLE, moving UP or DOWN,
   loading passengers or stopped.
   The state of the elevator is determined in 
   the "show_elevator_data" function that is defined
   in elevator_module.c
*/
typedef enum{
	IDLE = 0;
	UP = 1;
	DOWN = 2;
	LOADING = 3;
	STOPPED = 4;
} elev_movement_state;

/* 
   The following structs are used for essential
   information regarding the elevator itself,
   passengers (i.e which floor they're on, where
   they wish to go, passenger type: adult, child etc),
   as well as floor info (i.e, how many passengers are
   waiting to get on the floor, how many passengers have been
   served)
*/

struct elevator_info{
	elev_movement_state state;
	int currentFloor;
	int destinationFloor;
	int usedSpace;
	bool continueRun;
	struct list_head passengers;
};

struct passenger_info{
	int currFloor;
	int destFloor;
	char passengerType;
	struct list_head passengerList;
};

struct floor_info{
	int numPassServed;
	int queueWaiting;
	struct list_head queue;
};

/* Have yet to create a file to contain definitions for these */
long start_elevator(void);
long issue_request(char pass_type, int start_floor, int desired_floor);
long stop_elevator(void);

/* Have yet to create a file to contain definitions for these */
int remove_passengers(void);
int add_passengers(void);
int elevator_service(void * info);

/* Function definitions are located in elevator_module.c */
int show_elevator_data(struct seq_file *m, void *v);
int open_elevator(struct inode *inode, struct file *file);
int __exit exit_elevator(void);
int __init init_elevator(void);
