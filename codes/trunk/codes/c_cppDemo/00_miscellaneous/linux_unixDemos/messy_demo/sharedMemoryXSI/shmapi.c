#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include "shmapi.h"

static Byte *shmPtr;//shmPtr[0] used for send/receive flag.
static int resId = 0;

/*Used for shared memory key.*/
#define SHM_KEY_PATH "/dev/zero"
#define SHM_KEY_ID 1

int ShmCreate()
{
	int ret;
	key_t shmKey;

	ret = 0;
	/*compose key for get sharedmemory handle, note:no need to use ftok if you can have an unique key.*/
	shmKey = ftok(SHM_KEY_PATH, SHM_KEY_ID);
	if ((key_t) -1 == shmKey)
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	
	/*create sharedmemory handle by key, note:use IPC_CREAT flag*/
	resId = shmget(shmKey, SHM_SIZE, SHM_MODE |IPC_CREAT);//We cann't get shm with creat twice with  the same key!!!
	if (resId < 0)
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d, resId:%d\n", __func__, __LINE__, resId);
	ret = resId;

end:
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	return ret;
}

int ShmDestroy(int shmid)
{
	int ret;
	key_t shmKey;

	ret = 0;
	shmKey = ftok(SHM_KEY_PATH, SHM_KEY_ID);
	if ((key_t) -1 == shmKey)
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

	/*remove sharedmemory through its handle.*/
	if(-1 == shmctl(shmid, IPC_RMID, 0))
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

end:
	return ret;
}

int ShmInit()
{
	int ret;
	key_t shmKey;

	ret = 0;
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	shmKey = ftok(SHM_KEY_PATH, SHM_KEY_ID);
	if ((key_t) -1 == shmKey)
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

	resId = shmget(shmKey, SHM_SIZE, SHM_MODE);/*get exist sharedmemory handle through its key.*/
	if (-1 == resId)
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

	shmPtr = shmat(resId, 0, 0);/*get the sharedmemory address through its handle.*/
	if ((void *)-1 == shmPtr)
	{
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	ret = resId;
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

end:
	return ret;
}

int ShmUninit()
{
	int ret = 0;

	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	if (-1 == shmdt(shmPtr))
	{/*put the sharedmemory address through its handle.*/
		ret = -1;
		fprintf(stderr, "[Error] in %s, line:%d\n", __func__, __LINE__);
		goto end;
	}
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);

end:
	return ret;
}

int ShmReceive(Byte* recv)
{
	int ret;
	if ('r' == shmPtr[0])
	{/*we can have receive data only if shmPtr[0] is 'r'.*/
		memcpy(recv, shmPtr+1, MSG_SIZE);//one copy
		shmPtr[0]='s';
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	return ret;
}

int ShmSend(Byte* send, size_t size)
{
	int ret;
	if ('r' != shmPtr[0])
	{/*we can send data only if shmPtr[0] is not 'r', that is don't have receive data.*/
		shmPtr[0] = 'r';
		memset(shmPtr+1, '\0', size);
		memcpy(shmPtr+1, send, size);
		ret = size;
	}
	else
	{
		ret = -1;
	}
	return ret;
}
