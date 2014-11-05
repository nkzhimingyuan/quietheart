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
	char buf[32] = {"hello world, from client!\n"};
	if (argc != 2)
	{
		fprintf(stderr, "Error, argument number:%d\n", argc);
		fprintf(stderr, "Usage:%s <path>", argv[0]);
		ret = -1;
		goto end;
	}

	pipeFd = open(pipePath, O_APPEND |O_RDWR);

	printf("In client\n");
	fd = open(argv[1], O_APPEND |O_RDWR);
	printf("In client, the opened file descriptor is:%d \n", fd);

	count = write(fd, buf, sizeof(buf));
	printf("In client, the write content is:%s\n", buf);

	sprintf(filePath, "/proc/%d/fd/%d", pid, fd);
	write(pipeFd, filePath, sizeof(filePath));
	printf("In Client, send the virtual path of opened file:%s\n", filePath);

	read(pipeFd, &fd, sizeof(fd));
	printf("In client, the received file descriptor value is:%d \n", fd);

end:
	return 0;
}

