/*--------------------------------------------------------------------------------
 * Copyright (c) 2014 TPV Technology Group
 * All rights reserved
 * 
 * Unauthorized use, practice, perform, copy, distribution, reproduction,
 * or disclose of this information in whole or in part is prohibited
 *
 * -------------------------------------------------------------------------*/
/**@file tpapi_shm.cpp
 *@brief brief description
 *@author miracle.lv
 *@date 2014-03-21 
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/mman.h>
#include <linux/ion.h>
//#include <ion/ion.h>

#include "tpapi_types.h"
#include "tpapi_shm.h"

#define SHM_DEBUG 0
#if SHM_DEBUG == 1
#define TPAPI_SHM_PRINT(fmt...)
#define TPAPI_SHM_DEBUG(fmt...)        printf("---->" fmt)
#else
#define TPAPI_SHM_PRINT(fmt...)        TPAPI_LOG(IS_TPAPI_AMBI_LOG_ENABLE, "TPAPI_AMBI", fmt)
#define TPAPI_SHM_DEBUG(fmt...)
#endif

static int fd;
static bool shmInitFlag = false;

int tpapi_shm_Init(void)
{
	int ret = TPAPI_SHM_OK;
	
	TPAPI_SHM_PRINT(" Enter [%s] \n", __FUNCTION__);
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	if (true == shmInitFlag)
	{
		TPAPI_SHM_PRINT(" [%s]: tpapi shm already initialized, return. \n", __FUNCTION__);
		TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
		return ret;
	}

	shmInitFlag = true;
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	fd = open("/dev/ion", O_RDONLY | O_DSYNC);
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	if (fd < 0)
	{
		TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
		ret = TPAPI_SHM_ERROR;
	}
	return ret;

}

int tpapi_shm_Exit(void)
{
	int ret = TPAPI_SHM_OK;

	TPAPI_SHM_PRINT(" Enter [%s] \n", __FUNCTION__);
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	if (false == shmInitFlag)
	{
		TPAPI_SHM_PRINT(" [%s]: tpapi shm haven't initialized, return. \n", __FUNCTION__);
		ret = TPAPI_SHM_ERROR;
		return ret;
	}

	close(fd);
	return ret;
}

int tpapi_shm_Alloc(unsigned int size, unsigned int align, TPAPI_SHM_HANDLE* shmhandle)
{
	int ret = TPAPI_SHM_OK;
	unsigned int heapMask = ION_HEAP_SYSTEM_MASK;// XXX ???
	unsigned int flags = ION_HEAP_TYPE_SYSTEM;// XXX ???

	struct ion_fd_data share_data;
	struct ion_allocation_data alloc_data;

	TPAPI_SHM_PRINT(" Enter [%s] \n", __FUNCTION__);
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	alloc_data.len = size;
	alloc_data.align = align;
	alloc_data.heap_mask = heapMask;
	alloc_data.flags = flags;
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	ret = ioctl(fd, ION_IOC_ALLOC, &alloc_data);
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	if (ret < 0)
	{
		ret = TPAPI_SHM_MEMORY_ALLOCATION_FAILED;
		TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
		TPAPI_SHM_PRINT(" [%s]: tpapi shm alloc failed.\n", __FUNCTION__);
		goto end;
	}

	share_data.handle = alloc_data.handle;
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	ret = ioctl(fd, ION_IOC_SHARE, &share_data);
	if (ret < 0)
	{
		ret = TPAPI_SHM_MEMORY_ALLOCATION_FAILED;
		TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
		TPAPI_SHM_PRINT(" [%s]: tpapi shm share alloc failed.\n", __FUNCTION__);
		goto end;
	}

	*shmhandle = share_data.fd;
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	if (*shmhandle < 0)
	{
		ret = TPAPI_SHM_MEMORY_ALLOCATION_FAILED;
		TPAPI_SHM_PRINT(" [%s]: tpapi shm share alloc negative fd.\n", __FUNCTION__);
		goto end;
	}

end:
	return ret;
}


int tpapi_shm_Free(TPAPI_SHM_HANDLE shmhandle)
{
	int ret = TPAPI_SHM_OK;

	struct ion_handle_data handle_data;
	struct ion_fd_data fd_data;

	TPAPI_SHM_PRINT(" Enter [%s] \n", __FUNCTION__);
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);

	fd_data.fd = shmhandle;
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	ret = ioctl(fd, ION_IOC_IMPORT, &fd_data);
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	if (ret < 0)
	{
		ret = TPAPI_SHM_ERROR;
		TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
		TPAPI_SHM_PRINT(" [%s]: tpapi shm import handle error.\n", __FUNCTION__);
		goto end;
	}
	handle_data.handle = fd_data.handle;

	ret = close(shmhandle);
	if (ret < 0)
	{
		ret = TPAPI_SHM_ERROR;
		TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
		TPAPI_SHM_PRINT(" [%s]: tpapi shm close share error.\n", __FUNCTION__);
		goto end;
	}

	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	ioctl(fd, ION_IOC_FREE, &handle_data);
	TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
	if (ret < 0)
	{
		ret = TPAPI_SHM_ERROR;
		TPAPI_SHM_DEBUG(" func:[%s] line[%d] \n", __FUNCTION__, __LINE__);
		TPAPI_SHM_PRINT(" [%s]: tpapi shm free error.\n", __FUNCTION__);
		goto end;
	}

end:
	return ret;

}

int  tpapi_shm_Flush(TPAPI_SHM_HANDLE shmhandle, uint32_t offset, uint32_t size)
{/* TODO */
	int ret = TPAPI_SHM_OK;

	TPAPI_SHM_PRINT(" Enter [%s] \n", __FUNCTION__);
	/*
	ret = ion_sync_fd(fd, shmhandle);

	if(ret < 0)
	{
		ret = TPAPI_SHM_ERROR;
	}
	*/

	return ret;
}

int  tpapi_shm_Invalidate(TPAPI_SHM_HANDLE shmhandle, uint32_t offset, uint32_t size)
{/* TODO */
	int ret = TPAPI_SHM_OK;

	TPAPI_SHM_PRINT(" Enter [%s] \n", __FUNCTION__);
	/*
	ret = ion_sync_fd(fd, shmhandle);

	if(ret < 0)
	{
		ret = TPAPI_SHM_ERROR;
	}
	*/

	return ret;
}

int  tpapi_shm_GetVirtualAddress(TPAPI_SHM_HANDLE shmhandle, uint32_t offset, uint32_t size, void **virtaddr)
{/*XXX no munmap?*/
	int ret = TPAPI_SHM_OK;

	TPAPI_SHM_PRINT(" Enter [%s] \n", __FUNCTION__);
	//long pageSize = sysconf(_SC_PAGE_SIZE_);
	long pageSize = PAGE_SIZE;

	if ((offset % pageSize) & 1)
	{
		ret = TPAPI_SHM_ERROR;
		TPAPI_SHM_PRINT(" [%s]: tpapi shm offset not page aligned.\n", __FUNCTION__);
		goto end;
	}

	*virtaddr = mmap(0, size, PROT_READ |PROT_WRITE, MAP_SHARED, shmhandle, offset);
	if (MAP_FAILED == *virtaddr)
	{
		ret = TPAPI_SHM_ERROR;
		TPAPI_SHM_PRINT(" [%s]: tpapi shm mmap failed.\n", __FUNCTION__);
		goto end;
	}

end:
	return ret;
}
