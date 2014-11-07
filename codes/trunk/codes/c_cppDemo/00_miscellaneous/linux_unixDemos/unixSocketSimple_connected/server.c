#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <errno.h> 
#include "myunixsocket.h"

#define DEBUG 0
#if DEBUG==1
#define dbg() fprintf(stderr, "Line:%d, func:%s\n", __LINE__, __FUNCTION__)
#else
#define dbg()
#endif

#define SOCKET_SERVER_PATH "/tmp/my_server_socket"
#define MAX_CLI_NUM 5
int main(int argc, char *argv[])
{
	int ret = 0;
	int n = 0;
	int sockFd;
	int clifd;
	struct sockaddr_un s_server_addr;
	struct sockaddr_un s_remote_addr;
	int addrLen = 0;
	char recvStr[128] = {'\0'};
	char sendStr[] = {"hello world, from server."};

	ret = UnixDomainInit(&s_server_addr, SOCKET_SERVER_PATH);
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

	ret = bind(sockFd, (const struct sockaddr *)&s_server_addr, sizeof(struct sockaddr_un));
	if (ret < 0)
	{
		fprintf(stderr, "bind error, path:%s\n", s_server_addr.sun_path);
		ret = -1;
		goto end;
	}

	dbg();
	if (listen(sockFd, MAX_CLI_NUM) < 0)       
	{
		close(sockFd);
		fprintf(stderr, "server socket listen error, errorname:%s.\n", strerror(errno));
		ret = -1;
		goto end;
	}
	dbg();

	if ((clifd = accept(sockFd, (struct sockaddr *)&s_remote_addr, &addrLen)) < 0)    
	{   
		fprintf(stderr, "accept fd:%d fail, error:%s.\n", sockFd, strerror(errno)); 
		close(sockFd);
		ret = -1;
		goto end;
	}  
	dbg();

	n = recv(clifd, recvStr, sizeof(recvStr), 0); 
	printf("In Client recv count:%d, recv content:%s\n", n, recvStr);
	if (n < 0) 
	{ 
		fprintf(stderr, "sockFd[%d] recvfrom fail.\n", sockFd); 
		ret = -1;
		goto end;
	} 

	dbg();
    n = send(clifd, sendStr, sizeof(sendStr), MSG_DONTWAIT); 
	printf("In Server Send count:%d, send content:%s\n", n, sendStr);
    if (n != sizeof(sendStr)) 
    { 
        fprintf(stderr, "sockFd[%d] send fail, count:%d, error:%s.\n", clifd, n, strerror(errno)); 
		ret = -1;
		goto end;
    } 

end:
	remove(SOCKET_SERVER_PATH);
	return ret;
}
