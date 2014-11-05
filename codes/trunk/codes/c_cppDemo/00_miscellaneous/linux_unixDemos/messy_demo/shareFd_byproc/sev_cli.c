#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>

/*
Client:
Open file
write file.
Send file descriptor value.

Server:
Receive file descriptor value 
Get the file path in virtual file system by file descriptor value.
read file and write file.
*/

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd = -1;
	int socketFd[2];
	int count = 0;
	pid_t pid;
	char buf[32] = {'\0'};
	socketpair(AF_UNIX, SOCK_STREAM, 0, socketFd);
	if (argc != 2)
	{
		fprintf(stderr, "Error, argument number:%d\n", argc);
		fprintf(stderr, "Usage:%s <path>", argv[0]);
		ret = -1;
		goto end;
	}

	pid = fork();

	if(pid > 0)
	{
		char filePath[64] = {'\0'};
		printf("In server\n");

		read(socketFd[0], &fd, sizeof(fd));
		printf("In server, the received file descriptor is:%d \n", fd);

		sprintf(filePath, "/proc/%d/fd/%d", pid, fd);
		printf("In server, get the path of client opened file:%s\n", filePath);

		fd = -1;
		fd = open(filePath, O_APPEND |O_RDWR);
		printf("In server, the opened file descriptor is:%d \n", fd);

		read(fd, buf, sizeof(buf));
		printf("In server, the read content is:%s\n", buf);

		strcpy(buf, "hello world, from server!\n");
		count = write(fd, buf, sizeof(buf));
		printf("In client, the write content is:%s, the count is:%d\n", buf, count);

		write(socketFd[0], &fd, sizeof(fd));

	}
	else if (0 == pid)
	{
		printf("In client\n");
		strcpy(buf, "hello world, from client!\n");
		fd = open(argv[1], O_APPEND |O_RDWR);
		printf("In client, the opened file descriptor is:%d \n", fd);

		count = write(fd, buf, sizeof(buf));
		printf("In client, the write content is:%s\n", buf);

		write(socketFd[1], &fd, sizeof(fd));

		read(socketFd[1], &fd, sizeof(fd));
		printf("In client, the received file descriptor is:%d \n", fd);
	}
	else
	{
		ret = -1;
		fprintf(stderr, "Error, fork.\n");
		goto end;
	}
	

end:
	return 0;
}
