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
	char filePath[64] = {'\0'};
	char pipePath[32] = "./fifo";
	int pipeFd;
	int count = 0;
	pid_t pid;
	char buf[32] = {"hello world, from server!\n"};

	printf("In server\n");

	pipeFd = open(pipePath, O_APPEND |O_RDWR);

	//read(pipeFd, &pid, sizeof(pid_t));
	//printf("In server, the received pid is:%d \n", pid);
	//read(pipeFd, &fd, sizeof(fd));
	//printf("In server, the received file descriptor is:%d \n", fd);
	read(pipeFd, filePath, sizeof(filePath));
	printf("In server, the received file path is:%s \n", filePath);

	//sprintf(filePath, "/proc/%d/fd/%d", pid, fd);
	//printf("In server, get the path of client opened file:%s\n", filePath);

	fd = open(filePath, O_APPEND |O_RDWR);
	printf("In server, the opened file descriptor is:%d \n", fd);

	read(fd, buf, sizeof(buf));
	printf("In server, the read content is:%s\n", buf);

	count = write(fd, buf, sizeof(buf));
	printf("In client, the write content is:%s, the count is:%d\n", buf, count);

	write(pipeFd, &fd, sizeof(fd));


end:
	return 0;
}
