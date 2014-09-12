#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "shmapi.h"


static int DoServe();
Byte recvMsg[MSG_SIZE];

int main(int argc, char *argv[])
{
	int ret = 0;
	pid_t pid;

	ret = MmapCreate3();

	if (-1 == ret)
	{
		goto end;
	}

	ret = MmapInit();
	if (-1 == ret)
	{
		goto end;
	}

	ret = DoServe();
	MmapUninit();
	MmapDestroy();

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
