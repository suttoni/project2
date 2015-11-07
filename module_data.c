#include "module_data.h"
#include <syscalls.h>
#include <linux/printk.h>

/*create elevator*/
extern struct mutex elevatorLock;
extern struct mutex floorLock;
extern struct passenger_info passenger;
extern struct floor_info floors[NUM_FLOORS];
struct task_struct *elevatorThread;

extern long (*STUB_start_elevator)(void);
long start_elevator(void)
{
	int result = 0;

	mutex_lock(&elevatorLock);
	if(elevator.state == STOPPED){
		printk(KERN_DEBUG "Elevator Starting.");
		//Set elevator's state to IDLE
		elevator.state = IDLE;
		//Starting floor is set to 1
		elevator.currentFloor = 1;
		//Empty elevator!
		elevator.usedSpace = 0;
		//Empty elevator!
		elevator.usedWeightUnit = 0;
		//Set continueRun == 1 so we know we're supposed to be running
		elevator.continueRun = 1;
		//Unlock the elevator lock
		mutex_unlock(&elevatorLock);
		//Creating a kthread that runs elevator_service function from elevator_passenger.c
		elevatorThread = kthread_run(elevator_service, NULL, "elevator");

		//Checking for errors
		if( IS_ERR(elevatorThread) != 0){
			printk(KERN_ERR "Elevator failed to create elevatorThread.");
			result = -1;
		}

		//No errors
		else
			return 0;
	}

	//Elevator is currently running, no need to start again
	else
		result = 1;

	//Unlock elevator lock
	mutex_unlock(&elevatorLock);

	return result;
}

extern long (*STUB_issue_request)(int,int,int);
long issue_request(int pass_type, int start_floor, int desired_floor)
{
	//Standard checking to make sure all data types are legal
	if(((pass_type == 0) || (pass_type == 1) || (pass_type == 2) || (pass_type == 3))
		&& ((start_floor >= 1) && (start_floor <= 10))
		&& ((desired_floor >= 1) && (desired_floor <= 10))){

		//Create a new passenger
		struct passenger_info *passenger = NULL;
		//Allocating memory for that passenger
		//Flag 'GFP_KERNEL' makes sure the kernel will do 
		//whatever is necessary to obtain memory requested
		passenger = kmalloc(sizeof(struct passenger_info), GFP_KERNEL);
		//Setting the new passenger's type == the type given
		passenger->passengerType = pass_type;
		//Setting the new passenger's current floor == the start floor that was given
		passenger->currFloor = start_floor;
		//Same as above only setting the new passenger's desired floor
		passenger->destFloor = desired_floor;
		//Initializing the new passenger's list_head 'passengerList'
		INIT_LIST_HEAD(&passenger->passengerList);

		//Lock the elevator, add the passenger
		mutex_lock(&elevatorLock);
		list_add_tail(&passenger->passengerList, &floors[passenger->currFloor - 1].queue);
		//Increment the floor's queue of passengers waiting to use elevator
		floors[passenger->currFloor - 1].queueWaiting++;
		//Unlock elevator
		mutex_unlock(&elevatorLock);
		
		printk(KERN_ALERT "Request issued.");
		return 0;
	}

	else{
		printk(KERN_ALERT "Invalid passenger information.");
	}

	return 1;
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void)
{
	int result = 0;

	printk(KERN_DEBUG "Stopping Elevator.");

	//Lock elevator
	mutex_lock(&elevatorLock);
	//If elevator is already stopped, return a 1 
	if(elevator.state == STOPPED)
		result = 1;

	//Tell elevator to stop running
	elevator.continueRun = 0;
	
	//Unlock elevator
	mutex_unlock(&elevatorLock);

	return result;
}


void elevator_syscalls_create(void) {
	STUB_start_elevator =& (start_elevator);
	STUB_stop_elevator =& (stop_elevator);
	STUB_issue_request =& (issue_request);
}

void elevator_syscalls_remove(void) {
	STUB_start_elevator = NULL;
	STUB_stop_elevator = NULL;
	STUB_issue_request = NULL;
}
