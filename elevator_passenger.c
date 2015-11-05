#include "module_data.h"

//External information
extern int deliveredAdults,
		   deliveredChildren,
		   deliveredBellhops,
		   deliveredRoomService;
extern struct mutex elevatorLock;
extern struct mutex floorLock;
extern struct task_struct *elevatorThread;
extern struct elevator_info elevator;
extern struct floor_info floors[NUM_FLOORS];


int remove_passengers(void){
	int removed = 0;
	struct list_head *position, *q;
	struct passenger_info *info;

	//Go through the list of passengers - as each passenger is unloaded, subtract their weight
	list_for_each(position, q, &elevator.passengers){
		info = list_entry(position, struct passenger_info, passenger_list);
		switch (info->passengerType){

			//Each passenger has a different weight type: A has 1, C has 0.5, B & R have 2
			case 'A':
				elevator.usedSpace -= 1;
				deliveredAdults++;
				floors[elevator.currentFloor - 1].numPassServed++;
				removed++;
				break;
			case 'C':
				elevator.usedSpace -= 0.5;
				deliveredChildren++;
				floors[elevator.currentFloor - 1].numPassServed++;
				removed++;
				break;
			case 'B':
				elevator.usedSpace -= 2;
				deliveredBellhops++;
				floors[elevator.currentFloor - 1].numPassServed++;
				removed++;
				break;
			case 'R':
				elevator.usedSpace -= 2;
				deliveredRoomService++;
				floors[elevator.currentFloor - 1].numPassServed++;
				removed++;
				break;
		}
		list_del(position);
		kfree(info);
	}

	return removed;
}

int add_passengers(void){
	int added = 0;
	struct list_head *position, *q;
	struct passenger_info *info;
	
	//Go through the list of passengers - as each passenger is loaded, add their weight
	list_for_each_safe(position, q, &floors[elevator.currentFloor - 1].queue){
		info = list_entry(position, struct passenger_info, passenger_list);
		if(elevator.usedSpace < MAX_PASSENGERS){
			switch (info->passengerType){
				
				//Each passenger has a different weight type: A has 1, C has 0.5, B & R have 2
				case 'A':
					if((elevator.usedSpace + 1) <= MAX_PASSENGERS){
						elevator.usedSpace += 1;
						list_del(position);
						list_add(position, &elevator.passengers);
						added++;
					}
					break;
				case 'C':
					if((elevator.usedSpace + 0.5) <= MAX_PASSENGERS){
						elevator.usedSpace += 0.5;
						list_del(position);
						list_add(position, &elevator.passengers);
						added++;
					}
					break;
				case 'B':
					if((elevator.usedSpace + 2) <= MAX_PASSENGERS){
						elevator.usedSpace += 2;
						list_del(position);
						list_add(position, &elevator.passengers);
						added++;
					}
					break;
				case 'R':
					if((elevator.usedSpace + 2) <= MAX_PASSENGERS){
						elevator.usedSpace += 2;
						list_del(position);
						list_add(position, &elevator.passengers);
						added++;
					}
					break;
			}
		}

	return added;
}

int elevator_service(void * info){

	int changes;

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
		}

		mutex_lock(&elevatorLock);

		//Our destination is a floor above us, wait 2 sec, increment floor
		if(elevator.destinationFloor > elevator.currentFloor){
			elevator.state = UP;
			//Moving to next destination, wait 2 seconds
			msleep(2000);

			//We've reached our destination, change state to loading
			if(elevator.destinationFloor == elevator.currentFloor){
				elevator.state = LOADING;
			}
			elevator.currentFloor++;
		}
		
		//Our destination is below, wait 2 sec, decrement floor
		else if(elevator.destinationFloor < elevator.currentFloor){
			elevator.state = DOWN;
			//Moving to next destination, wait 2 seconds
			msleep(2000);

			//We've reached our destination, change state to loading
			if(elevator.destinationFloor == elevator.currentFloor){
				elevator.state = LOADING;
			}
			elevator.currentFloor--;
		}

		//We're at our destination
		else{
			elevator.state = LOADING;
			elevator.currentFloor = elevator.destinationFloor; 
		}
		mutex_unlock(&elevatorLock);
	}

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
	}

	elevator.state = STOPPED;
	kthread_stop(elevatorThread);

	return 0;
}
