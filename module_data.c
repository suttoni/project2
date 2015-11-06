#include "module_data.h"

/*create elevator*/
extern struct mutex elevatorLock;
extern struct mutex floorLock;

int start_elevator()
{
	/*create array of floors*/

	/*check if elevator is active*/
	if(elevator.continueRun == 1) return(1);

	/*not active*/
	else{
		/*set elevator status*/
		mutex_lock(&elevatorLock);
		elevator.currentFloor = 1;
		elevator.usedSpace = 0;
		elevator.usedWeightUnit = 0;
		elevator.state = IDLE;
		elevator.continueRun = 1;
		elevator.goingDown = 0;
		mutex_unlock(&elevatorLock);
		/*allocate memory for floors*/
		return (0);
	}
}

int issue_request(int pass_type, int start_floor, int desired_floor)
{
	/*if any of the passed in paramaters are invalid*/
	if (pass_type >=4 || start_floor <=0 || start_floor >= 11 || desired_floor <= 0 || desired_floor >= 11 || elevator.continueRun == 0){
		return(1);
	}

	else{
		/*define variables for the passenger*/
		passenger.currFloor = start_floor;
		passenger.destFloor = desired_floor;
		passenger.passengerType = pass_type;
		/*check if passenger is going up or down and set bool accordingly*/
		if (desired_floor - start_floor > 0)
			passenger.goingUp = 1;
		else 
			passenger.goingUp = 0;
		/*add passenger to floor*/
		list_add_tail(&passenger.passengerList, &floors[start_floor-1].queue);
		/*increment counter*/
		mutex_lock(&floorLock);
		floors[start_floor].queueWaiting++;	
		mutex_unlock(&floorLock);
		return (0);
	}
}
int stop_elevator()
{
	/*if there are people on the elevator*/
	mutex_lock(&elevatorLock);
	if (elevator.continueRun)
	{
		elevator.continueRun = 0;
		mutex_unlock(&elevatorLock);	
		return(0);
	}
	else{ 
		mutex_unlock(&elevatorLock);
		return(1);
	}

}

