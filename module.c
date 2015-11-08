

#include "module.h"

// Mutexi (latin for multiple mutex)
struct mutex activateElevatorLock;
struct mutex deactivateElevatorLock;
struct mutex requestLock;
struct mutex accessLock;

//Here we have a task_struct that is used to create threads
struct task_struct *elevatorThread;

//Our elevator struct
struct elevator_info elevator;

//Two static variables used to keep track of whether or not the elevator is active
static int act = 0;
static int deact = 0;

//Elevator open for the proc entry
int elevator_open(struct inode *inode, struct file *file){
	return single_open(file, elevator_proc_entry, NULL);
}

//Our file_operations declaration to be used by elev_proc_entry
const struct file_operations elevator_fops = {
	.owner = THIS_MODULE,
	.open = elevator_open,
	.read = seq_read,
	.release = single_release,
};

// Extended definition of the original syscalls provided by Mr. Dennis
extern long (*STUB_start_elevator)(void);
long start_elevator(void) {
	int result = 1;

	mutex_lock (&activateElevatorLock);
	//If elevator isn't active - set result to 0
	//and then set act variable to 1, indicating 
	//elevator is now active
	if(act == 0){
		result = 0;
		if(!act)
			elevator.state = IDLE;	
		act = 1;
	}

	mutex_unlock(&activateElevatorLock);
	
	return result;
}

// Extended definition of the original syscalls provided by Mr. Dennis
extern long (*STUB_issue_request)(int,int,int);
long issue_request(int pass_type, int start_floor, int destination_floor) {

	//Initialize our variables
	int weight;
	passenger_type passType;
	struct passenger_info *passenger;

	//Simple check to make sure our floors are within bounds
	//Then we set up the passenger variables
	if((start_floor > 0) && (start_floor < 11) && (destination_floor > 0) && (destination_floor < 11)){
		
		switch(pass_type){
			case 0:
				passType = ADULT;
				weight = 2;
				break;
			case 1:
				passType = CHILD;
				weight = 1;
				break;
			case 2:
				passType = BELLHOP;
				weight = 4;
				break;
			case 3:
				passType = ROOMSERVICE;
				weight = 4;
				break;
			default:
				return 1;
		}//end switch

		//Add a passenger to floor's waiting queue
		mutex_lock(&requestLock);
		//Allocate memory for the new passenger
		passenger = kmalloc(sizeof(struct passenger_info), __GFP_FS);
		//Set up the passengers information
		passenger->passType = passType;
		passenger->weight = weight;
		passenger->currentFloor = destination_floor-1;

		//lock up the access lock
		mutex_lock(&accessLock);
		//Add the passenger to the waiting queue of struct floor_info
		list_add_tail(&(passenger->list), &(elevator.floors[start_floor-1].waitQueue.list));
		//Increment the number of folks waiting for the elevator on that floor
		(elevator.floors[start_floor-1].waitingNum)++;
		mutex_unlock(&accessLock);

		mutex_unlock(&requestLock);
	}//end if
	return 0;
}

// Extended definition of the original syscalls provided by Mr. Dennis
extern long (*STUB_stop_elevator)(void);
long stop_elevator(void) {

	mutex_lock (&activateElevatorLock);
	if(act == 0)
		return 0;
	mutex_unlock (&activateElevatorLock);

	//Get ready to deactivate elevator
	mutex_lock (&deactivateElevatorLock);

	//If elevator isn't deactivated - change state to
	//STOPPED and then set the deact variable to 1 to
	//indicate that elevator is deactivated
	if(deact == 0){
		deact = 1;
		elevator.state = STOPPED;
	}

	//If elevator is already deactivated - unlock and return 1
	else if(deact == 1){
		mutex_unlock(&deactivateElevatorLock);
		return 1;
	}

	mutex_unlock(&deactivateElevatorLock);

	mutex_lock (&activateElevatorLock);
	mutex_lock (&deactivateElevatorLock);
	
	//Set both deact and act variables to 0, indicated
	//elevator is neither active, nor deactive
	act = 0;
	deact = 0;

	mutex_unlock (&activateElevatorLock);
	mutex_unlock (&deactivateElevatorLock);

	return 0;
}

// Create syscalls 
void elevator_syscalls_create(void) {
	STUB_start_elevator =& (start_elevator);
	STUB_issue_request =& (issue_request);
	STUB_stop_elevator =& (stop_elevator);
}

// Remove our syscalls
void elevator_syscalls_remove(void) {
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;
}

// Initialize our program
int init_elevator(void) {
	int i;

	//Add the syscalls
	elevator_syscalls_create();

	//Intialize elevator info
	elevator.numPeople = 0;	
	elevator.load = 0;
	elevator.currentFloor = 0;
	elevator.destinationFloor = 0;
	elevator.state = STOPPED;

	//Intialize mutex locks
	mutex_init(&activateElevatorLock);
	mutex_init(&requestLock);
	mutex_init(&deactivateElevatorLock);
	mutex_init(&accessLock);

	//Initialize the floors to reflect 0 folks waiting or served
	for(i = 0; i < FLOOR_MAX; ++i){
		INIT_LIST_HEAD(&elevator.floors[i].waitQueue.list);
		elevator.floors[i].servicedNum = 0;
		elevator.floors[i].waitingNum = 0;
	}

	//Initialize elevator's passenger list
	INIT_LIST_HEAD(&elevator.elev_pass_list.list);
	//Creating thread - calls the function "elevator_run" to work on a thread
	elevatorThread = kthread_run(elevator_service, NULL, "elevatorThread");
	//Create the proc entry
	proc_create("elevator", 0, NULL, &elevator_fops);
	return 0;
}

// Exit our program
void exit_elevator(void) {
	struct passenger_info *temp_passenger;
	struct list_head *pos, *q;
	int i;

	//Exiting the thread 
	elevator.state = STOPPED;
	elevator.shutdown = 1;
	kthread_stop(elevatorThread);

	//Removing the proc entry
	remove_proc_entry("elevator", NULL);
	
	// Exiting the elevator
	list_for_each_safe(pos, q, &elevator.elev_pass_list.list){
		 temp_passenger = list_entry(pos, struct passenger_info, list);
		 list_del(pos);
		 kfree(temp_passenger);
	}	

	//Clear out the floors
	for(i=0; i < FLOOR_MAX; ++i){
		list_for_each_safe(pos, q, &elevator.floors[i].waitQueue.list){
			 temp_passenger = list_entry(pos, struct passenger_info, list);
			 list_del(pos);
			 kfree(temp_passenger);
		}	
	}
	//remove syscalls
	elevator_syscalls_remove();
}

int elevator_proc_entry(struct seq_file *m, void *v){
	// Initialize variables
	int i = 0;
	char elevatorStatus[9];

	if(act == 0)
		strcpy(elevatorStatus, "IDLE");
	else if(elevator.state == IDLE)
		strcpy(elevatorStatus, "IDLE");
	else if(elevator.state == UP)
		strcpy(elevatorStatus, "UP");
	else if(elevator.state == DOWN)
		strcpy(elevatorStatus, "DOWN");
	else if(elevator.state == LOADING)
		strcpy(elevatorStatus, "LOADING");
	else if(elevator.state == STOPPED)
		strcpy(elevatorStatus, "STOPPED");
	
	// Print out statistics - if we have an uneven load - i.e 	
	seq_printf(m, "Elevator State: %s\nOur current floor is: %d\nOur destination is: %d\nOur current weight: %d\nThe number of current Passengers: %d\n\n", 
			elevatorStatus, elevator.currentFloor+1, elevator.destinationFloor+1, elevator.load/2, elevator.numPeople);

	// Print out floor information
	for(i = 0; i < FLOOR_MAX; ++i){
		seq_printf(m, "Floor %d - Number of passengers waiting: %d Number of passengers delivered: %d\n", 
			i+1,  elevator.floors[i].waitingNum, elevator.floors[i].servicedNum);
	}
	mutex_unlock(&accessLock);
	return 0;
}

// Macros used to link the module
module_init(init_elevator);
module_exit(exit_elevator);
