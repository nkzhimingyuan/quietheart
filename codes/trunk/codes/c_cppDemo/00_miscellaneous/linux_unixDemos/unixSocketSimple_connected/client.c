/*
This code shows a simple process communicate with unix socket and select.
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <errno.h> 

#include "myunixsocket.h"

#define DEBUG 1
#if DEBUG==1
#define dbg() fprintf(stderr, "Line:%d, func:%s\n", __LINE__, __FUNCTION__)
#else
#define dbg()
#endif

#define SOCKET_SERVER_PATH "/tmp/my_server_socket"
#define SOCKET_CLIENT_PATH "/tmp/my_client_socket"


int main(int argc, char *argv[])
{
	int ret = 0;
	int n = 0;
	int sockFd;
	struct sockaddr_un s_server_addr;
	struct sockaddr_un s_client_addr;
	char sendStr[] = {"hello world, from client."};
	char recvStr[128] = {'\0'};

	ret = UnixDomainInit(&s_server_addr, SOCKET_SERVER_PATH);
	if (ret < 0)
	{
		ret = -1;
		fprintf(stderr, "server domain init error.\n");
		goto end;
	}

	ret = UnixDomainInit(&s_client_addr, SOCKET_CLIENT_PATH);
	if (ret < 0)
	{
		ret = -1;
		fprintf(stderr, "server domain init error.\n");
		goto end;
	}

	sockFd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockFd < 0)
	{
		fprintf(stderr, "create socket error.\n");
		ret = -1;
		goto end;
	}

	ret = bind(sockFd, (const struct sockaddr *)&s_client_addr, sizeof(struct sockaddr_un));
	if (ret < 0)
	{
		fprintf(stderr, "bind error, path:%s\n", s_client_addr.sun_path);
		ret = -1;
		goto end;
	}

	if (connect(sockFd, (struct sockaddr *)&s_server_addr, sizeof(struct sockaddr_un)) < 0)
	{
		fprintf(stderr, "connect error, path:%s\n",  s_server_addr.sun_path);
		close(sockFd);
		ret = -1;
		goto end;
	}

	dbg();
    n = send(sockFd, sendStr, sizeof(sendStr), MSG_DONTWAIT); 
	printf("In Client Send count:%d, send content:%s\n", n, sendStr);
    if (n != sizeof(sendStr)) 
    { 
        fprintf(stderr, "sockFd[%d] send fail, count:%d, error:%s.\n", sockFd, n, strerror(errno)); 
		ret = -1;
		goto end;
    } 

	n = recv(sockFd, recvStr, sizeof(recvStr), 0); 
	printf("In Client recv count:%d, recv content:%s\n", n, recvStr);
	if (n < 0) 
	{ 
		fprintf(stderr, "sockFd[%d] recvfrom fail.\n", sockFd); 
		ret = -1;
		goto end;
	} 

end:
	remove(SOCKET_CLIENT_PATH);
	return ret;
}

