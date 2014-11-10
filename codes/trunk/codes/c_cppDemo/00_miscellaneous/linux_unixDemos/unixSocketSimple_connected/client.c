/*
This code shows a simple process communicate with unix socket and select.
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/un.h> 
#include <errno.h> 

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

	s_server_addr.sun_family = AF_LOCAL;
	sprintf(s_server_addr.sun_path, "%s", SOCKET_SERVER_PATH);

	s_client_addr.sun_family = AF_LOCAL;
	sprintf(s_client_addr.sun_path, "%s", SOCKET_CLIENT_PATH);

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
		goto socket_end;
	}

    n = send(sockFd, sendStr, sizeof(sendStr), MSG_DONTWAIT); 
	printf("In Client Send count:%d, send content:%s\n", n, sendStr);
    if (n != sizeof(sendStr)) 
    { 
        fprintf(stderr, "sockFd[%d] send fail, count:%d, error:%s.\n", sockFd, n, strerror(errno)); 
		ret = -1;
		goto socket_end;
    } 

	n = recv(sockFd, recvStr, sizeof(recvStr), 0); 
	printf("In Client recv count:%d, recv content:%s\n", n, recvStr);
	if (n < 0) 
	{ 
		fprintf(stderr, "sockFd[%d] recvfrom fail.\n", sockFd); 
		ret = -1;
		goto socket_end;
	} 

socket_end:
	close(sockFd);
	remove(SOCKET_CLIENT_PATH);

end:
	return ret;
}
