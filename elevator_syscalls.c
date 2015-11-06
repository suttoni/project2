/* This is a file that contains our syscalls for elevator module 
   Author: Ian Sutton
   Team mates: Yilin Wang, Ibrahim Atiya, Sai Gunesagaran
*/

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

int (* STUB_issue_request)( int pass_type, int start_floor, int desired_floor ) = NULL;
int (* STUB_start_elevator)( void ) = NULL;
int (* STUB_stop_elevator)( void ) = NULL;

/* 
    We need to export these STUBs, so we use EXPORT_SYMBOL 
    from library linux/export.h
*/

EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_stop_elevator);

/* Here, we create the syscall wrappers with asmlinkage */

asmlinkage long sys_issue_request( int pass_type, int start_floor, int desired_floor ){
	//This basic structure is found in lab notes "system_calls.pdf"
	if(STUB_issue_request != NULL)
		return STUB_issue_request(pass_type, start_floor, desired_floor);
	else
		return -ENOSYS;
}

asmlinkage long sys_start_elevator( void ){
	//This basic structure is found in lab notes "system_calls.pdf"
	if(STUB_start_elevator != NULL)
		return STUB_start_elevator();
	else
		return -ENOSYS;
}

asmlinkage long sys_stop_elevator( void ){
	//This basic structure is found in lab notes "system_calls.pdf"
	if(STUB_stop_elevator != NULL)
		return STUB_stop_elevator();
	else
		return -ENOSYS;
}
