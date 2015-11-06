/* Simple driver program to test elevator-scheduler */

#include <unistd.h>
#include <sys/syscall.h>

#define __NR_ISSUE_REQUEST 324
#define __NR_START_ELEVATOR 323
#define __NR_STOP_ELEVATOR 322

int issue_request

int main(){

    /* 
       All three syscall's corresponding #'s can be found in syscall_64.tbl
       Begin by requesting elevator with syscall # of 324
       0 refers to an adult (no longer using char, but int instead),
       1 refers to start_floor (current floor), while 3 refers to destination floor
    */
    
    syscall(324, 0, 1, 3);
    //Start the elevator with syscall # of 323
    syscall(323);
    //Stop elevator with syscall # of 325
    syscall(325);
    
    //When doing heavier testing, try to call Stop or Start more than once
    
    return 0;
}
