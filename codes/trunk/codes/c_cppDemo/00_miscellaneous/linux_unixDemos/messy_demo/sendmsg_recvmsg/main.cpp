/**Test usage of sendmsg/recvmsg.
Server is parent, client is child.
*client send "request_pipe" to socket.
*server recv "request_pipe" from socket.
*server alloc pipe, and send pipe_fd back to client socket.
*client recv pipe_fd from socket.
*client send "request_data" to socket.
*server recv "request_data" from socket.
*server put "Server Data" to pipe which is mmap from shared_fd, and send back to client socket.
*client get "Server Data" from pipe.

 * */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int ret = 0;
	pid_t pid;
	int sd[2];

	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sd);/*sd[0] used for parent read/write, sd[1] used for child read/write.*/
	
	if (-1 == ret)
	{
		fprintf(stderr, "socketpair error!\n");
		goto InitEnd;
	}

	pid = fork();
	if (pid > 0)
	{/*parent*/
		/*receive request_pipe from client.*/
		struct msghdr rcvMsg;
		memset(&rcvMsg, 0, sizeof(struct msghdr));

		char request[16] = {'\0'};
		int size = 0;
		int align = 0;
		struct iovec io_vec[3];
		io_vec[0].iov_base = request;
		io_vec[0].iov_len = sizeof(request);
		io_vec[1].iov_base = &size;
		io_vec[1].iov_len = sizeof(int);
		io_vec[2].iov_base = &align;
		io_vec[2].iov_len = sizeof(int);
		rcvMsg.msg_iov = io_vec;
		rcvMsg.msg_iovlen = sizeof(io_vec) / sizeof(struct iovec);

		if (recvmsg(sd[0], &rcvMsg, 0) < 0)
		{
			fprintf(stderr, "server receive message error!\n");
		}
		printf("[In server] receive from client, request: %s, size: %d, align: %d\n", request, size, align);

		/*create pipe_fd*/
		char pipeName[16] = "./pipe_file";
		int pfd = -1;
		if (0 == strcmp("request_pipe", request))
		{
			mkfifo(pipeName, S_IRWXU);
		}

		pfd = open(pipeName, O_RDWR);
		if (pfd < 0)
		{
			fprintf(stderr, "server create pipe error!\n");
		}

		/*send pipe_fd to client*/
		struct msghdr sndMsg;
		memset(&sndMsg, 0, sizeof(struct msghdr));

		io_vec[0].iov_base = pipeName;
		io_vec[0].iov_len = sizeof(pipeName);
		sndMsg.msg_iov = io_vec;
		sndMsg.msg_iovlen = 1;
		char ctlBuf[CMSG_SPACE(sizeof(int))];
		sndMsg.msg_control = ctlBuf;
		sndMsg.msg_controllen = sizeof(ctlBuf);
		struct cmsghdr *cmsg;
		cmsg = CMSG_FIRSTHDR(&sndMsg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		*(int *)CMSG_DATA(cmsg) = pfd;

		printf("[In server] send to client, pipe_name:%s, pfd: %d\n", pipeName, pfd);
		if (-1 == sendmsg(sd[0], &sndMsg, 0))
		{
			fprintf(stderr, "server send pipe fd error!\n");
		}

		/*server receive request_data from client*/
		memset(&rcvMsg, 0, sizeof(struct msghdr));
		memset(request, 0, sizeof(request));

		io_vec[0].iov_base = request;
		io_vec[0].iov_len = sizeof(request);
		rcvMsg.msg_iov = io_vec;
		rcvMsg.msg_iovlen = 1;

		if (recvmsg(sd[0], &rcvMsg, 0) < 0)
		{
			fprintf(stderr, "server receive message error!\n");
		}
		printf("[In server] receive from client, request: %s\n", request);

		/*server put data to pipe.*/
		memset(request, 0, sizeof(request));
		sprintf(request, "Server Data.");

		printf("[In server] put data:\"%s\" to pipe.\n", request);
		if (write(pfd, request, sizeof(request)) < 0)
		{
			fprintf(stderr, "server write to pipe error!\n");
		}
		

		waitpid(pid, NULL, 0);
		close(pfd);
		unlink(pipeName);
		printf("[In server] Server exit.\n");

	}
	else if (0 == pid)
	{/*child*/
		/*client send request to server.*/
		struct msghdr sndMsg;
		memset(&sndMsg, 0, sizeof(struct msghdr));

		char request[16]="request_pipe";
		int size = 64;
		int align = 32;
		struct iovec io_vec[3];
		io_vec[0].iov_base = request;
		io_vec[0].iov_len = sizeof(request);
		io_vec[1].iov_base = &size;
		io_vec[1].iov_len = sizeof(int);
		io_vec[2].iov_base = &align;
		io_vec[2].iov_len = sizeof(int);
		sndMsg.msg_iov = io_vec;
		sndMsg.msg_iovlen = sizeof(io_vec) / sizeof(struct iovec);

		printf("[In client] send to server, request: %s, size: %d, align: %d\n", request, size, align);
		if (-1 == sendmsg(sd[1], &sndMsg, 0))
		{
			fprintf(stderr, "client send request_shm error!\n");
		}


		/*client receive pipe fd from server.*/
		struct msghdr rcvMsg;
		memset(&rcvMsg, 0, sizeof(struct msghdr));

		char pipeName[16] = {'\0'};
		io_vec[0].iov_base = pipeName;
		io_vec[0].iov_len = sizeof(pipeName);
		rcvMsg.msg_iov = io_vec;
		rcvMsg.msg_iovlen = 1;
		char ctlBuf[CMSG_SPACE(sizeof(int))];
		rcvMsg.msg_control = ctlBuf;
		rcvMsg.msg_controllen = sizeof(ctlBuf);

		if (recvmsg(sd[1], &rcvMsg, 0) < 0)
		{
			fprintf(stderr, "client receive fd error!\n");
		}

		struct cmsghdr *cmsg = CMSG_FIRSTHDR(&rcvMsg);
		if (NULL == cmsg)
		{
			fprintf(stderr, "no cmsg received in client!\n");
		}
		int recvfd =  *(int*)CMSG_DATA(cmsg);
		if (recvfd < 0)
		{
			fprintf(stderr, "couldn't receive fd from socket.");
		}

		printf("[In client] receive from server, pipeName: %s, pipe fd: %d\n", pipeName, recvfd);

		/*Client send request_data to server.*/
		memset(&sndMsg, 0, sizeof(struct msghdr));

		memset(request, 0, sizeof(request));
		sprintf(request,"%s","request_data");
		io_vec[0].iov_base = request;
		io_vec[0].iov_len = sizeof(request);
		sndMsg.msg_iov = io_vec;
		sndMsg.msg_iovlen = 1;

		printf("[In client] send to server, request :%s\n", request);
		if (-1 == sendmsg(sd[1], &sndMsg, 0))
		{
			fprintf(stderr, "client send request_data error!\n");
		}

		/*Client read data from pipe whose pfd is received by server*/
		memset(request, 0, sizeof(request));
		if(read(recvfd, request, sizeof(request))< 0)
		{
			fprintf(stderr, "client read data from pipe error!\n");
		}
		printf("[In client] get data:\"%s\" from pipe.\n", request);

		close(recvfd);
		unlink(pipeName);
		printf("[In client] Client exit.\n");
	}

InitEnd:
	return ret;
}
