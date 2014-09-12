#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "shmapi.h"


static int DoRequest();
Byte sendMsg[MSG_SIZE];

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

	ret = DoRequest();
	MmapUninit();
	MmapDestroy();

end:
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
