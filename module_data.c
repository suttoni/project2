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
	if(elevator.state == STOPPED || elevator.state == IDLE){
		printk(KERN_DEBUG "Elevator Starting.");
		elevator.state = LOADING;
		elevator.currentFloor = 1;
		elevator.usedSpace = 0;
		elevator.usedWeightUnit = 0;
		elevator.continueRun = 1;
		mutex_unlock(&elevatorLock);
		elevatorThread = kthread_run(elevator_service, NULL, "elevator");

		if( IS_ERR(elevatorThread) != 0){
			printk(KERN_ERR "Elevator failed to create elevatorThread.");
			result = -1;
		}

		else
			return 0;
	}

	else
		result = 0;

	mutex_unlock(&elevatorLock);

	return result;
}

extern long (*STUB_issue_request)(int,int,int);
long issue_request(int pass_type, int start_floor, int desired_floor)
{
	if(((pass_type == 0) || (pass_type == 1) || (pass_type == 2) || (pass_type == 3))
		&& ((start_floor >= 1) && (start_floor <= 10))
		&& ((desired_floor >= 1) && (desired_floor <= 10))){

		struct passenger_info *passenger = NULL;
		passenger = kmalloc(sizeof(struct passenger_info), GFP_KERNEL);
		passenger->passengerType = pass_type;
		passenger->currFloor = start_floor;
		passenger->destFloor = desired_floor;
		INIT_LIST_HEAD(&passenger->passengerList);

		//Add the passenger
		mutex_lock(&elevatorLock);
		list_add_tail(&passenger->passengerList, &floors[passenger->currFloor - 1].queue);
		floors[passenger->currFloor - 1].queueWaiting++;
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

	mutex_lock(&elevatorLock);
	if(elevator.state == STOPPED || elevator.state == IDLE)
		result = 1;

	elevator.continueRun = 0;
	
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
