/*--------------------------------------------------------------------------------
 * Copyright (c) 2013 TPV Technology Group
 * All rights reserved
 *
 * Unauthorized use, practice, perform, copy, distribution, reproduction,
 * or disclose of this information in whole or in part is prohibited
 *
 * -------------------------------------------------------------------------*/
/**@file tpapi_shm_test.cpp
 *@brief brief description
 *@author miracle.lv
 *@date 2014-11-6
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h> 
#include <sys/time.h> 
#include <pthread.h>
#include <errno.h> 
#include <fcntl.h>

/*
Server:
create socket, bind socket(if path not exists).
1.thread1 write fd, send fd to socket, and set fd to pipe, and exists.
2.thread2 listen socket, accept for fd receive.
or (exists)


Client:
1.read pipe to get socket path, connect to socket and get the fd.
*/


/************* Declaration and Macros*************/
/*For debug*/
#define UTIL_DBG 0
#if UTIL_DBG == 1
#define dbg(...) do{\
	fprintf(stderr, "Function:%s, Line:%d,  ", __func__, __LINE__);\
	fprintf(stderr, __VA_ARGS__);\
			}while(0)
#else
#define dbg(...)
#endif

#define MAX_CLI_NUM 5
#define FIFO_FILE "./fifo"
#define TEST_FILE "./mytest"

#define WAIT_CONDITION(value) do {\
	while(value != condition)\
	{\
		pthread_cond_wait(&socketReady, &daemoLock);\
	}\
	sleep(1);\
}while(0)

#define NOTIFY_CONDITION(value) do {\
	pthread_mutex_lock(&daemoLock);\
	condition = 1;\
	pthread_mutex_unlock(&daemoLock);\
	pthread_cond_signal(&socketReady);\
}while(0)

typedef struct FdHandle_t
{
	pid_t pid;
	int fd;
}FdHandle;

static pthread_mutex_t daemoLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t socketReady = PTHREAD_COND_INITIALIZER;
static int condition = 0;

/***********Function declaration.************/
static int SendFd(int sockfd, int fd_to_send);
static int ReceiveFd(int sockfd, int *fd_to_receive);

static int SendFdPipe(char *fifoPath, int fd_to_send);
static int ReceiveFdPipe(char *fifoPath, int *fd_to_receive);

static int ServerRun();
static int ClientRun();
static void *demonThread();


/*************Main function********************/
int main(int argc, char *argv[])
{
	int ret = -1;
	int childStatus = -1;
	int fd = -1;
	char buf[128] = {"hello world from server.\n"};
	pid_t pid;

	fd = open(TEST_FILE, O_CREAT |O_RDWR);
	if (fd < 0)
	{
		ret = -1;
		fprintf(stderr, "open fd fail!\n");
		goto end;
	}
	
	if(write(fd, buf, sizeof(buf)) < 0)
	{
		ret = -1;
		fprintf(stderr, "Server write fd fail!\n");
		goto end;
	}
	close(fd);
	printf("Write to file %s: %s\n", TEST_FILE, buf);


	pid = fork();
	if (pid > 0)
	{/*parent*/
		ret = ServerRun();
		wait(&childStatus);
	}
	else if (0 == pid)
	{/*child*/
		ret = ClientRun();
	}

end:
	if (0 == childStatus && 0 == ret)
	{
		printf("TEST PASS.\n");
	}
	return ret;
}

/***********Implementation functions****************/
static int ServerRun()
{
	int ret = 0;
	pthread_t tid;

	int fd_to_send = -1;

	/*normal file to be sent.*/
	fd_to_send = open(TEST_FILE, O_APPEND|O_RDWR);
	if (fd_to_send < 0)
	{
		ret = -1;
		fprintf(stderr, "Server open fd fail!\n");
		goto end;
	}
	printf("[In server] open the file:%s, The opened fd is:%d.\n", TEST_FILE, fd_to_send);

	/*daemon thread for send fd with socket.*/
	if(0 != pthread_create(&tid, NULL, demonThread, NULL))
	{
		ret = -1;
		fprintf(stderr, "Create demon thread for send fd in server error!\n");
		goto end;
	}

	WAIT_CONDITION(1);
	/*send fd*/
	printf("[In server] send the fd to client.\n");
	dbg("Server before send fd:%d\n", fd_to_send);
	if(0 != SendFdPipe(FIFO_FILE, fd_to_send))
	{
		ret = -1;
		fprintf(stderr, "Send fd in server error!\n");
	}
	dbg("Server after send fd:%d\n", fd_to_send);
	
end:
	pthread_join(tid,NULL);
	return ret;
}

static int ClientRun()
{
	int ret = 0;
	int fd_to_receive = -1;
	char buf[128] = {'\0'};

	dbg("Client before receive fd\n");
	if(0 != ReceiveFdPipe(FIFO_FILE, &fd_to_receive))
	{
		ret = -1;
		fprintf(stderr, "Receive fd in client error!\n");
		goto end;
	}
	dbg("Client after receive fd:%d\n", fd_to_receive);
	printf("[In client] received the fd, The received fd is:%d.\n", fd_to_receive);

	if(read(fd_to_receive, buf, sizeof(buf)) < 0)
	{
		ret = -1;
		fprintf(stderr, "Client Read fd fail!\n");
		goto end;
	}

	fprintf(stderr, "[In client] read content from the received fd:%s\n", buf);

end:
	return ret;
}

static void *demonThread()
{
	int n = 0;
	int ret = 0;
	int loopCount = 1;

	pid_t ownerPid;
	int fd_to_send = -1;

	int sockFd;
	char socketPath[128]={'\0'};
	struct sockaddr_un serverAddr;

	int cliFd;
	int addrLen = 0;
	struct sockaddr_un remoteAddr;


	/*socket file initialize.*/
	ownerPid = getpid();
	serverAddr.sun_family = AF_LOCAL;
	sprintf(serverAddr.sun_path, "/tmp/server_%d_socket", ownerPid);

	sockFd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockFd < 0)
	{
		fprintf(stderr, "create socket error.\n");
		ret = -1;
		goto end;
	}

	ret = bind(sockFd, (const struct sockaddr *)&serverAddr, sizeof(struct sockaddr_un));
	if (ret < 0)
	{
		fprintf(stderr, "bind error, path:%s\n", serverAddr.sun_path);
		ret = -1;
		goto end;
	}

	if (listen(sockFd, MAX_CLI_NUM) < 0)       
	{
		close(sockFd);
		fprintf(stderr, "server socket listen error, errorname:%s.\n", strerror(errno));
		ret = -1;
		goto socket_end;
	}

	NOTIFY_CONDITION(1);

	/*loop for accept.*/
	while(loopCount)
	{
		if ((cliFd = accept(sockFd, (struct sockaddr *)&remoteAddr, &addrLen)) < 0)    
		{   
			fprintf(stderr, "accept fd:%d fail, error:%s.\n", sockFd, strerror(errno)); 
			ret = -1;
			goto socket_end;
		}  

		n = recv(cliFd, &fd_to_send, sizeof(int), 0); 
		dbg("In server recv fd_to_send:%d\n", fd_to_send);
		if (n < 0) 
		{ 
			fprintf(stderr, "sockFd[%d] recvfrom fail.\n", sockFd); 
			ret = -1;
			goto end;
		} 

		/*If we read fd_to_send here, it will effects the client application's file offset.*/

		if(SendFd(cliFd, fd_to_send) < 0)
		{
			fprintf(stderr, "Send fd fail.\n"); 
			goto socket_end;
		}
		loopCount--;
	}

socket_end:
	close(sockFd);
	remove(serverAddr.sun_path);

end:
	return;
}

static int SendFdPipe(char *fifoPath, int fd_to_send)
{
	int ret = 0;
	FdHandle handleToSend;
	pid_t owner_pid;
	int fifoFd = 0;

	if ((fifoFd = open(fifoPath, O_RDWR)) < 0)
	{
		fprintf(stderr, "open fifo file error.\n");
		ret = -1;
		goto end;
	}

	handleToSend.pid = getpid();
	handleToSend.fd = fd_to_send;

	if ((write(fifoFd, &handleToSend, sizeof(FdHandle))) < 0)
	{
		fprintf(stderr, "write handle to fifo error.\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ReceiveFdPipe(char *fifoPath, int *fd_to_receive)
{
	int ret = 0;
	int n = 0;
	int fifoFd = -1;
	//int fd_to_receive = -1;
	FdHandle handleToReceive;

	pid_t socketPid;
	int sockFd;
	struct sockaddr_un serverAddr;
	struct sockaddr_un clientAddr;
	char socketPath[128]={'\0'};

	fifoFd = open(fifoPath, O_RDWR);
	if ((read(fifoFd, &handleToReceive, sizeof(FdHandle))) < 0)
	{
		fprintf(stderr, "read server handle from fifo error!\n");
		ret = -1;
		goto end;
	}
	dbg("In client recv server's fd_to_send:%d\n", handleToReceive.fd);

	serverAddr.sun_family = AF_LOCAL;
	sprintf(serverAddr.sun_path, "/tmp/server_%d_socket", handleToReceive.pid);

	socketPid = getpid();
	clientAddr.sun_family = AF_LOCAL;
	sprintf(clientAddr.sun_path, "/tmp/client_%d_socket", socketPid);

	if ((sockFd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "create socket error.\n");
		ret = -1;
		goto end;
	}

	if ((ret = bind(sockFd, (const struct sockaddr *)&clientAddr, sizeof(struct sockaddr_un))) < 0)
	{
		fprintf(stderr, "bind error, path:%s\n", clientAddr.sun_path);
		ret = -1;
		goto end;
	}

	if (connect(sockFd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_un)) < 0)
	{
		fprintf(stderr, "connect error, path:%s\n",  serverAddr.sun_path);
		close(sockFd);
		ret = -1;
		goto socket_end;
	}

    if ((n = send(sockFd, &handleToReceive.fd, sizeof(int), MSG_DONTWAIT)) != sizeof(int))
    { 
        fprintf(stderr, "sockFd[%d] send fail, count:%d, error:%s.\n", sockFd, n, strerror(errno)); 
		ret = -1;
		goto socket_end;
    } 

	if (ReceiveFd(sockFd, fd_to_receive) < 0)
	{
        fprintf(stderr, "Receive fd from socket fail.\n"); 
		ret = -1;
		goto socket_end;
	}

socket_end:
	close(sockFd);
	remove(clientAddr.sun_path);

end:
	return ret;
}

static int SendFd(int sockfd, int fd_to_send)
{
	int ret = 0;
	struct msghdr sndMsg;
	struct iovec sVec[1];
	int sVecInt;

	memset(&sndMsg, 0, sizeof(struct msghdr));

	sVecInt = 0;
	sVec[0].iov_base = &sVecInt;
	sVec[0].iov_len = sizeof(sVecInt);
	sndMsg.msg_iov = sVec;
	sndMsg.msg_iovlen = sizeof(sVec) / sizeof(struct iovec);

	if (fd_to_send < 0)
	{
		fprintf(stderr, "send negative fd: %d!\n", fd_to_send);
		sndMsg.msg_control = NULL;
		sndMsg.msg_controllen = 0;

		sVecInt = fd_to_send;
		ret = -1;
	}
	else
	{
		struct cmsghdr *cmsg;
		char ctlBuf[CMSG_SPACE(sizeof(int))];
		memset(ctlBuf, 0, sizeof(ctlBuf));
		sndMsg.msg_control = ctlBuf;
		sndMsg.msg_controllen = sizeof(ctlBuf);
		cmsg = CMSG_FIRSTHDR(&sndMsg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		*(int *)CMSG_DATA(cmsg) = fd_to_send;

		sVecInt = 0;
	}

	if (-1 == sendmsg(sockfd, &sndMsg, 0))
	{
		fprintf(stderr, "server send shared fd error!\n");
		ret = -1;
		goto end;
	}

end:
	return ret;
}

static int ReceiveFd(int sockfd, int *fd_to_receive)
{
	int ret = 0;

	struct msghdr rcvMsg;
	struct cmsghdr *cmsg;
	char ctlBuf[CMSG_SPACE(sizeof(int))];
	struct iovec rVec[1];
	int rVecInt;

	memset(&rcvMsg, 0, sizeof(struct msghdr));

	rVecInt = 0;
	rVec[0].iov_base = &rVecInt;
	rVec[0].iov_len = sizeof(rVecInt);
	rcvMsg.msg_iov = rVec;
	rcvMsg.msg_iovlen = sizeof(rVec) / sizeof(struct iovec);


	memset(ctlBuf, 0, sizeof(ctlBuf));
	rcvMsg.msg_control = ctlBuf;
	rcvMsg.msg_controllen = sizeof(ctlBuf);

	if (recvmsg(sockfd, &rcvMsg, 0) < 0)
	{
		fprintf(stderr, "client receive message error!\n");
		ret = -1;
		goto end;
	}

	//if (*(int*)rcvMsg.msg_iov[0].iov_base < 0)
	if (rVecInt < 0)
	{
		fprintf(stderr, "receive negative fd: %d!\n", rVecInt);
		*fd_to_receive = rVecInt;
		ret = -1;
	}
	else
	{
		struct cmsghdr *cmsg = CMSG_FIRSTHDR(&rcvMsg);
		if (NULL == cmsg)
		{
			fprintf(stderr, "no cmsg received in client!\n");
			ret = -1;
			goto end;
		}
		*fd_to_receive = *(int *)CMSG_DATA(cmsg);
	}
end:
	return ret;
}
