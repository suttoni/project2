/* Part 1 of Project 2 for COP4610:
   Simple C program that uses Strace
   to track number of syscalls - ultimately 
   ending with exactly 10 more syscalls than empty program.

   Empty Program Syscalls: 25
   New Number of Syscalls: 35

   Author: Ian Sutton
   FSUID: iss13
   Team: Satisfries SP3
   Team Members: Yilin Wang, Sai Gunasegaran, Ibrahim Atiya
*/

#include <stdio.h>

int main(){
	
	int i = 0;

	for(i = 0; i < 8; i++)
		printf("%s \n", "Hello World");

	return 0;
}
