#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <errno.h> 

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

	s_server_addr.sun_family = AF_LOCAL;
	sprintf(s_server_addr.sun_path, "%s", SOCKET_SERVER_PATH);

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

	if (listen(sockFd, MAX_CLI_NUM) < 0)       
	{
		close(sockFd);
		fprintf(stderr, "server socket listen error, errorname:%s.\n", strerror(errno));
		ret = -1;
		goto socket_end;
	}

	while(1)
	{
		if ((clifd = accept(sockFd, (struct sockaddr *)&s_remote_addr, &addrLen)) < 0)    
		{   
			fprintf(stderr, "accept fd:%d fail, error:%s.\n", sockFd, strerror(errno)); 
			ret = -1;
			goto socket_end;
		}  

		n = recv(clifd, recvStr, sizeof(recvStr), 0); 
		printf("In Client recv count:%d, recv content:%s\n", n, recvStr);
		if (n < 0) 
		{ 
			fprintf(stderr, "sockFd[%d] recvfrom fail.\n", sockFd); 
			close(clifd);
			ret = -1;
			goto socket_end;
		} 

		n = send(clifd, sendStr, sizeof(sendStr), MSG_DONTWAIT); 
		printf("In Server Send count:%d, send content:%s\n", n, sendStr);
		if (n != sizeof(sendStr)) 
		{ 
			fprintf(stderr, "sockFd[%d] send fail, count:%d, error:%s.\n", clifd, n, strerror(errno)); 
			close(clifd);
			ret = -1;
			goto socket_end;
		} 
	}

socket_end:
	close(sockFd);
	remove(SOCKET_SERVER_PATH);

end:
	return ret;
}
