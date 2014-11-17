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

#ifndef _TPVSHMMAP_H_
#define _TPVSHMMAP_H_

#ifdef __KERNEL__
//#include  <linux/list.h>
//#include <linux/wakelock.h>
#endif

/*define constants */
#define TPVSHMMAP_IOC_MAGIC  'S'
#define TPVSHMMAP_IOC_FD2PHY             _IOWR(TPVSHMMAP_IOC_MAGIC,  1,int)
#define TPVSHMMAP_IOC_PHY2FD            _IOWR(TPVSHMMAP_IOC_MAGIC,  2,int)

struct shmmap_dev 
{ 
    int major;
    struct device *dev;
    struct class * class;
	struct cdev cdev; 
	//unsigned char mem[GLOBALMEM_SIZE]; 
}; 

#ifdef __KERNEL__
#endif //__KERNEL__

#endif
