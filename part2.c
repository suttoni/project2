/* Part 2 of Project 2: my_xTime Module
   Simple module - get value in xtime
   then display in proc file.

   Take difference of two time values when read exists

   Team: Satisfries SP3
   Team Members: Yilin Wang, Sai Gunasegaran, Ian Sutton, Ibrahim Atiya
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/seqlock.h>
#include <asm-generic/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sutton, Wang, Gunasegaran, Atiya");
MODULE_DESCRIPTION("my_xTime module");

#define ENTRY_NAME "timed"
#define PERMS 0644
#define PARENT NULL

static struct timespec start, finish;
static struct file_operations fops;

static bool flag =  false;



int my_xtime_read(char *page, char **start, off_t off, int count, int *eof, void *data);

static struct proc_dir_entry *proc_entry;

int init_my_xtime_module( void ){
	int retVal = 0;

	proc_entry = create_proc_entry(ENTRY_NAME, 0644, NULL);

	if (proc_entry == NULL){
		retVal = -ENOMEM;
		printk(KERN_INFO "my_xtime: Couldn't create proc entry\n");
	}

	else{
		fops.open = my_xtime_open;
		fops.read = my_xtime_read;
		fops.release = my_xtime_release;
		printk(KERN_INFO "my_xtime: Module loaded.\n");
		
		if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
		{
			printk("ERROR! proc_create\n");
			remove_proc_entry(ENTRY_NAME, NULL);
			return retVal = -ENOMEM;
		}
		return retVal;
	}

	return retVal;
}

static int my_xtime_release(struct inode* sp_inode, struct file* sp_file)
{
	printk("my_xtime called release\n");
	
}

void cleanup_my_xtime_module(void){
	remove_proc_entry(ENTRY_NAME, &proc_root);
	printk(KERN_INFO "xtime: Module unloaded. \n");
}

int my_xtime_read(char *page, char **start, off_t off, int count, int *eof, void *data){
	
}

module_init(init_my_xtime_module);
module_exit(cleanup_my_xtime_module);
