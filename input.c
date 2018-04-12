// Shanan, Mario, Brianna
// COP 4600 - Program 3 INPUT
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/mutex.h>

#define DEVICE_NAME "inputDevice"
#define MAX_SIZE 1024
#define CLASS_NAME "inDev"

static int majorNum;
static char buff[MAX_SIZE] = {0};
short msg_len =0;

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("use your imagination");
MODULE_VERSION("2.0");

DEFINE_MUTEX(communicate_mutex);
EXPORT_SYMBOL(communicate_mutex);
EXPORT_SYMBOL(buff);
EXPORT_SYMBOL(msg_len);



static int s_open(struct inode *, struct file *);
static int s_close(struct inode *, struct file *);
static ssize_t s_write(struct file *, char *, size_t, loff_t *);
static struct class* outDevclass = NULL;
static struct device* outDevDevice = NULL;

//Setting the file operations structure to call our created functions
static struct file_operations fops =
{
	.open = s_open,
	.release = s_close,
	.write = s_write,
	.read = NULL,
};

static int __init input_module(void)
{

	//Register device with major device number
	int regVal = register_chrdev(0, DEVICE_NAME, &fops);

	if(regVal < 0){
		//Device registration has failed
		printk(KERN_ALERT "Device registration attempt has failed.\n");
		return -1;
	}
	//Device registration was successful, initialize mutex
	
	majorNum = regVal;
	printk(KERN_ALERT "Device has been registered. Major number: %d.\n", majorNum);

	outDevclass = class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(outDevclass)){
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk(KERN_ALERT "Class registration attempt has failed.\n");
		return -1;
	}
	printk(KERN_ALERT "Class has been registered.\n");

	outDevDevice = device_create(outDevclass, NULL, MKDEV(majorNum, 0), NULL, DEVICE_NAME);
	if(IS_ERR(outDevDevice)){
		class_destroy(outDevclass);
		unregister_chrdev(majorNum, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device.\n");
		return -1;
	}

	mutex_init(&communicate_mutex);
	return 0;
}

static void __exit cleanupin_module(void)
{
	//Unregister the module
	printk(KERN_ALERT "Unregistered device %d.\n", majorNum);
	unregister_chrdev(majorNum, DEVICE_NAME);
}

static int s_open(struct inode *node, struct file *f)
{
	printk(KERN_ALERT "Device opened.\n");
	return 0;
}

static int s_close(struct inode *node, struct file *f)
{
	mutex_destroy(&communicate_mutex);
	printk(KERN_ALERT "Device closed.\n");
	return 0;
}

static ssize_t s_write(struct file *f, char *arr, size_t length, loff_t *off)
{
	if(mutex_trylock(&communicate_mutex) == 0){
	return 0;
	}

	if(msg_len >= MAX_SIZE){
		mutex_unlock(&communicate_mutex);
		return length;
	}
	else if( (msg_len + length) > MAX_SIZE ){
		int i=0, j=0;
		
		for( i = msg_len-1; i<MAX_SIZE-1; i++)
			{
				j++;
				buff[i] = arr[j];
			}
		msg_len+=j;
		printk(KERN_INFO "sorry, too full >.<");
		mutex_unlock(&communicate_mutex);
		return length;
		}
	else{
		int i=0, j=0;
		
		for( i = msg_len-1; i<MAX_SIZE-1; i++)
			{
				j++;
				buff[i] = arr[j];
			}
	msg_len+=length;
	printk(KERN_INFO "hey, you did it! message with %d chars sent!", length);
	mutex_unlock(&communicate_mutex);
	return length;
	}


}

module_init(input_module);
module_exit(cleanupin_module);
