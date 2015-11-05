/* 
   Implements the start_elevator, stop_elevator, issue_request
   syscalls.
   Authors: Sai Gunesagaran, Ibrahim Atiya 
   Team mates: Yilin Wang, Ian Sutton 
*/

#include "module_data.h"

int start_elevator ()
{
	/*create elevator*/
	elevator_info Elevator; 
	/*create array of floors*/
	floor_info *allFloors;

	/*check if elevator is active*/
	if(continueRun == 1) return(1);

	/*not active*/
	else{
	Elevator = malloc(sizeof(elevator_info));
	/*set elevator status*/
	mutex_lock(&elevatorLock);
	Elevator.currentFloor = 1;
	Elevator.usedSpace = 0;
	Elevator.usedWeight = 0;
	Elevator.state = IDLE;
	Elevator.continueRun = 1;
	mutex_unlock(&elevatorLock);
	/*allocate memory for floors*/
	allFloors = malloc(10*sizeof(floor_info));
	return (0);}
}

int issue_request(int pass_type, int start_floor, int desired_floor)
{
	/*if any of the passed in paramaters are invalid*/
	if (pass_type >=4 || start_floor <=0 || start_floor >= 11 || desired_floor <= 0 || desired_floor >= 11 || Elevator.continueRun == 0){
		return(1);
	}
else 
{
	passenger_info Passenger = malloc(sizeof(passenger_info)); /*make new passenger*/
	/*define variables for the passenger*/
	Passenger.currFloor = start_floor;
	Passenger.destFloor = desired_floor;
	Passenger.passengerType = pass_type;
	/*add passenger to floor*/
	list_add_tail(*Passenger, allFloors[start_floor].queue);
	/*increment counter*/
	mutex_lock(&floorLock);
	allFloors[start_floor].queueWaiting++;	
	mutex_unlock(&floorLock);
	return (0);
}

int stop_elevator()
{
	/*if there are people on the elevator*/
	mutex_lock(&elevatorLock)
	if (Elevator.continueRun)
	{
		Elevator.continueRun = 0;
		mutex_unlock(&elevatorLock);	
		return(0);
	}
	else{ 
		mutex_unlock(&elevatorLock);
		return(1);
	}

}

module_data.c
Displaying module_data.c.
