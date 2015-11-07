#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm-generic/uaccess.h>

#include <linux/time.h>
struct timespec current_kernel_time(void);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Britton");
MODULE_DESCRIPTION("Simple module featuring proc read");

#define ENTRY_NAME "timed"
#define PERMS 0644
#define PARENT NULL

#define STRALLOC 80
static struct file_operations fops;

static char *message = NULL;
static struct timespec *start = NULL;
static int read_p;
static bool read = false;


int hello_proc_open(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_ALERT "proc called open\n");
	
	read_p = 1;
	if(message == NULL)
		message = kmalloc(sizeof(char) * STRALLOC, __GFP_WAIT | __GFP_IO | __GFP_FS);
	if (message == NULL) {
		printk("ERROR");
		return -ENOMEM;
	}
	if(start == NULL)
		start = kmalloc(sizeof(struct timespec), __GFP_WAIT | __GFP_IO | __GFP_FS);
	if (start == NULL) {
		printk("ERROR");
		return -ENOMEM;
	}
	return 0;
}

ssize_t hello_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {
	struct timespec curTime = current_kernel_time();
	if(read)
	{
		long long sec = (long long)curTime.tv_sec - (long long)start->tv_sec;
		long long nsec = (long long)curTime.tv_nsec - (long long)start->tv_nsec;
		if (nsec < 0)
		{
		    sec--;
		    nsec = nsec + 1000000000;
		}
		sprintf(message, "current time: %lld.%.9ld\nelapsed time: %lld.%.9ld\n", (long long)curTime.tv_sec, curTime.tv_nsec, (long long)sec, nsec);
	}else{
		sprintf(message, "current time: %lld.%.9ld\n", (long long)curTime.tv_sec, curTime.tv_nsec);
	}
	start->tv_sec = curTime.tv_sec;
	start->tv_nsec = curTime.tv_nsec;

	int len = strlen(message);

	read_p = !read_p;
	if (read_p) {
		return 0;
	}
	printk(KERN_ALERT "proc called read\n");
	copy_to_user(buf, message, len);
	
	read = true;
	return len;
}

int hello_proc_release(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_ALERT "proc called release\n");
	kfree(message);
	//we DON'T need this: kfree(start);
	return 0;
}



static int hello_init(void) {
	printk("/proc/%s create\n", ENTRY_NAME); 
	fops.open = hello_proc_open;
	fops.read = hello_proc_read;
	fops.release = hello_proc_release;
	
	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
		printk("ERROR! proc_create\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}
	return 0;
}

static void hello_exit(void) {
	remove_proc_entry(ENTRY_NAME, NULL);
	kfree(message);
	kfree(start);
	printk("Removing /proc/%s.\n", ENTRY_NAME);
}

module_init(hello_init);
module_exit(hello_exit);
