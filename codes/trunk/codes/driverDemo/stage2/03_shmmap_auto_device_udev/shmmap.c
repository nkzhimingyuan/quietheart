/*
 *  Copyright(C) 2012 TP Vision Holding B.V.,
 *  All Rights Reserved.
 *  This  source code and any compilation or derivative thereof is the
 *  proprietary information of TP Vision Holding B.V.
 *  and is confidential in nature.
 *  Under no circumstances is this software to be exposed to or placed
 *  under an Open Source License of any type without the expressed
 *  written permission of TP Vision Holding B.V.
 *
 */

#if 0
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>

#include <linux/device.h>

/* For socket etc */
#include <linux/net.h>
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <linux/file.h>
#include <linux/socket.h>
#include <linux/slab.h>
#include <linux/input.h>
#include  <linux/list.h>
#include <linux/wakelock.h>

#include "tpvinput.h"


#define RF4CE_TOUCHPAD_X 240
#define RF4CE_TOUCHPAD_Y 241

#define TPV_DEBUG_PRINT 1

MODULE_AUTHOR("TPVision OS/Infra");
MODULE_LICENSE("GPL");

static LIST_HEAD(input_key_list);

static struct  tpvinput_device_type   tpvinput_device;

static void print_current_sequence_table(void);
static void init_key_sequence_table (void);
static void key_sequence_timer_callback (void);
static void key_sequence_update_timer(void);

/*code for handling the key in disable state*/
static struct workqueue_struct *tpvdisable_workqueue;

static int major;

module_param (major, int, S_IRUGO);
#endif

#include <linux/module.h> 
#include <linux/types.h> 
#include <linux/fs.h> 
#include <linux/errno.h> 
#include <linux/mm.h> 
//#include <linux/sched.h> 
#include <linux/init.h> 
#include <linux/cdev.h> 
#include <linux/device.h>
//#include <asm/io.h> 
//#include <asm/system.h> 
#include <asm/uaccess.h> 

#include "shmmap.h"

/*no use if dymanical alloc.*/

MODULE_AUTHOR("TPV OS/Infra");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module used for mmap ion shm fd to/from physical address."); 
MODULE_ALIAS("shared memory map module"); 

static int major = 1025;
static struct  shmmap_dev   shmmap_device;


static int shmmap_open(struct inode *inode, struct file *filp);
static int shmmap_release(struct inode *inode, struct file *filp);
static ssize_t shmmap_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos);
static ssize_t shmmap_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos);
static loff_t shmmap_llseek(struct file *filp, loff_t offset, int orig);
static int shmmap_ioctl(struct inode *inodep, struct file *filp, unsigned int cmd, unsigned long arg);

static const struct file_operations shmmap_fops = 
{
	.owner = THIS_MODULE,

	.llseek = shmmap_llseek,
	.read = shmmap_read,
	.write = shmmap_write,
	.unlocked_ioctl = shmmap_ioctl,
	.open = shmmap_open,
	.release = shmmap_release,
};



//void tpvinput_rpc_callback(int type, int sys, int key, int val)
//{
//}
//EXPORT_SYMBOL(tpvinput_rpc_callback);


int shmmap_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &shmmap_device;
	printk(KERN_DEBUG "open for shmmap!\n");
	return 0;
}

int shmmap_release(struct inode *inode, struct file *filp)
{
	printk(KERN_DEBUG "close for shmmap!\n");
	return 0;
}

static ssize_t shmmap_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	int ret = 0;
	struct shmmap_dev *dev = filp->private_data;
	char buffer[] = {"No content available.\n"};
	printk(KERN_ALERT "Read shmmap!\n");

	if(copy_to_user(buf, (void*)(buffer), sizeof(buffer) + 1))
	{
		printk(KERN_ALERT "some error2\n");
		ret = -EFAULT;
	}

	return ret;
}

static ssize_t shmmap_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	int ret = -EFAULT;
	struct shmmap_dev *dev = filp->private_data;
	printk(KERN_ALERT "Write shmmap not supported!\n");
	return ret;
}

static loff_t shmmap_llseek(struct file *filp, loff_t offset, int orig)
{
	int ret = -EFAULT;
	struct shmmap_dev *dev = filp->private_data;
	printk(KERN_ALERT "lseek shmmap not supported!\n");
	return ret;
}

static int shmmap_ioctl(struct inode *inodep, struct file *filp, unsigned int cmd, unsigned long arg)
{

	struct shmmap_dev *dev = filp->private_data;

	printk(KERN_DEBUG "ioctl for shmmap!\n");
	switch(cmd)
	{
		case TPVSHMMAP_IOC_FD2PHY:
			printk(KERN_ALERT "shmmap is set to zero\n");
			break;
		case TPVSHMMAP_IOC_PHY2FD:
			printk(KERN_ALERT "shmmap is set to zero\n");
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static int __init shmmap_init(void)
{
    dev_t dev_no = MKDEV(major, 0);
    int ret=0;

    shmmap_device.major = major;

	/******register major******/
    if (shmmap_device.major)
	{
        ret = register_chrdev_region(dev_no, 1, "shmmap");
        if (ret < 0)
		{
            printk(KERN_NOTICE "Error  registering the shmmap device\n");
			goto end;
        }
        else
		{
            printk(KERN_DEBUG "shmmap device is registered\n");
        }
    }
    else
	{
        ret = alloc_chrdev_region(&dev_no, 0, 1, "shmmap");
        shmmap_device.major = MAJOR(dev_no);
        if (ret < 0)
		{
            printk(KERN_WARNING "shmmap: unable to get major %d\n", shmmap_device.major);
			goto end;
        }
    }

	/******setup cdev******/
	cdev_init(&shmmap_device.cdev, &shmmap_fops);
	shmmap_device.cdev.owner = THIS_MODULE;
	shmmap_device.cdev.ops = &shmmap_fops;//no use?
	ret = cdev_add(&shmmap_device.cdev, dev_no, 1);
	if(ret < 0)
	{
		printk(KERN_ALERT "Error:%d  adding the shmmap device.\n", ret);
		goto end;
	}

	/******create the class and device automatic through udev.******/
    shmmap_device.class = class_create(THIS_MODULE, "shmmap");
    if (IS_ERR(shmmap_device.class))
	{
        ret = PTR_ERR(shmmap_device.class);
        printk(KERN_WARNING "shmmap class not created..\n");
        unregister_chrdev_region(dev_no, 1);
		goto end;
    }
    else
    {
        printk(KERN_DEBUG "shmmap class created........\n");
    }

    shmmap_device.dev = device_create(shmmap_device.class, NULL, dev_no, NULL,"shmmap");
    if (IS_ERR(shmmap_device.dev)) 
	{
        ret = PTR_ERR(shmmap_device.dev);
        printk(KERN_WARNING "shmmap device not  created..\n");
        class_destroy(shmmap_device.class);
        unregister_chrdev_region(dev_no, 1);
		goto end;
    }
    else
    {
        printk(KERN_DEBUG "shmmap device created..\n");
    }

	printk(KERN_DEBUG "Hello, shmmap driver inserted,major number is %d\n", shmmap_device.major);

end:
    return ret;
}

static void __exit shmmap_exit(void)
{
    printk("shmmap driver exiting...");
    cdev_del(&shmmap_device.cdev);
    device_destroy(shmmap_device.class, MKDEV(shmmap_device.major, 0));
    class_destroy(shmmap_device.class);
    cdev_del(&shmmap_device.cdev);
    unregister_chrdev_region(MKDEV(shmmap_device.major, 0), 1);
}

module_init(shmmap_init);
module_exit(shmmap_exit);

module_param (major, int, S_IRUGO);
