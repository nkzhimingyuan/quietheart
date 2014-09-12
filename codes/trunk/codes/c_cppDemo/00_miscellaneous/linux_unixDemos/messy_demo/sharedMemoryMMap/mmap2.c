#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "shmapi.h"


static int DoServe();
Byte recvMsg[MSG_SIZE];

static int DoRequest();
Byte sendMsg[MSG_SIZE];

int main(int argc, char *argv[])
{
	int ret = 0;
	pid_t pid;

	ret = MmapCreate2();

	if (-1 == ret)
	{
		goto end;
	}

	ret = MmapInit();
	if (-1 == ret)
	{
		goto end;
	}

	if((pid = fork()) < 0)
	{
		ret = -1;
		goto end;
	}
	else if (pid > 0)
	{/*parent act as server*/
		ret = DoServe();
		MmapUninit();
		MmapDestroy();
	}
	else
	{/*child act as client, Note:we cann't use mmap sharedmemory with another process that have no relationship with server.*/
		ret = DoRequest();
		MmapUninit();
		MmapDestroy();
	}

end:
	return ret;
}

static int DoServe()
{
	int ret = 0;
	int recvLen = 0;
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	for(;;)
	{
		struct timeval timeout = {2,0};
		select(1, NULL, NULL, NULL, &timeout);
		ret = MmapReceive(recvMsg);
		if(ret != -1)
		{
			printf("received from client is:\"%s\"\n", recvMsg);
			ret = 0;
			break;
		}
	}
	return ret;
}

static int DoRequest()
{
	int ret = 0;
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	strcpy(sendMsg, "hello, server!");
	ret = MmapSend(sendMsg, MSG_SIZE);
	return ret;
}
