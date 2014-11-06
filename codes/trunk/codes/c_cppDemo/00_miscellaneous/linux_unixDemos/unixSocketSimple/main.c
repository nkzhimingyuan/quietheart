/*This code shows a simple process communicate with unix socket and select.
Infact, we can use socketpair for unnamed socket between parent/child process.
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <errno.h> 
#include <sys/time.h> 
#include <dirent.h> 


#define NO_WAIT             0 
#define WAIT_FOREVER       -1 
#define DEFAULT_DIR "/tmp/"

static struct sockaddr_un s_UnAddr;
static char socketPath[128];

static int UnixDomainSocketInit(void);
static int UnixDomainSocketCreate();
static int UnixDomainSocketRecv(int sockFd, void *pBuf, unsigned int bufSize, int timeout);
static int UnixDomainSocketSend(int sockFd, void *pBuf, unsigned bufSize, int timeout);

int main(int argc, char *argv[])
{
	int ret = 0;
	int n = 0;
	int sockfd;
	pid_t pid;

	UnixDomainSocketInit();
	sockfd = UnixDomainSocketCreate();	
	if (sockfd < 0)
	{
		ret = -1;
		goto end;
	}

	pid = fork();
	if (pid > 0)
	{//parent
		char sendStr[] = {"hello world, from server."};

		n = UnixDomainSocketSend(sockfd, sendStr, sizeof(sendStr), NO_WAIT);
		printf("In Server Send count:%d, send content:%s\n", n, sendStr);
		if (n < 0)
		{
			ret = -1;
			fprintf(stderr, "send to client error.\n");
			goto end;
		}

		waitpid(pid, NULL, 0);
	}
	else if (0 == pid)
	{//child
		char recvStr[128] = {'\0'};

		n = UnixDomainSocketRecv(sockfd, recvStr, sizeof(recvStr), 5000);
		printf("In Client recv count:%d, recv content:%s\n", n, recvStr);
		if (n < 0)
		{
			ret = -1;
			fprintf(stderr, "recv from server error.\n");
			goto end;
		}
	}
	else
	{
		ret = -1;
	}

end:
	return ret;
}

static int UnixDomainSocketInit(void)
{
	int ret = 0;
	struct sockaddr_un *pAddr = NULL;
	DIR *pdir = NULL;
	FILE *pFile = NULL;
	pdir = opendir(DEFAULT_DIR);
	if (pdir == NULL)
	{
		fprintf(stderr, "create dir %d, %s.\n", errno, strerror(errno));
		ret = -1;
		goto end;
	}
	closedir(pdir);

	sprintf(socketPath, "%s%s.%d", DEFAULT_DIR, "mysocket",getpid());
	pFile = fopen(socketPath, "w+");
	if (pFile == NULL)
	{
		fprintf(stderr, "create file %d, %s.\n", errno, strerror(errno));
		ret = -1;
		goto end;
	}
	fclose(pFile);

	s_UnAddr.sun_family = AF_LOCAL;
	sprintf(s_UnAddr.sun_path, "%s", socketPath);

end:
	return ret;
}

static int UnixDomainSocketCreate()
{
	int ret = 0;
	int sockFd = -1;
	//int bufSize = 0;
	sockFd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (-1 == sockFd)
	{
		fprintf(stderr, "create socket.%d, %s\n", errno, strerror(errno));
		return -1;
	}

	remove(s_UnAddr.sun_path);
	if (-1 == bind(sockFd, (const struct sockaddr *)&s_UnAddr, sizeof(s_UnAddr)))
	{
		fprintf(stderr, "bind error:%d, %s, path:%s\n", errno, strerror(errno), s_UnAddr.sun_path);
		ret = -1;
		close(sockFd);
		goto end;
	}

	/*
	ret = setsockopt(sockFd, SOL_SOCKET, SO_SNDBUF, &bufSize, sizeof(bufSize));
	if (0 != ret)
	{
		fprintf(stderr, "setsockopt %d:%d, %s\n", bufSize, errno, strerror(errno));
		close(sockFd);
		goto end;
	}

	ret = setsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(bufSize));
	if (0 != ret)
	{
		fprintf(stderr, "setsockopt %d:%d, %s\n", bufSize, errno, strerror(errno));
		close(sockFd);
		goto end;
	}
	*/

	ret = sockFd;

end:
	return ret;
}

static int UnixDomainSocketRecv(int sockFd, void *pBuf, unsigned int bufSize, int timeout)
{
	int ret = 0;
	int flags = 0;
	ssize_t n = 0;
	socklen_t addrLen = sizeof(struct sockaddr_un);
	struct sockaddr_un remoteAddr;

	if ((sockFd < 0) || (NULL == pBuf) || (0 == bufSize)) 
	{ 
		ret = -1;
		goto end;
	}     

	if ((NO_WAIT == timeout) || (timeout > 0)) 
	{ 
		flags |= MSG_DONTWAIT; 
	} 

	if (timeout > 0) 
	{ 
		fd_set r; 
		int sr = 0; 
		struct timeval time; 

		FD_ZERO(&r); 
		FD_SET(sockFd, &r); 

		time.tv_usec = ((timeout%1000)*1000); 
		time.tv_sec  = (timeout/1000); 


		sr = select(sockFd + 1, &r, NULL, NULL, &time); 
		if (sr <= 0) 
		{ 
			ret = -1;
			goto end;
		}          
	} 


	n = recvfrom(sockFd, pBuf, bufSize, flags, (struct sockaddr *)&remoteAddr, &addrLen); 
	if (n < 0) 
	{ 
		fprintf(stderr, "sockFd[%d] recvfrom fail:%d, %s", sockFd, errno, strerror(errno)); 
		ret = -1;
		goto end;
	} 
	ret = n;

end:
	return ret;
}

static int UnixDomainSocketSend(int sockFd, void *pBuf, unsigned bufSize, int timeout)
{
	int ret = 0;
    int flags = 0; 
    ssize_t n = 0; 

    if ((sockFd < 0) || (NULL == pBuf) || (0 == bufSize)) 
    { 
		ret = -1;
		goto end;
    } 

 
    if (NO_WAIT == timeout) 
    { 
        flags |= MSG_DONTWAIT; 
    } 
     
    n = sendto(sockFd, pBuf, bufSize, flags, (const struct sockaddr *)&s_UnAddr, sizeof(s_UnAddr)); 
    if (n != bufSize) 
    { 
        fprintf(stderr, "sockFd[%d] send fail:%d, %s", sockFd, errno, strerror(errno)); 
		ret = -1;
		goto end;
    } 
	ret = n;

end:
    return ret; 
}
