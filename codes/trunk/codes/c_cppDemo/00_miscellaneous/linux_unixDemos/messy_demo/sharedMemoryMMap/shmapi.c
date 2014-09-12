#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "shmapi.h"

static Byte *mmapPtr;//mmapPtr[0] used for send/receive flag.
static int mmapFd = 0;

/*Used for mmap shared memory file.*/
#define MMAP_PATH_SPECIAL "/dev/zero"
#define MMAP_PATH_NORMAL "./mmap_shared"

int MmapCreate1()
{
	int ret;

	ret = 0;
	mmapFd = open(MMAP_PATH_SPECIAL, O_RDWR);/*We use an file such as "/dev/zero" to do mmap.*/
	if (-1 == mmapFd)
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	
	mmapPtr = mmap(0, SHM_SIZE, PROT_READ |PROT_WRITE, MAP_SHARED, mmapFd, 0);
	if ( (void*)-1 == mmapPtr)/*MAP_FAILED is (void*)-1.*/
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	close(mmapFd);
	fprintf(stderr, "[Debug] in %s, line:%d, mmapFd:%d\n", __func__, __LINE__, mmapFd);
	ret = mmapFd;

end:
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	return ret;
}

int MmapCreate2()
{
	int ret;

	ret = 0;
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	
	mmapPtr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);/*Anonymous mmap without file.*/
	if ( (void*)-1 == mmapPtr)/*MAP_FAILED is (void*)-1.*/
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d, mmapFd:%d\n", __func__, __LINE__, mmapFd);

end:
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	return ret;
}

int MmapCreate3()
{
	int ret;

	ret = 0;
	mmapFd = open(MMAP_PATH_NORMAL, O_RDWR |O_CREAT |O_TRUNC);/*We use an file such as "/dev/zero" to do mmap.*/
	if (-1 == mmapFd)
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	lseek(mmapFd, SHM_SIZE-1,SEEK_SET);
	write(mmapFd, "",1);//ensure the file size not small than mmap size.
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	
	mmapPtr = mmap(0, SHM_SIZE, PROT_READ |PROT_WRITE, MAP_SHARED, mmapFd, 0);
	if ( (void*)-1 == mmapPtr)/*MAP_FAILED is (void*)-1.*/
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	close(mmapFd);
	fprintf(stderr, "[Debug] in %s, line:%d, mmapFd:%d\n", __func__, __LINE__, mmapFd);
	ret = mmapFd;

end:
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	return ret;
}

int MmapDestroy()
{
	int ret;

	ret = 0;
	if(-1 == munmap(mmapPtr, SHM_SIZE))
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

end:
	return ret;
}

int MmapInit()
{
	int ret;

	ret = 0;
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

end:
	return ret;
}

int MmapUninit()
{
	int ret = 0;

	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

end:
	return ret;
}

int MmapReceive(Byte* recv)
{
	int ret;
	//fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	if ('r' == mmapPtr[0])
	{/*we can have receive data only if mmapPtr[0] is 'r'.*/
		memcpy(recv, mmapPtr+1, MSG_SIZE);//one copy
		mmapPtr[0]='s';
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	return ret;
}

int MmapSend(Byte* send, size_t size)
{
	int ret;
	//fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	if ('r' != mmapPtr[0])
	{/*we can send data only if mmapPtr[0] is not 'r', that is don't have receive data.*/
		mmapPtr[0] = 'r';
		memset(mmapPtr+1, '\0', size);
		memcpy(mmapPtr+1, send, size);
		ret = size;
	}
	else
	{
		ret = -1;
	}
	return ret;
}
