// Shanan, Mario, Brianna
// COP 4600 - Program 3 OUTPUT

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/mutex.h>

#define DEVICE_NAME "outputDevice"
#define MAX_SIZE 1024
#define CLASS_NAME "outDev"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("use your imagination");
MODULE_VERSION("2.0");

extern struct mutex communicate_mutex;

static int majorNum;
extern char buff[MAX_SIZE];
extern short msg_len;

static int s_open(struct inode *, struct file *);
static int s_close(struct inode *, struct file *);
static ssize_t s_read(struct file *, const char*, size_t, loff_t *);
static struct class* outDevclass = NULL;
static struct device* outDevDevice = NULL;

//Setting the file operations structure to call our created functions
static struct file_operations fops =
{
	.open = s_open,
	.release = s_close,
	.read = s_read,
	.write = NULL,
};

static int __init output_module(void)
{

	//Register device with major device number
	int regVal = register_chrdev(0, DEVICE_NAME, &fops);

	if(regVal < 0){
		//Device registration has failed
		printk(KERN_ALERT "Device registration attempt has failed.\n");
		return -1;
	}
		//Device registration was successful
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

	return 0;
}

static void __exit cleanupout_module(void)
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
	printk(KERN_ALERT "Device closed.\n");
	return 0;
}

static ssize_t s_read(struct file *f, const char *arr, size_t length, loff_t *off)
{
	int err =0;
	
	if(mutex_trylock(&communicate_mutex) == 0){
	return 0;
	}

	err = copy_to_user(arr, buff, msg_len);
	buff[0] = '\0';
	if(*off > 0){
		mutex_unlock(&communicate_mutex);
		return 0;
		}

	if(err == 0){
		printk(KERN_INFO "sent %d chars! to user!\n", msg_len);
		*off = msg_len;
		msg_len = 0;
		mutex_unlock(&communicate_mutex);
		return (*off);
		}
	else{
		printk(KERN_INFO "couldn't send chars :(");
		mutex_unlock(&communicate_mutex);
		return -1;
		}
	
}

module_init(output_module);
module_exit(cleanupout_module);
