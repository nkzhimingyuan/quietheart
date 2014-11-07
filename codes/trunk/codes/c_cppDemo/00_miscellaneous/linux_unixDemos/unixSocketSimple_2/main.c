/*
This code shows a simple process communicate with unix socket and select.
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <errno.h> 
#include <sys/time.h> 
#include <dirent.h> 
#include <fcntl.h>


#define NO_WAIT             0 
#define WAIT_FOREVER       -1 
#define DEFAULT_DIR "/tmp"
#define SOCKET_SERVER_PATH "/tmp/my_server_socket"
#define SOCKET_CLIENT_PATH "/tmp/my_client_socket"

static struct sockaddr_un s_server_addr;
static struct sockaddr_un s_client_addr;

static int UnixDomainInit();
static int UnixDomainServerSocket(int *socketFd);
static int UnixDomainClientSocket(int *socketFd);
static int UnixDomainSocketRecvFrom(int sockFd, struct sockaddr_un *remoteAddr, void *pBuf, unsigned int bufSize, int timeout);
static int UnixDomainSocketSendTo(int sockFd, struct sockaddr_un *remoteAddr, void *pBuf, unsigned int bufSize, int timeout);
static int UnixDomainUnInit();

int main(int argc, char *argv[])
{
	int ret = 0;
	int n = 0;
	int sockfd;
	pid_t pid;

	UnixDomainInit();

	pid = fork();
	if (pid > 0)
	{//parent
		char recvStr[128];
		char sendStr[] = {"hello world, from server."};

		ret = UnixDomainServerSocket(&sockfd);
		if (ret < 0 || sockfd < 0)
		{
			ret = -1;
			fprintf(stderr, "socket init error.\n");
			goto end;
		}

		n = UnixDomainSocketRecvFrom(sockfd, &s_client_addr, recvStr, sizeof(recvStr), WAIT_FOREVER);
		printf("In Server recv count:%d, recv content:%s\n", n, recvStr);
		if (n < 0)
		{
			ret = -1;
			fprintf(stderr, "recv from server error.\n");
			goto end;
		}

		n = UnixDomainSocketSendTo(sockfd, &s_client_addr, sendStr, sizeof(sendStr), NO_WAIT);
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
		char sendStr[] = {"hello world, from client."};
		char recvStr[128] = {'\0'};

		ret = UnixDomainClientSocket(&sockfd);
		if (ret < 0 || sockfd < 0)
		{
			ret = -1;
			fprintf(stderr, "socket init error.\n");
			goto end;
		}

		n = UnixDomainSocketSendTo(sockfd, &s_server_addr, sendStr, sizeof(sendStr), NO_WAIT);
		printf("In Client Send count:%d, send content:%s\n", n, sendStr);
		if (n < 0)
		{
			ret = -1;
			fprintf(stderr, "send to client error.\n");
			goto end;
		}

		n = UnixDomainSocketRecvFrom(sockfd, &s_server_addr, recvStr, sizeof(recvStr), 5000);
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
	UnixDomainUnInit();
	return ret;
}

static int UnixDomainInit()
{
	int ret = 0;
	int sockFd = -1;
	DIR *pdir = NULL;

	pdir = opendir(DEFAULT_DIR);
	if (pdir == NULL)
	{
		fprintf(stderr, "create dir %d, %s.\n", errno, strerror(errno));
		ret = -1;
		goto end;
	}
	closedir(pdir);

	s_server_addr.sun_family = AF_LOCAL;
	sprintf(s_server_addr.sun_path, "%s", SOCKET_SERVER_PATH);

	s_client_addr.sun_family = AF_LOCAL;
	sprintf(s_client_addr.sun_path, "%s", SOCKET_CLIENT_PATH);

	if (0 == access(SOCKET_SERVER_PATH, F_OK))
	{/*check if the file already exists, as we can't bind a socket to an existing file.*/
		fprintf(stderr, "Warning server socket file Already exists, don't bind more than once.\n");
	}

	if (0 == access(SOCKET_CLIENT_PATH, F_OK))
	{/*check if the file already exists, as we can't bind a socket to an existing file.*/
		fprintf(stderr, "Warning client socket file Already exists, don't bind more than once.\n");
	}

end:
	return ret;
}

static int UnixDomainServerSocket(int *socketFd)
{
	int ret = 0;
	int sockFd = -1;

	sockFd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (-1 == sockFd)
	{
		fprintf(stderr, "create socket.%d, %s\n", errno, strerror(errno));
		return -1;
	}

	if (-1 == bind(sockFd, (const struct sockaddr *)&s_server_addr, sizeof(s_server_addr)))
	{
		fprintf(stderr, "bind error:%d, %s, path:%s\n", errno, strerror(errno), s_server_addr.sun_path);
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
	*socketFd = sockFd;
end:
	return ret;
}

static int UnixDomainClientSocket(int *socketFd)
{
	int ret = 0;
	int sockFd = -1;
	sockFd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (-1 == sockFd)
	{
		fprintf(stderr, "create socket.%d, %s\n", errno, strerror(errno));
		ret = -1;
		goto end;
	}
	*socketFd = sockFd;

	if (-1 == bind(sockFd, (const struct sockaddr *)&s_client_addr, sizeof(s_client_addr)))
	{
		fprintf(stderr, "bind error:%d, %s, path:%s\n", errno, strerror(errno), s_client_addr.sun_path);
		ret = -1;
		close(sockFd);
		goto end;
	}
end:
	return ret;
}

static int UnixDomainSocketRecvFrom(int sockFd, struct sockaddr_un *remoteAddr, void *pBuf, unsigned int bufSize, int timeout)
{
	int ret = 0;
	int flags = 0;
	ssize_t n = 0;
	socklen_t addrLen = sizeof(struct sockaddr_un);

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

	n = recvfrom(sockFd, pBuf, bufSize, flags, (struct sockaddr *)remoteAddr, &addrLen); 
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

static int UnixDomainSocketSendTo(int sockFd, struct sockaddr_un *remoteAddr, void *pBuf, unsigned int bufSize, int timeout)
{
	int ret = 0;
    int flags = 0; 
    ssize_t n = 0; 
	socklen_t addrLen = sizeof(struct sockaddr_un);

    if ((sockFd < 0) || (NULL == pBuf) || (0 == bufSize)) 
    { 
		ret = -1;
		goto end;
    } 

 
    if (NO_WAIT == timeout) 
    { 
        flags |= MSG_DONTWAIT; 
    } 
     
    n = sendto(sockFd, pBuf, bufSize, flags, (const struct sockaddr *)remoteAddr, addrLen); 
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

static int UnixDomainUnInit()
{
	int ret = 0;
	
	remove(s_server_addr.sun_path);//remove the socket file.
	remove(s_client_addr.sun_path);//remove the socket file.
end:
	return ret;
}
