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

#define DEFAULTLEN 64

#define ENTRY_NAME "timed"
#define PERMS 0644
#define PARENT NULL

static struct timespec * last;
static struct file_operations fops;
static char *info = NULL;
static int read_p;
static bool flag =  false;
static struct proc_dir_entry *proc_entry;

int my_xtime_open(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_ALERT "my_xtime: proc called open\n");
	
	read_p = 1;
	if(info == NULL)
		info = kmalloc(sizeof(char) * DEFAULTLEN, __GFP_WAIT | __GFP_IO | __GFP_FS);
	if (info == NULL) {
		printk("ERROR");
		return -ENOMEM;
	if(last == NULL)
		start = kmalloc(sizeof(struct timespec), __GFP_WAIT | __GFP_IO | __GFP_FS);
	if (last == NULL) {
		printk("ERROR");
		return -ENOMEM;
	}
}

ssize_t my_xtime_read(struct file *sp_file, size_t size, char __user *buf, loff_t *offset) {
	struct timespec currentTime = current_kernel_time();
	if(!flag)
	{
		sprintf(info, "current time: %lld.%.9ld\n", (long long)currentTime.tv_sec, curTime.tv_nsec);
	}
	else	// flag == true
	{
		long long s = (long long)currentTime.tv_sec - (long long)start->tv_sec;
		long long ns = (long long)currentTime.tv_nsec - (long long)start->tv_nsec;
		if (ns < 0)
		{
		    s--;
		    ns = ns + 1000000000;
		}
		sprintf(info, "current time: %lld.%.9ld\nelapsed time: %lld.%.9ld\n", (long long)currentTime.tv_sec, curTime.tv_nsec);
	}
	last->tv_sec = currentTime.tv_sec;
	last->tv_nsec = currentTime.tv_nsec;
	
	int len = strlen(info);
	read_p = !read_p;
	if (read_p) 
	{
		return 0;
	}
	printk(KERN_ALERT "my_xtime: proc called read\n");
	copy_to_user(buf, info, len);
	
	read = true;
	return len;

}



int init_my_xtime_module( void ){
	int retVal = 0;
	fops.open = my_xtime_open;
	fops.read = my_xtime_read;
	fops.release = my_xtime_release;

	proc_entry = create_proc_entry(ENTRY_NAME, PERMS, NULL, &fops);

	if (proc_entry == NULL){
		retVal = -ENOMEM;
		remove_proc_entry(ENTRY_NAME, NULL);
		printk(KERN_INFO "my_xtime: Couldn't create proc entry\n");
	}
	else
		printk(KERN_INFO "my_xtime: Module loaded.\n");

	return retVal;
}

static int my_xtime_release(struct inode* sp_inode, struct file* sp_file)
{
	printk(KERN_ALERT "proc called release\n");
	kfree(info);
	return 0;
}

void cleanup_my_xtime_module(void){
	remove_proc_entry(ENTRY_NAME, NULL);
	kfree(info);
	printk(KERN_INFO "xtime: Module unloaded. \n");
}


module_init(init_my_xtime_module);
module_exit(cleanup_my_xtime_module);
