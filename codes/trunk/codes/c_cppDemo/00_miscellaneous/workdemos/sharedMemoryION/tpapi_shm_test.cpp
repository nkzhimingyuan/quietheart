/*
*client send "request_shm" to socket.
*server recv "request_shm" from socket.
*server alloc shm, and send share_fd back to client socket.
*client recv shared_fd from socket.
*client send "request_data" to socket.
*server recv "request_data" from socket.
*server puts "Server Data" to shm which is mmap from shared_fd, and send "respose_data" to socket to tell client read data from shm.
*client recv "response_data" from socket, and gets "Server Data" from shm.
 * */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tpapi_shm.h"

static struct iovec rVec[3];
char rVecStr[16] = {'\0'};
int rVecInt1 = 0;
int rVecInt2 = 0;

static struct iovec sVec[3];
char sVecStr[16] = {'\0'};
int sVecInt1 = 0;
int sVecInt2 = 0;

char ctlBuf[CMSG_SPACE(sizeof(TPAPI_SHM_HANDLE))];

static int ServerRecvShmRequest(int sockfd, struct msghdr *pRcvMsg);
static int ServerAllocShmFd(struct msghdr *pRcvMsg, TPAPI_SHM_HANDLE *shmHandle);//Alloc shm.
static int ServerSendShmResponse(int sockfd, struct msghdr *pSndMsg, TPAPI_SHM_HANDLE *shmHandle);//Reponse fd
static int ServerRecvShmDataRequest(int sockfd, struct msghdr *pRcvMsg);
static int ServerPutShmData(TPAPI_SHM_HANDLE *shmHandle, char *data, int size);//Write shm.
static int ServerSendShmDataResponse(int sockfd, struct msghdr *pSndMsg);

static int ClientSendShmRequest(int sockfd, struct msghdr *pSndMsg);
static int ClientRecvShmResponse(int sockfd, struct msghdr *pRcvMsg);
static int ClientRetriveShmFd(struct msghdr *pRcvMsg, TPAPI_SHM_HANDLE *shmHandle);
static int ClientSendShmDataRequest(int sockfd, struct msghdr *pSndMsg);
static int ClientRecvShmDataResponse(int sockfd, struct msghdr *pRcvMsg);
static int ClientGetShmData(TPAPI_SHM_HANDLE *shmHandle, char *data, int size);//Read shm.

static int SendErrorNotify(int sockfd, struct msghdr *pSndMsg);

int main(int argc, char *argv[])
{
	int ret = 0;
	pid_t pid;
	int sd[2];
	struct msghdr sndMsg;
	struct msghdr rcvMsg;

	TPAPI_SHM_HANDLE shmHandle;

	ret = tpapi_shm_Init();
	if (TPAPI_SHM_OK != ret)
	{
		fprintf(stderr, "shared memory initialized error!\n");
		goto End;
	}

	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sd);/*sd[0] used for parent read/write, sd[1] used for child read/write.*/
	if (-1 == ret)
	{
		fprintf(stderr, "socketpair error!\n");
		goto End;
	}

	sVec[0].iov_base = sVecStr;
	sVec[0].iov_len = sizeof(sVecStr);
	sVec[1].iov_base = &sVecInt1;
	sVec[1].iov_len = sizeof(int);
	sVec[2].iov_base = &sVecInt2;
	sVec[2].iov_len = sizeof(int);
	sndMsg.msg_iov = sVec;
	sndMsg.msg_iovlen = sizeof(sVec) / sizeof(struct iovec);

	rVec[0].iov_base = rVecStr;
	rVec[0].iov_len = sizeof(rVecStr);
	rVec[1].iov_base = &rVecInt1;
	rVec[1].iov_len = sizeof(int);
	rVec[2].iov_base = &rVecInt2;
	rVec[2].iov_len = sizeof(int);
	rcvMsg.msg_iov = rVec;
	rcvMsg.msg_iovlen = sizeof(rVec) / sizeof(struct iovec);

	pid = fork();
	if (pid > 0)
	{/*parent*/
		char data[16] = "Server Data";
		/*server recv "request_shm" from socket.*/
		if ((ret = ServerRecvShmRequest(sd[0], &rcvMsg)) < 0)
		{
			SendErrorNotify(sd[0], &sndMsg);
			goto ParentEnd;
		}

		/*server alloc shm.*/
		if ((ret = ServerAllocShmFd(&rcvMsg, &shmHandle)) < 0)
		{
			SendErrorNotify(sd[0], &sndMsg);
			goto ParentEnd;
		}

		/* send share_fd back to client socket.*/
		if ((ret = ServerSendShmResponse(sd[0], &sndMsg, &shmHandle)) < 0)
		{
			SendErrorNotify(sd[0], &sndMsg);
			goto ParentCloseEnd;
		}

		/*server recv "request_data" from client socket.*/
		if ((ret = ServerRecvShmDataRequest(sd[0], &rcvMsg)) < 0)
		{
			SendErrorNotify(sd[0], &sndMsg);
			goto ParentCloseEnd;
		}

		/*server puts "Server Data" to shm which is mmap from shared_fd,*/
		if ( (ret = ServerPutShmData(&shmHandle, data, sizeof(data))) < 0)
		{
			SendErrorNotify(sd[0], &sndMsg);
			goto ParentCloseEnd;
		}

		
		/*and send "respose_data" to socket to tell client read data from shm.*/
		if ((ret = ServerSendShmDataResponse(sd[0], &sndMsg)) < 0)
		{
			SendErrorNotify(sd[0], &sndMsg);
			goto ParentCloseEnd;
		}


		//munmap(shmPtr, sizeof(data));//XXX where to mumap?

ParentCloseEnd:
		close(shmHandle);
ParentEnd:
		waitpid(pid, NULL, 0);
		printf("[In server] Server exit.\n");
	}
	else if (0 == pid)
	{/*child*/
		char data[16] = {'\0'};
		/*client send "request_shm" to socket.*/
		if ((ret = ClientSendShmRequest(sd[1], &sndMsg)) < 0)
		{
			SendErrorNotify(sd[1], &sndMsg);
			goto ChildEnd;
		}


		/*client recv shared_fd from socket.*/
		if ((ret = ClientRecvShmResponse(sd[1], &rcvMsg)) < 0)
		{
			SendErrorNotify(sd[1], &sndMsg);
			goto ChildEnd;
		}

		if ((ret = ClientRetriveShmFd(&rcvMsg, &shmHandle)) < 0)
		{
			SendErrorNotify(sd[1], &sndMsg);
			goto ChildCloseEnd;
		}

		/*client send "request_data" to socket.*/
		if ( (ret = ClientSendShmDataRequest(sd[1], &sndMsg)) < 0)
		{
			SendErrorNotify(sd[1], &sndMsg);
			goto ChildCloseEnd;
		}

		/*client recv "response_data" from socket*/
		if ( (ret = ClientRecvShmDataResponse(sd[1], &rcvMsg)) < 0)
		{
			goto ChildCloseEnd;
		}

		/*and gets "data from server" from shm.*/
		if ( (ret = ClientGetShmData(&shmHandle, data, sizeof(data))) < 0)
		{
			goto ChildCloseEnd;
		}

		//munmap(shmPtr, sizeof(data));

ChildCloseEnd:
		close(shmHandle);
ChildEnd:
		printf("[In client] Client exit.\n");
	}


	ret = tpapi_shm_Exit();
	if (TPAPI_SHM_OK != ret)
	{
		fprintf(stderr, "shared memory exit error!\n");
		goto End;
	}

End:
	return ret;
}

static int ServerRecvShmRequest(int sockfd, struct msghdr *pRcvMsg)
{
	int ret = 0;

	memset(pRcvMsg, 0, sizeof(struct msghdr));
	memset(rVec[0].iov_base, 0, sizeof(rVec[0].iov_len));//XXX MACRO INDEX
	memset(rVec[1].iov_base, 0, sizeof(rVec[1].iov_len));
	memset(rVec[2].iov_base, 0, sizeof(rVec[2].iov_len));

	pRcvMsg->msg_iov = rVec;
	pRcvMsg->msg_iovlen = sizeof(rVec) / sizeof(struct iovec);

	if (recvmsg(sockfd, pRcvMsg, 0) < 0)
	{
		fprintf(stderr, "server receive message error!\n");
		ret = -1;
		goto end;
	}
	printf("[In server] receive from client, request: %s, size: %d, align: %d\n", 
			(char*)pRcvMsg->msg_iov[0].iov_base, 
			*(int*)pRcvMsg->msg_iov[1].iov_base, 
			*(int*)pRcvMsg->msg_iov[2].iov_base
		  );

	if (0 != strcmp("request_shm", (char*)pRcvMsg->msg_iov[0].iov_base))
	{
		fprintf(stderr, "message error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ServerAllocShmFd(struct msghdr *pRcvMsg, TPAPI_SHM_HANDLE *shmHandle)
{

	int ret = 0;
	int size = *(int*)(pRcvMsg->msg_iov[1].iov_base);
	int align = *(int*)(pRcvMsg->msg_iov[2].iov_base);

	/*server alloc shm, and send share_fd back to client socket.*/
	ret = tpapi_shm_Alloc(size, align, shmHandle);
	if (TPAPI_SHM_OK != ret)
	{
		fprintf(stderr, "shared memory alloc error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ServerSendShmResponse(int sockfd, struct msghdr *pSndMsg, TPAPI_SHM_HANDLE *shmHandle)
{
	int ret = 0;

	memset(pSndMsg, 0, sizeof(struct msghdr));

	memset(ctlBuf, 0, sizeof(ctlBuf));
	pSndMsg->msg_control = ctlBuf;
	pSndMsg->msg_controllen = sizeof(ctlBuf);
	struct cmsghdr *cmsg;
	cmsg = CMSG_FIRSTHDR(pSndMsg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(TPAPI_SHM_HANDLE));
	*(TPAPI_SHM_HANDLE *)CMSG_DATA(cmsg) = *shmHandle;

	memset(sVec[0].iov_base, 0, sizeof(sVec[0].iov_len));
	pSndMsg->msg_iov = sVec;
	pSndMsg->msg_iovlen = 1;
	sprintf((char*)pSndMsg->msg_iov[0].iov_base, "%s", "/dev/ion");//XXX MACRO

	printf("[In server] send to client, dev_name:%s, sharedfd: %d\n", (char*)pSndMsg->msg_iov[0].iov_base , *shmHandle);
	if (-1 == sendmsg(sockfd, pSndMsg, 0))
	{
		fprintf(stderr, "server send shared fd error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ServerRecvShmDataRequest(int sockfd, struct msghdr *pRcvMsg)
{
	int ret = 0;

	memset(pRcvMsg, 0, sizeof(struct msghdr));
	memset(rVec[0].iov_base, 0, sizeof(rVec[0].iov_len));//XXX MACRO INDEX

	pRcvMsg->msg_iov = rVec;
	pRcvMsg->msg_iovlen = 1;

	if (recvmsg(sockfd, pRcvMsg, 0) < 0)
	{
		fprintf(stderr, "server receive message error!\n");
		ret = -1;
		goto end;
	}

	printf("[In server] receive from client, request: %s\n", (char*)pRcvMsg->msg_iov[0].iov_base);

	if (0 != strcmp("request_data", (char*)pRcvMsg->msg_iov[0].iov_base))
	{
		fprintf(stderr, "message error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ServerPutShmData(TPAPI_SHM_HANDLE *shmHandle, char *data, int size)
{
	int ret = 0;
	int offset = 0;

	void *shmPtr = NULL;
	ret = tpapi_shm_GetVirtualAddress(*shmHandle, offset, size, &shmPtr);
	if (TPAPI_SHM_OK != ret)
	{
		fprintf(stderr, "shared memory get virtual address error!\n");
		ret = -1;
		goto end;
	}

	sprintf((char*)shmPtr, "%s", data);
	ret = tpapi_shm_Flush(*shmHandle, offset, size);
	if (TPAPI_SHM_OK != ret)
	{
		fprintf(stderr, "shared memory flush error!\n");
		ret = -1;
		goto end;
	}

	printf("[In server] put data:\"%s\" to shm.\n", (char*)shmPtr);

end:
	return ret;
}

static int ServerSendShmDataResponse(int sockfd, struct msghdr *pSndMsg)
{
	int ret = 0;

	memset(pSndMsg, 0, sizeof(struct msghdr));

	memset(sVec[0].iov_base, 0, sizeof(sVec[0].iov_len));
	pSndMsg->msg_iov = sVec;
	pSndMsg->msg_iovlen = 1;
	sprintf((char*)pSndMsg->msg_iov[0].iov_base, "%s", "response_data");//XXX MACRO

	printf("[In server] reponse to client:%s\n", (char*)pSndMsg->msg_iov[0].iov_base);
	if (-1 == sendmsg(sockfd, pSndMsg, 0))
	{
		fprintf(stderr, "server send response_data error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ClientSendShmRequest(int sockfd, struct msghdr *pSndMsg)
{
	int ret = 0;
	int size = 32;
	int align = 16;

	memset(pSndMsg, 0, sizeof(struct msghdr));
	memset(sVec[0].iov_base, 0, sizeof(sVec[0].iov_len));
	memset(sVec[1].iov_base, 0, sizeof(sVec[1].iov_len));
	memset(sVec[2].iov_base, 0, sizeof(sVec[2].iov_len));
	pSndMsg->msg_iov = sVec;
	pSndMsg->msg_iovlen = sizeof(sVec) / sizeof(struct iovec);
	sprintf((char*)pSndMsg->msg_iov[0].iov_base, "%s", "request_shm");//XXX MACRO
	*((int*)pSndMsg->msg_iov[1].iov_base) = size;
	*((int*)pSndMsg->msg_iov[2].iov_base) = align;

	printf("[In client] send to server, request: %s, size: %d, align: %d\n", (char*)pSndMsg->msg_iov[0].iov_base, size, align);
	if (-1 == sendmsg(sockfd, pSndMsg, 0))
	{
		fprintf(stderr, "client send request error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ClientRecvShmResponse(int sockfd, struct msghdr *pRcvMsg)
{
	int ret = 0;

	memset(pRcvMsg, 0, sizeof(struct msghdr));
	memset(rVec[0].iov_base, 0, sizeof(rVec[0].iov_len));//XXX MACRO INDEX
	memset(ctlBuf, 0, sizeof(ctlBuf));

	pRcvMsg->msg_iov = rVec;
	pRcvMsg->msg_iovlen = 1;

	pRcvMsg->msg_control = ctlBuf;
	pRcvMsg->msg_controllen = sizeof(ctlBuf);

	if (recvmsg(sockfd, pRcvMsg, 0) < 0)
	{
		fprintf(stderr, "client receive message error!\n");
		ret = -1;
		goto end;
	}
	printf("[In client] receive from server, device: %s\n", (char*)pRcvMsg->msg_iov[0].iov_base);

	if (0 != strcmp("/dev/ion", (char*)pRcvMsg->msg_iov[0].iov_base))//XXX MACRO
	{
		fprintf(stderr, "message error!\n");
		ret = -1;
		goto end;
	}
end:
	return ret;
}


static int ClientRetriveShmFd(struct msghdr *pRcvMsg, TPAPI_SHM_HANDLE *shmHandle)
{
	int ret = 0;
	struct cmsghdr *cmsg = CMSG_FIRSTHDR(pRcvMsg);
	if (NULL == cmsg)
	{
		fprintf(stderr, "no cmsg received in client!\n");
		ret = -1;
		goto end;
	}

	*shmHandle = *(int*)CMSG_DATA(cmsg);
	if (*shmHandle < 0)
	{
		fprintf(stderr, "couldn't receive fd from socket.");
		ret = -1;
		goto end;
	}

	printf("[In client] retrive shared fd: %d\n", *shmHandle);

end:
	return ret;
}

static int ClientSendShmDataRequest(int sockfd, struct msghdr *pSndMsg)
{
	int ret = 0;

	memset(pSndMsg, 0, sizeof(struct msghdr));
	memset(sVec[0].iov_base, 0, sizeof(sVec[0].iov_len));
	pSndMsg->msg_iov = sVec;
	pSndMsg->msg_iovlen = 1;
	sprintf((char*)pSndMsg->msg_iov[0].iov_base, "%s", "request_data");//XXX MACRO

	printf("[In client] send to server, request: %s\n", (char*)pSndMsg->msg_iov[0].iov_base);
	if (-1 == sendmsg(sockfd, pSndMsg, 0))
	{
		fprintf(stderr, "client send request_data error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ClientRecvShmDataResponse(int sockfd, struct msghdr *pRcvMsg)
{
	int ret = 0;

	memset(pRcvMsg, 0, sizeof(struct msghdr));
	memset(rVec[0].iov_base, 0, sizeof(rVec[0].iov_len));//XXX MACRO INDEX

	pRcvMsg->msg_iov = rVec;
	pRcvMsg->msg_iovlen = 1;

	if (recvmsg(sockfd, pRcvMsg, 0) < 0)
	{
		fprintf(stderr, "client receive message error!\n");
		ret = -1;
		goto end;
	}

	printf("[In client] receive response from server, response: %s\n", (char*)pRcvMsg->msg_iov[0].iov_base);

	if (0 != strcmp("response_data", (char*)pRcvMsg->msg_iov[0].iov_base))
	{
		fprintf(stderr, "response message error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ClientGetShmData(TPAPI_SHM_HANDLE *shmHandle, char *data, int size)
{

	int ret = 0;
	int offset = 0;//XXX

	memset(data, 0, size);
	void *shmPtr = NULL;

	ret = tpapi_shm_Invalidate(*shmHandle, offset, size);
	if (TPAPI_SHM_OK != ret)
	{
		fprintf(stderr, "shared memory invalidate error!\n");
		ret = -1;
		goto end;
	}

	ret = tpapi_shm_GetVirtualAddress(*shmHandle, offset, size, &shmPtr);
	if (TPAPI_SHM_OK != ret)
	{
		fprintf(stderr, "shared memory get virtual address error!\n");
		ret = -1;
		goto end;
	}
	sprintf(data, "%s", (char*)shmPtr);
	printf("[In client] get data:\"%s\" from shm.\n", data);

end:
	return ret;

}

static int SendErrorNotify(int sockfd, struct msghdr *pSndMsg)
{
	int ret = 0;

	memset(pSndMsg, 0, sizeof(struct msghdr));
	memset(sVec[0].iov_base, 0, sizeof(sVec[0].iov_len));
	pSndMsg->msg_iov = sVec;
	pSndMsg->msg_iovlen = 1;
	sprintf((char*)pSndMsg->msg_iov[0].iov_base, "%s", "error_message");//XXX MACRO

	printf("Send : %s !\n", (char*)pSndMsg->msg_iov[0].iov_base);
	if (-1 == sendmsg(sockfd, pSndMsg, 0))
	{
		fprintf(stderr, "sendmsg error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}
