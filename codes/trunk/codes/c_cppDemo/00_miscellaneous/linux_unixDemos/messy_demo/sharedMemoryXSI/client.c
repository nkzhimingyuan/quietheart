#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "shmapi.h"

static int DoRequest(int shmid);
Byte sendMsg[MSG_SIZE];

int main(int argc, char *argv[])
{
	int ret = 0;
	int shmid;
	ret = ShmInit();

	if (ret > 0)
	{
		ret = DoRequest(shmid);
		ShmUninit();
	}

	return ret;
}

static int DoRequest(int shmid)
{
	int ret = 0;
	fprintf(stderr, "[Debug] in %s, line:%d\n", __func__, __LINE__);
	strcpy(sendMsg, "hello, server!");
	ret = ShmSend(sendMsg, MSG_SIZE);
	return ret;
}
