/* This is a file that contains our syscalls for elevator module */

//Include needed libraries
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/export.h>
#include <linux/syscalls.h>

/* 
    Here, we define our STUBs for the syscalls - these STUBs are 
    needed in elevator_module.c.
    The syscalls themselves are implemented in module_data.c 
*/

int (* STUB_issue_request)( char pass_type, int start_floor, int desired_floor ) = NULL;
int (* STUB_start_elevator)( void ) = NULL;
int (* STUB_stop_elevator)( void ) = NULL;

/* 
   We need to export these STUBs, so we use EXPORT_SYMBOL 
   from library linux/export.h
*/

EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_stop_elevator);

/* Here, we create the syscalls themselves with asmlinkage */

asmlinkage int sys_issue_request( char pass_type, int start_floor, int desired_floor ){
	//We might need to do some error checking - maybe return an error msg
	//in case STUB_issue_request isn't implemented? 
	return STUB_issue_request(pass_type, start_floor, desired_floor);
}

asmlinkage int sys_start_elevator( void ){
	//Might also need error checking - same as above
	return STUB_start_elevator();
}

asmlinkage int sys_stop_elevator( void ){
	//Error checking same as above?
	return STUB_stop_elevator();
}
