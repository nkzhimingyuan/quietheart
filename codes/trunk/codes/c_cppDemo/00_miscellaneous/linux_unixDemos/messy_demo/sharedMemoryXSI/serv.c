#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "shmapi.h"


static int DoServe();
Byte recvMsg[MSG_SIZE];

int main(int argc, char *argv[])
{
	int ret = 0;
	int shmid;
	ret = ShmCreate();

	if (ret != -1)
	{
		ret = ShmInit();
	}

	if (ret != -1)
	{
		shmid = ret;
		ret += DoServe();
		ShmUninit();
	}
	ShmDestroy(shmid);

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
		ret = ShmReceive(recvMsg);
		if(ret != -1)
		{
			printf("received from client is:\"%s\"\n", recvMsg);
			ret = 0;
			break;
		}
	}
	return ret;
}
