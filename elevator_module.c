/* This initializes the elevator, exits the elevator, performs procfs entry 
   by using function show_elevator_data which uses seq_printf to print to proc file
   Author: Ian Sutton
   Team mates: Yilin Wang, Sai Gunesagaran, Ibrahim Atiya 
*/
#include "module_data.h"
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sutton");
MODULE_DESCRIPTION("Module for elevator scheduler");

int deliveredAdults,
	deliveredChildren,
	deliveredBellhops,
	deliveredRoomService;

extern struct inode *inode; 
extern struct file *file;

struct mutex elevatorLock = __MUTEX_INITIALIZER(elevatorLock);
struct mutex floorLock = __MUTEX_INITIALIZER(floorLock);

/* 
	Stubs - we get these from our syscall file elevator_syscalls.c and
	are implemented in module_data.h
*/

extern int(* STUB_issue_request)(int pass_type, int start_floor, int desired_floor);
extern int(* STUB_start_elevator)(void);
extern int(* STUB_stop_elevator)(void);

int elevator_open(struct inode *inode, struct file *file){
	return single_open(file, show_elevator_data, NULL);
}

//Struct for file_operations
const struct file_operations elevator_fops = {
	.owner = THIS_MODULE,
	.open = elevator_open,
	.read = seq_read,
	.release = single_release,
};

/* Module function definitions */

int __init init_elevator(void){
	int i = 0;

	STUB_issue_request = &issue_request;
	STUB_start_elevator = &start_elevator;
	STUB_stop_elevator = &stop_elevator;

	deliveredAdults = 0;
	deliveredChildren = 0;
	deliveredBellhops = 0;
	deliveredRoomService = 0;

	mutex_init(&elevatorLock);
	mutex_init(&floorLock);

	mutex_lock(&elevatorLock);
	elevator.state = IDLE;
	elevator.currentFloor = 1;
	elevator.usedSpace = 0;
	INIT_LIST_HEAD(&elevator.passengers);
	mutex_unlock(&elevatorLock);

	mutex_lock(&floorLock);
	for(i = 0; i < NUM_FLOORS; i++){
		floors[i].queueWaiting = 0;
		floors[i].numPassServed = 0;
		INIT_LIST_HEAD(&floors[i].queue);
	}
	mutex_unlock(&floorLock);

	proc_create("elevator", 0, NULL, &elevator_fops);
	return 0;
}

void __exit exit_elevator(void){

	STUB_issue_request = NULL;
	STUB_start_elevator = NULL;
	STUB_stop_elevator = NULL;

	mutex_lock(&elevatorLock);
	elevator.continueRun = false;
	mutex_unlock(&elevatorLock);

	while(1){
		//Wait for elevator to stop before removing module
		mutex_lock(&elevatorLock);

		if(elevator.state == IDLE){
			mutex_unlock(&elevatorLock);
			break;
		}
		mutex_unlock(&elevatorLock);
		msleep(1000);
	}

	mutex_destroy(&elevatorLock);
	mutex_destroy(&floorLock);

	remove_proc_entry("elevator", NULL);
}

int show_elevator_data(struct seq_file *m, void *v){
	/* Standard variable declarations */
	//Ints for keeping track of passengers and iteration variable i
	int adults = 0;
	int children = 0;
	int bellhops = 0;
	int roomService = 0;
	int i = 0;

	//char to check on state of elevator (i.e IDLE or UP)
	char idle[5] = "IDLE";
	char up[3] = "UP";
	char down[5] = "DOWN";
	char loading[8] = "LOADING";
	char stopped[8] = "STOPPED";
	char *stateOfElevator;

	//structs for the list_head position and passenger type
	struct list_head *position;
	struct passenger_info *info;

	//mutex_locks for the elevator and floor
	mutex_lock(&elevatorLock);
	mutex_lock(&floorLock);

	//Switch statement to determine the state of the elevator
	switch (elevator.state){
			case IDLE:
				stateOfElevator = idle;
				break;
			case UP:
				stateOfElevator = up;
				break;
			case DOWN:
				stateOfElevator = down;
				break;
			case LOADING:
				stateOfElevator = loading;
				break;
			case STOPPED:
				stateOfElevator = stopped;
				break;
			default:
				stateOfElevator = idle;
				break;
	}

	//Print the state of the elevator
	seq_printf(m, "Elevator's Status: %s\n", stateOfElevator);
	
	/* 
		Iterate through every passenger on elevator's passenger list
		using the function "list_for_each", perform a check on 
		passenger type using a switch statement,
		increment number of certain type of passenger.
	*/
	
	list_for_each(position, &elevator.passengers){
		info = list_entry(position, struct passenger_info, passengerList);
		switch (info->passengerType){
			case 0:
				adults++;
				break;
			case 1:
				children++;
				break;
			case 2:
				bellhops++;
				break;
			case 3:
				roomService++;
				break;
		}
	}

	//Print the floor that the elevator is currently located at
	seq_printf(m, "Our current floor is: %d\n", elevator.currentFloor);

	//Print the destination floor
	seq_printf(m, "Our destination is: %d\n", elevator.destinationFloor);
	
	//Print the current number of passengers in elevator of each type
	seq_printf(m, "Our passengers are: %d (%d adults, %d children, %d bellhops, %d room service)\n",
			  (adults + children + bellhops + roomService), adults, children, bellhops, roomService);

	/* 
		Print total number of waiting passengers at each floor and number of passengers delivered.
		To do so, we need a for loop to iterate through every floor and check the number of 
		passengers waiting at each floor as well as those that have been delivered (numPassServed).
	*/

	for(i = 0; i < NUM_FLOORS; i++){
		adults = 0;
		children = 0;
		bellhops = 0;
		roomService = 0;

		seq_printf(m, "Floor %d:", i+1);

		//Again, use "list_for_each" to iterate through the list
		list_for_each(position, &floors[i].queue){
			info = list_entry(position, struct passenger_info, passengerList);
			switch (info->passengerType){
				case 'A':
					adults++;
					break;
				case 'C':
					children++;
					break;
				case 'B':
					bellhops++;
					break;
				case 'R':
					roomService++;
					break;
			}		
		}
		seq_printf(m, "\t%d adults, %d children, %d bellhops, %d room service are waiting. %d passengers have been delivered.\n",
				  adults, children, bellhops, roomService, floors[i].numPassServed);
	}

	mutex_unlock(&elevatorLock);
	mutex_unlock(&floorLock);

	return 0;
}
	
module_init(init_elevator);
module_exit(exit_elevator);
