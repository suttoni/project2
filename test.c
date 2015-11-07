
// INCLUDES
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <asm-generic/uaccess.h>
#include <syscalls.h>

// MODULE DEFINITIONS
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Britton");
MODULE_DESCRIPTION("Simple module designed to illustrate scheduling");

// PROGRAM DEFINITIONS
#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PARENT NULL

#define MIN_FLOOR 1
#define MAX_FLOOR 10

#define MAX_LOAD 16
#define MAX_PERSONS 8
#define WAIT_FLOORS_TIME 2000 /*msec*/
#define WAIT_LOAD_TIME 1000 /*msec*/

#define MAX_MESSAGE_LENGTH 2046
#define KFLAGS (__GFP_WAIT | __GFP_IO | __GFP_FS) 

// STRUCTURE DEFINITIONS

typedef enum
{
	ADULT, CHILD, BELLHOP, ROOMSERVICE
} PersonType;

typedef enum
{
	IDLE, UP, DOWN, LOADING, STOPPED, INACTIVE
} ElevatorStatus;


struct Person{
	PersonType type;
	int weight;
	int floor;
	struct list_head list;
};

struct Floor{
	struct Person waitingList;
	int numWaiting;
	int numServiced;
};

struct Elevator{
	int floor;
	int load;
	int numPeople;
	int targetFloor;
	ElevatorStatus status;
	struct Floor floors[MAX_FLOOR];
	struct Person insideList;
};


// EVENT CONTROLLERS
static struct file_operations fops;
static int read_p;

// MUTEXS'
static struct mutex activating_mutex;
static struct mutex deactivating_mutex;
static struct mutex issue_request_mutex;
static struct mutex data_access_mutex;
static struct task_struct *elevator_thread;

// STATIC
static int active = 0;
static int deactivating = 0;
static int shutdown = 0;
static struct Elevator elevator;

// DYNAMIC
static char *message = NULL; // safe

int absVal(int a)
{
	if(a < 0)
		return -1 * a;
	else
		return a;
}

// ELEVATOR THREAD
int elevator_run(void *data)
{
	// HEY UHH WHAT DO I DO YO?
	printk("ELEVATOR THREAD STARTED\n");
	while(shutdown == 0)
	{
		//printk("ELEVATOR THREAD LOOP\n");
		while(elevator.status != STOPPED)
		{
			struct list_head *pos, *q;
			struct Person *person;
			if(elevator.status == LOADING)
			{
				
				printk("LOADING IF ENTER\n");
				printk("LOADING SLEEP 1 SEC...\n");
				ssleep(1);
				
				// GET PEOPLE OFF ( ͡° ͜ʖ ͡°)
				mutex_lock(&data_access_mutex);
				list_for_each_safe(pos, q, &elevator.insideList.list) 
				{
					person = list_entry(pos, struct Person, list);
					if(person->floor == elevator.floor)
					{
						elevator.load -= person->weight;
						elevator.numPeople--;
						list_del(pos);
						kfree(person);
					}
				}	

				// GET PEOPLE ON ̿̿’̿’\̵͇̿̿\=(•̪●)=/̵͇̿̿/’̿̿ ̿ ̿ ̿
				while(elevator.floors[elevator.floor].numWaiting > 0)
				{
					person = list_first_entry(&elevator.floors[elevator.floor].waitingList.list, struct Person, list);

					if(elevator.load + person->weight <= 16 && elevator.numPeople <= 8)
					{
						list_del(&person->list);
						list_add_tail(&person->list, &elevator.insideList.list);
						elevator.numPeople++;
						elevator.floors[elevator.floor].numWaiting--;
						elevator.floors[elevator.floor].numServiced++;
						elevator.load += person->weight;
					}else{
						break;
					}
				}		

				//  (ﾉ◕ヮ◕)ﾉ*:･ﾟ✧  CLOSEST FLOOR FIRST (ﾉ◕ヮ◕)ﾉ*:･ﾟ✧ 
				if(elevator.numPeople != 0)
				{
					int targetFloor = 100000; // big number
					list_for_each(pos, &elevator.insideList.list) 
					{
						person = list_entry(pos, struct Person, list);
						if(absVal(person->floor - elevator.floor) < absVal(targetFloor - elevator.floor))
						{
							targetFloor = person->floor;
						}
					}	
					elevator.targetFloor = targetFloor;
					if(elevator.targetFloor == elevator.floor){
						elevator.status = LOADING;
						printk("STATE CHANGE: LOADING TO LOADING\n");
					}else if(elevator.targetFloor > elevator.floor){
						elevator.status = UP;
						printk("STATE CHANGE: LOADING TO UP\n");
					}else if(elevator.targetFloor < elevator.floor){
						elevator.status = DOWN;	
						printk("STATE CHANGE: LOADING TO DOWN\n");
					}
				
				}else{
					elevator.status = IDLE;
					printk("STATE CHANGE: LOADING TO IDLE\n"); // goes to IDLE so that it can check where to go next. State will only be idle for like 1ms.
				}
				mutex_unlock(&data_access_mutex);
			

			}
			if(elevator.status == UP)
			{
				printk("UP IF ENTER\n");

				printk("GOING UP SLEEP 2 SEC...\n");
				ssleep(2);
				elevator.floor++;
			
				mutex_lock(&data_access_mutex);

				// IF PPL WANNA GET ON OR OFF LETS LOAD
				if(elevator.floors[elevator.floor].numWaiting > 0)
				{
					elevator.status = LOADING;
					printk("STATE CHANGE: UP TO LOADING\n");
				}
			
				list_for_each(pos, &elevator.insideList.list) 
				{
					person = list_entry(pos, struct Person, list);
					if(person->floor == elevator.floor)
					{
						elevator.status = LOADING;
						printk("STATE CHANGE: UP TO LOADING\n");
					}
				}	
				mutex_unlock(&data_access_mutex);
			
			}else if(elevator.status == DOWN)
			{
				printk("DOWN IF ENTER\n");

				printk("GOING DOWN SLEEP 2 SEC...\n");
				ssleep(2);
				elevator.floor--;
			

				mutex_lock(&data_access_mutex);
				// IF PPL WANNA GET ON OR OFF LETS LOAD
				if(elevator.floors[elevator.floor].numWaiting > 0)
				{
					elevator.status = LOADING;
					printk("STATE CHANGE: DOWN TO LOADING\n");
				}
			
				list_for_each(pos, &elevator.insideList.list) 
				{
					person = list_entry(pos, struct Person, list);
					if(person->floor == elevator.floor)
					{
						elevator.status = LOADING;
						printk("STATE CHANGE: DOWN TO LOADING\n");
					}
				}	
				mutex_unlock(&data_access_mutex);

			}

			if(elevator.status == IDLE)
			{
				int i;
				printk("IDLE IF ENTER\n");
				
				mutex_lock(&data_access_mutex);
				// HEY WHERE ARE PEOPLE WAITING? GO TO NEAREST FLOOR
				int targetFloor = 1234; // big num
				for(i = 0;i<MAX_FLOOR;++i)
				{
					if(elevator.floors[i].numWaiting > 0 && absVal(i - elevator.floor) < absVal(targetFloor - elevator.floor))
					{
						targetFloor = i;
					}
				}
				if(targetFloor != 1234)
				{
					elevator.targetFloor = targetFloor;
					if(elevator.targetFloor == elevator.floor){
						elevator.status = LOADING;
						printk("STATE CHANGE: IDLE TO LOADING\n");
					}else if(elevator.targetFloor > elevator.floor){
						elevator.status = UP;
						printk("STATE CHANGE: IDLE TO UP\n");
					}else if(elevator.targetFloor < elevator.floor){
						elevator.status = DOWN;	
						printk("STATE CHANGE: IDLE TO DOWN\n");
					}				
				}


				mutex_unlock(&data_access_mutex);
				
				// STILL WAITING?
				if(elevator.status == IDLE)
				{
					printk("IDLE STILL WAITING SLEEP 1 SEC...\n");
					ssleep(1);
				}				
			}
		}
		ssleep(1);
	}
	printk("ELEVATOR THREAD STOPPED\n");	

	return 0;
}

// START ELEVATOR --MULTITHREADED safe
extern long (*STUB_start_elevator)(void);
long start_elevator(void) {
	int ret = 1;

	mutex_lock (&activating_mutex);
	if(active == 0)
	{
		ret = 0;
		printk("Starting elevator\n");
		//kthread_run(producer_run, NULL, "producer thread"); 

		if(!active)
		{

			// START THREAD
			elevator.status = IDLE;
			
		}
		active = 1;
	}
	mutex_unlock(&activating_mutex);
	
	return ret;
}

// ADD PERSON --MULTITHREADED safe
extern long (*STUB_issue_request)(int,int,int);
long issue_request(int passenger_type, int start_floor, int destination_floor) {

	// INIT VARIABLES
	int weight;
	PersonType type;
	struct Person *person;

	// GATHER INFORMATION
	printk("NEW REQUEST: %d, %d => %d\n", passenger_type, start_floor, destination_floor);
	if(start_floor == destination_floor)
		return 1; // wtf

	if(start_floor > MAX_FLOOR || start_floor < MIN_FLOOR)
		return 1;

	if(destination_floor > MAX_FLOOR || destination_floor < MIN_FLOOR)
		return 1;

	

	if(passenger_type == 0){
		weight = 2;
		type = ADULT;
	}else if(passenger_type == 1){
		weight = 1;
		type = CHILD;
	}else if(passenger_type == 2){
		weight = 4;
		type = BELLHOP;
	}else if(passenger_type == 3){
		weight = 4;
		type = ROOMSERVICE;
	}else{
		return 1;
	}
		

	// PUT A PERSON ON WAIT LIST
	mutex_lock(&issue_request_mutex);
	
	person = kmalloc(sizeof(struct Person), __GFP_WAIT | __GFP_IO | __GFP_FS);
	person->weight = weight;
	person->floor = destination_floor-1;
	person->type = type;

	mutex_lock(&data_access_mutex);
	list_add_tail(&(person->list), &(elevator.floors[start_floor-1].waitingList.list));
	++(elevator.floors[start_floor-1].numWaiting);
	mutex_unlock(&data_access_mutex);

	mutex_unlock(&issue_request_mutex);

	return 0;
}

// STOP ELEVATOR --MULTITHREADED safe
extern long (*STUB_stop_elevator)(void);
long stop_elevator(void) {

	mutex_lock (&activating_mutex);
	if(active == 0)
		return 0;
	mutex_unlock (&activating_mutex);

	mutex_lock (&deactivating_mutex);
	if(deactivating == 0){
		deactivating = 1;
		elevator.status = STOPPED;
	}else{
		mutex_unlock(&deactivating_mutex);
		return 1;
	}
	mutex_unlock(&deactivating_mutex);
	
	printk("STOPPING ELEVATOR\n");
	// CLEANUP CODE HERE

	//

	mutex_lock (&activating_mutex);
	mutex_lock (&deactivating_mutex);

	
	active = 0;
	deactivating = 0;

	mutex_unlock (&activating_mutex);
	mutex_unlock (&deactivating_mutex);

	return 0;
}

// CREATE SYSCALLS
void elevator_syscalls_create(void) {
	STUB_start_elevator =& (start_elevator);
	STUB_issue_request =& (issue_request);
	STUB_stop_elevator =& (stop_elevator);
}

// REMOVE SYSCALLS
void elevator_syscalls_remove(void) {
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;
}

// PROC OPEN
int evan_proc_open(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_ALERT "PROC OPEN\n");
	read_p = 1;
	return 0;
}

// PROC READ
ssize_t evan_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) 
{
	// INIT VARIABLES
	char* msg = kmalloc(sizeof(char) * MAX_MESSAGE_LENGTH, __GFP_WAIT | __GFP_IO | __GFP_FS); // safe
	int i = 0;
	int len = 0;
	char moveState[9];
	message[0] = 0; // clear message.

	printk("PROC READ\n");
	
	// DETERMINE STATE	

	// IDLE, UP, DOWN, LOADING, STOPPED, INACTIVE
	mutex_lock(&data_access_mutex);
	if(active == 0)
		strcpy(moveState, "IDLE");
	else if(elevator.status == IDLE)
		strcpy(moveState, "IDLE");
	else if(elevator.status == UP)
		strcpy(moveState, "UP");
	else if(elevator.status == DOWN)
		strcpy(moveState, "DOWN");
	else if(elevator.status == LOADING)
		strcpy(moveState, "LOADING");
	else if(elevator.status == STOPPED)
		strcpy(moveState, "STOPPED");
	

	// ELEVATOR FLOOR INDEPENDENT PRINT
	if(elevator.load % 2 == 0)
	{
		sprintf(message, "Elevator State: %s\nFloor: %i\nTarget Floor: %i\nCurrent Weight: %i\nCurrent Passengers: %i\n\n", 
			moveState, elevator.floor+1, elevator.targetFloor+1, elevator.load/2, elevator.numPeople);
	}else{
		sprintf(message, "Elevator State: %s\nFloor: %i\nTarget Floor: %i\nCurrent Weight: %i.5\nCurrent Passengers: %i\n\n", 
			moveState, elevator.floor+1, elevator.targetFloor+1, elevator.load/2, elevator.numPeople);
	}

	// ELEVATOR FLOOR DEPENDENT PRINT
	for(i=0;i<MAX_FLOOR;++i)
	{
		sprintf(msg, "- Floor %i -\nNum Waiting: %i\nNum Serviced: %i\n\n", i+1,  elevator.floors[i].numWaiting, elevator.floors[i].numServiced);
		strcat(message, msg);
	}
	mutex_unlock(&data_access_mutex);

	len = strlen(message);

	// CLEANUP
	kfree(msg);	
	if(len > MAX_MESSAGE_LENGTH)
	{
		printk("ERROR: MESSAGE TOO LONG\n");
		return 0;
	}

	// COPY TO USER
	read_p = !read_p;
	if (read_p) {
		return 0;
	}

	copy_to_user(buf, message, len);

	return len;
}

// PROC EXIT
int evan_proc_release(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_ALERT "PROC RELEASE\n");

	return 0;
}

// PROGRAM INIT
static int mike_init(void) {
	int i;

	printk("PROGRAM INIT\n"); 

	// ADDING SYSCALLS
	printk("CREATING SYSTEM CALLS\n"); 
	elevator_syscalls_create();


	// INIT MUTEXS'
	mutex_init(&activating_mutex);
	mutex_init(&issue_request_mutex);
	mutex_init(&deactivating_mutex);
	mutex_init(&data_access_mutex);

	// ALLOCATING DYNAMIC VARIABLES
	message = kmalloc(sizeof(char) * MAX_MESSAGE_LENGTH, __GFP_WAIT | __GFP_IO | __GFP_FS);

	// INIT ELEVATOR
	elevator.floor = 0;
	elevator.load = 0;
	elevator.status = STOPPED;	
	elevator.targetFloor = 0;
	elevator.numPeople = 0;

	for(i=0;i<MAX_FLOOR;++i)
	{
		INIT_LIST_HEAD(&elevator.floors[i].waitingList.list);
		elevator.floors[i].numWaiting = 0;
		elevator.floors[i].numServiced = 0;

	}
	INIT_LIST_HEAD(&elevator.insideList.list);

	// INIT PROC
	printk("CREATING PROC /proc/%s create\n", ENTRY_NAME); 
	fops.open = evan_proc_open;
	fops.read = evan_proc_read;
	fops.release = evan_proc_release;
	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
		printk("ERROR! proc_create\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	// INIT THREAD
	elevator_thread = kthread_run(elevator_run, NULL, "elevator_thread");

	return 0;
}

// PROGRAM EXIT
static void elliot_exit(void) {
	struct Person *tmp;
	struct list_head *pos, *q;
	int i;
	printk("PROGRAM EXIT\n");

	// EXIT THREAD
	elevator.status = STOPPED;
	shutdown = 1;
	kthread_stop(elevator_thread);

	// EXIT PROC
	remove_proc_entry(ENTRY_NAME, NULL);
	
	// EXIT ELEVATOR
	list_for_each_safe(pos, q, &elevator.insideList.list){
		 tmp = list_entry(pos, struct Person, list);
		 list_del(pos);
		 kfree(tmp);
	}	

	for(i=0;i<MAX_FLOOR;++i)
	{
		list_for_each_safe(pos, q, &elevator.floors[i].waitingList.list){
			 tmp = list_entry(pos, struct Person, list);
			 list_del(pos);
			 kfree(tmp);
		}	
	}

	// DEALLOCATING DYNAMIC VARIABLES
	kfree(message);

	// REMOVING SYSCALLS
	elevator_syscalls_remove();
	printk("PROGRAM EXITED SUCCESFULLY\n");
}


// LINKING MACROS
module_init(mike_init);
module_exit(elliot_exit);

