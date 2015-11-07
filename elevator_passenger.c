#include "module_data.h"


//External information
extern struct task_struct *elevatorThread;
struct elevator_info elevator;
struct floor_info floors[NUM_FLOORS];
extern struct mutex elevatorLock;
extern struct mutex floorLock;

struct passenger_info passenger;

int remove_passengers(void){
	int removed = 0;
	struct list_head *position;
	struct passenger_info *info;
	deliveredAdults = 0;
	deliveredChildren = 0;
	deliveredBellhops = 0;
	deliveredRoomService = 0;

//printf("here in remove_passengers\n");

	/*Go through the list of passengers - as each passenger is unloaded, subtract their weight*/
	list_for_each(position, &elevator.passengers){
		info = list_entry(position, struct passenger_info, passengerList);
		switch (info->passengerType){

			//Each passenger has a different weight type: A has 1, C has 0.5, B & R have 2
			case 0: /*adult*/
				elevator.usedSpace -= 1;
				elevator.usedWeightUnit -= 2;
				deliveredAdults++;
				floors[elevator.currentFloor - 1].numPassServed++;
				removed++;
				break;
			case 1: /*child*/
				elevator.usedSpace -= 1;
				elevator.usedWeightUnit -= 1;
				deliveredChildren++;
				floors[elevator.currentFloor - 1].numPassServed++;
				removed++;
				break;
			case 2: /*bellhop*/
				elevator.usedSpace -= 2;
				elevator.usedWeightUnit -= 4;
				deliveredBellhops++;
				floors[elevator.currentFloor - 1].numPassServed++;
				removed++;
				break;
			case 3: /*roomservice*/
				elevator.usedSpace -= 1;
				elevator.usedWeightUnit -= 4;
				deliveredRoomService++;
				floors[elevator.currentFloor - 1].numPassServed++;
				removed++;
				break;
		}/*end switch*/
		list_del(position);
		kfree(info);
	}/*end list_for_each*/
	/*check if there are any passengers and adjust destination floor*/
	if (elevator.usedSpace == 0)
		elevator.destinationFloor = 0;
	
	return removed;
}/*end remove_passengers*/

int add_passengers(void){
	int added = 0;
	int numChecked;
	elev_movement_state previous;
	struct list_head *position;
	struct list_head *q;
	struct passenger_info *info;
	numChecked = 0;


	/*dddddddddddddddddddddddddddddddd*/
//	printf("here in add_passengers\n")
	/*Go through the list of passengers - as each passenger is loaded, add their weight*/
	list_for_each_safe(position, q, &floors[elevator.currentFloor - 1].queue){
		info = list_entry(position, struct passenger_info, passengerList);
		if(elevator.usedSpace < MAX_PASSENGERS && elevator.usedWeightUnit < MAX_LOAD){

			do
			{			
				if (elevator.usedSpace == 0) /*if there are no passengers*/
				{ 
					switch (info->passengerType)
					{				
						/*Each passenger has a different weight type: A has 1, C has 0.5, B & R have 2*/
						case 0: /*adults*/
							if((elevator.usedSpace + 1) <= MAX_PASSENGERS && (elevator.usedWeightUnit + 2) <= MAX_LOAD) 
							{
								elevator.usedSpace += 1;
								elevator.usedWeightUnit += 2;
								list_del(position);
								list_add_tail(position, &elevator.passengers);
								added++;
								elevator.destinationFloor = info->destFloor;
							}
							break;
						case 1: /*child*/
							if((elevator.usedSpace + 1) <= MAX_PASSENGERS && (elevator.usedWeightUnit + 1) <= MAX_LOAD)
							{
								elevator.usedSpace += 1;
								elevator.usedWeightUnit += 1;
								list_del(position);
								list_add_tail(position, &elevator.passengers);
								added++;
								elevator.destinationFloor = info->destFloor;
							}
							break;
						case 2: /*bellhop*/
							if((elevator.usedSpace + 2) <= MAX_PASSENGERS && (elevator.usedWeightUnit + 4) <= MAX_LOAD)
							{
								elevator.usedSpace += 2;
								elevator.usedWeightUnit += 4;
								list_del(position);
								list_add_tail(position, &elevator.passengers);
								added++;
								elevator.destinationFloor = info->destFloor;
							}
							break;
						case 3: /*roomservice*/
							if((elevator.usedSpace + 1) <= MAX_PASSENGERS && (elevator.usedWeightUnit + 4) <= MAX_LOAD)
							{
								elevator.usedSpace += 1;
								elevator.usedWeightUnit += 4;
								list_del(position);
								list_add_tail(position, &elevator.passengers);
								added++;
								elevator.destinationFloor = info->destFloor;
							}
							break;
					}/*end switch*/
				numChecked++;


				}/*end if*/
				else /*people on the elevator*/
				{
					switch (info->passengerType)
					{		
						/*check if going in the same direction*/
						if (((info->destFloor > elevator.currentFloor) && previous == UP) || 
						    ((info->destFloor < elevator.currentFloor) && previous == DOWN))
						{		
							/*Each passenger has a different weight type: A has 1, C has 0.5, B & R have 2*/
							case 0: /*adults*/
								if((elevator.usedSpace + 1) <= MAX_PASSENGERS && (elevator.usedWeightUnit + 2) <= MAX_LOAD) ////////ACCOUNT FOR 	DIRECTION
								{
									elevator.usedSpace += 1;
									elevator.usedWeightUnit += 2;
									list_del(position);
									list_add(position, &elevator.passengers);

									added++;								
								}
								break;
							case 1: /*child*/
								if((elevator.usedSpace + 1) <= MAX_PASSENGERS && (elevator.usedWeightUnit + 1) <= MAX_LOAD)
								{
									elevator.usedSpace += 1;
									elevator.usedWeightUnit += 1;
									list_del(position);
									list_add(position, &elevator.passengers);
									added++;								
								}
								break;
							case 2: /*bellhop*/
								if((elevator.usedSpace + 2) <= MAX_PASSENGERS && (elevator.usedWeightUnit + 4) <= MAX_LOAD)
								{
									elevator.usedSpace += 2;
									elevator.usedWeightUnit += 4;
									list_del(position);
									list_add(position, &elevator.passengers);
									added++;								
								}
								break;
							case 3: /*roomservice*/
								if((elevator.usedSpace + 1) <= MAX_PASSENGERS && (elevator.usedWeightUnit + 4) <= MAX_LOAD)
								{
									elevator.usedSpace += 1;
									elevator.usedWeightUnit += 4;
									list_del(position);
									list_add(position, &elevator.passengers);
									added++;								
								}
								break;
						}/*end if*/
					}/*end switch*/
				}/*end else*/					
			/*keep checking passengers on that floor until full queue checked or full*/
			}while(elevator.usedSpace < MAX_PASSENGERS && elevator.usedWeightUnit < MAX_LOAD && numChecked < floors[elevator.currentFloor - 1].queueWaiting);
		}/*end if*/
	}/*end list_for_each_safe*/	
	return (added);
}/*end add_passengers*/

int elevator_service(void * data){

	int changes;
		/*dddddddddddddddddddddddddddddddd*/
//	printf("here in elevator elevator_service\n")
	while(elevator.continueRun){

		if(elevator.state == LOADING){
			changes = 0;
			mutex_lock(&elevatorLock);
			changes += remove_passengers();
			mutex_lock(&floorLock);
			changes += add_passengers();
			//elevator.destinationFloor = (elevator.currentFloor % NUM_FLOORS) + 1;
			mutex_unlock(&elevatorLock);
			mutex_unlock(&floorLock);

			if(changes <= 4)
				changes = 0;
			//Loading passengers, wait for one second
			msleep(1000);

		}//end if

		mutex_lock(&elevatorLock);

		//Our destination is a floor above us, wait 2 sec, increment floor
		if(elevator.destinationFloor > elevator.currentFloor){
			elevator.state = UP;
			//Moving to next destination, wait 2 seconds
			msleep(2000);

			//We've reached our destination, change state to loading
			//We want to use list_for_each to include passenger info struct
			//to keep track of passenger's destination
			if(elevator.destinationFloor == elevator.currentFloor){ 
				elevator.state = LOADING;
			}
			elevator.currentFloor++;
		}//end if
		
		//Our destination is below, wait 2 sec, decrement floor
		else if(elevator.destinationFloor < elevator.currentFloor){
			elevator.state = DOWN;
			//Moving to next destination, wait 2 seconds
			msleep(2000);

			//We've reached our destination, change state to loading
			//We want to use list_for_each to include passenger info struct
			//to keep track of passenger's destination
			if(elevator.destinationFloor == elevator.currentFloor){  
				elevator.state = LOADING;
			}
			elevator.currentFloor--;
		}//end else

		//We're at our destination
		else{
			elevator.state = LOADING;
			elevator.currentFloor = elevator.destinationFloor; 
		}
		mutex_unlock(&elevatorLock);
	}//end while

	//We're no longer running the elevator
	elevator.state = IDLE;

	while(elevator.usedSpace != 0){
		//Unloading passengers
		changes = 0;
		mutex_lock(&elevatorLock);
		remove_passengers();
		mutex_unlock(&elevatorLock);
		msleep(1000);
		mutex_lock(&elevatorLock);
		elevator.currentFloor = elevator.destinationFloor;
		elevator.destinationFloor = 0;
		mutex_unlock(&elevatorLock);
		msleep(1000);
	}//end while

	elevator.state = STOPPED;
	kthread_stop(elevatorThread);

	return 0;
}

