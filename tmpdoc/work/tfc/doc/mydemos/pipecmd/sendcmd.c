#include <stdio.h>
#include <fcntl.h>
#define MSG_SIZE     512
#define PIPE_CMD     "/tmp/pip_cmd"
#define PIPE_ECHO    "/tmp/pip_echo"
#define PIPE_LOG     "/tmp/pip_log"

static int sendCmd(const char *cmd, int len);

int main(int argc, char *argv[])
{
    char cmd1[] = "echo this is command1";
    char cmd2[] = "ls aaa";
    char cmd3[] = "sleep 2";
    char cmd4[] = "echo this is command4;ls";
    char cmd5[] = "echo this is command5;echo what is it? |sed 's/what/it/g'>ttt";
    int status = -1;

    status = sendCmd(cmd1, sizeof(cmd1));
    fprintf(stderr, "end1 status:%d.\n", status);

    status = sendCmd(cmd2, sizeof(cmd2));
    fprintf(stderr, "end2 status:%d.\n", status);

    status = sendCmd(cmd3, sizeof(cmd3));
    fprintf(stderr, "end3 status:%d.\n", status);

    status = sendCmd(cmd4, sizeof(cmd4));
    fprintf(stderr, "end4 status:%d.\n", status);

    status = sendCmd(cmd5, sizeof(cmd5));
    fprintf(stderr, "end5 status:%d.\n", status);
}

static int sendCmd(const char *cmd, int len)
{
    int pipe_cmd = -1;
    int pipe_echo  = -1;
    int ret = -1;

    pipe_cmd = open(PIPE_CMD, O_RDWR | O_NONBLOCK);
    pipe_echo = open(PIPE_ECHO, O_RDWR);

    fprintf(stderr, "cmd:%s, length:%d\n", cmd, len);

    if (pipe_cmd < 0 )
    {
        printf("open %s err\n", PIPE_CMD);
        perror("open");
        return ret;
    }
    if (pipe_echo < 0)
    {
        printf("open %s err\n", PIPE_ECHO);
        perror("open");
        return ret;
    }

    if (write(pipe_cmd, cmd, len) != len)
    {
        return ret;
    }

    if (read(pipe_echo, &ret, sizeof(ret)) <= 0)
    {
        perror("read");
        return -1;
    }

    return ret;
}
