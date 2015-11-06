/* Part 2 of Project 2: my_xTime Module
   Simple module - get value in xtime
   then display in proc file.

   Take difference of two time values when read exists

   Team: Satisfries SP3
   Team Members: Yilin Wang, Sai Gunasegaran, Ian Sutton, Ibrahim Atiya
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/seqlock.h>
#include <asm-generic/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sutton");
MODULE_DESCRIPTION("my_xTime module");

#define ENTRY_NAME "timed"
#define PERMS 0644
#define PARENT NULL

int my_xtime_read(char *page, char **start, off_t off, int count, int *eof, void *data);

static struct proc_dir_entry *proc_entry;

int init_my_xtime_module( void ){
	int retVal = 0;

	proc_entry = create_proc_entry(ENTRY_NAME, 0644, NULL);

	if (proc_entry == NULL){
		ret = -ENOMEM;
		printk(KERN_INFO "my_xtime: Couldn't create proc entry\n");
	}

	else{
		proc_entry->read_proc = my_xtime_read;
		proc_entry->owner = THIS_MODULE;
		printk(KERN_INFO "my_xtime: Module loaded.\n");
	}

	return retVal;
}

void cleanup_my_xtime_module(void){
	remove_proc_entry(ENTRY_NAME, &proc_root);
	printk(KERN_INFO "xtime: Module unloaded. \n");
}

int my_xtime_read(char *page, char **start, off_t off, int count, int *eof, void *data){
	
}

module_init(init_my_xtime_module);
module_exit(cleanup_my_xtime_module);
