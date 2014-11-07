#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <errno.h> 
#include <sys/time.h> 
#include <dirent.h> 
#include <fcntl.h>

#include "myunixsocket.h"
int UnixDomainInit(struct sockaddr_un *sockAddr, char *socketPath)
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

	sockAddr->sun_family = AF_LOCAL;
	sprintf(sockAddr->sun_path, "%s", socketPath);

end:
	return ret;
}

int UnixDomainSocket(int *socketFd)
{
	int ret = 0;
	int sockFd = -1;

	sockFd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (-1 == sockFd)
	{
		fprintf(stderr, "create socket.%d, %s\n", errno, strerror(errno));
		ret = -1;
	}
	*socketFd = sockFd;

end:
	return ret;
}

int UnixDomainSocketBind(int socketFd, struct sockaddr_un *bindAddr)
{
	int ret = 0;

	if(bindAddr != NULL)
	{
		if (0 == access(bindAddr->sun_path, F_OK))
		{/*check if the file already exists, as we can't bind a socket to an existing file.*/
			fprintf(stderr, "Warning socket file:%s Already exists, don't bind more than once.\n", bindAddr->sun_path);
		}

		if (-1 == bind(socketFd, (const struct sockaddr *)bindAddr, sizeof(struct sockaddr_un)))
		{
			fprintf(stderr, "bind error:%d, %s, path:%s\n", errno, strerror(errno), bindAddr->sun_path);
			ret = -1;
			goto end;
		}
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
end:
	return ret;
}

int UnixDomainSocketRecvFrom(int sockFd, struct sockaddr_un *remoteAddr, void *pBuf, unsigned int bufSize, int timeout)
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

int UnixDomainSocketSendTo(int sockFd, struct sockaddr_un *remoteAddr, void *pBuf, unsigned int bufSize, int timeout)
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

int UnixDomainUnInit(struct sockaddr_un *sockAddr)
{
	int ret = 0;
	
	remove(sockAddr->sun_path);//remove the socket file.
end:
	return ret;
}
