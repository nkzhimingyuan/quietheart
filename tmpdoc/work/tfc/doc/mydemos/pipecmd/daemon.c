#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>

#define MSG_SIZE     512
#define PIPE_CMD     "/tmp/pip_cmd"
#define PIPE_ECHO    "/tmp/pip_echo"
#define PIPE_LOG     "/tmp/pip_log"

int pipe_cmd = -1;
int pipe_echo = -1;

int process_cmd(char *pbuf)
{
    int echo;
    signed char ret = 0;
    if (pbuf == NULL)
    {
        return -1;
    }

    echo = system(pbuf);

#if 0
    ret = (signed char)(echo >> 8);
    echo = (int)ret;
#else
    ret = WEXITSTATUS(echo);
    echo = (int)ret;
#endif
    /*write back */
    if (write(pipe_echo, &echo, sizeof(echo)) != sizeof(echo))
    {
        perror("write");
        return -1;
    }
    return echo;
}

int main(int argc,char **argv)
{
    char buffer[MSG_SIZE];
    int ret;

    if (access(PIPE_CMD, F_OK) != 0)
    {
        mkfifo(PIPE_CMD, 0777);
    }

    if (access(PIPE_ECHO, F_OK) != 0)
    {
        mkfifo(PIPE_ECHO, 0777);
    }

    if (access(PIPE_LOG, F_OK) != 0)
    {
        mkfifo(PIPE_LOG, 0777);
    }

    ret = fork();
    if (ret < 0)
    {
        return -1;
    }
    else if (ret == 0) /*parent*/
    {
        system(argv[1]);
    }
    else/*child*/
    {
        pipe_cmd = open(PIPE_CMD, O_RDWR | O_NONBLOCK);
        pipe_echo = open(PIPE_ECHO, O_RDWR | O_NONBLOCK);
        if (pipe_cmd < 0 )
        {
            printf("open %s err\n", PIPE_CMD);
            perror("open");
            exit(1);
        }
        if (pipe_echo < 0)
        {
            printf("open %s err\n", PIPE_ECHO);
            perror("open");
            exit(1);
        }
        for ( ; ; )
        {
            fd_set rds;
            struct timeval step;
            int ret, len;

            FD_ZERO(&rds);
            FD_SET(pipe_cmd, &rds);
            step.tv_sec  = 5;
            step.tv_usec = 0;

            ret = select(pipe_cmd + 1, &rds, NULL, NULL, &step);
            if (ret < 0)
            {
                perror("select");
                exit(1);
            }
            else if (ret == 0)/*heart beating*/
            {
                continue;
            }
            else
            {
                if (FD_ISSET(pipe_cmd, &rds))
                {
                    memset(buffer, 0, sizeof(buffer));
                    len = read(pipe_cmd, buffer, MSG_SIZE);
                    if (len <= 0)
                    {
                        perror("read");
                        continue;
                    }
                    else if (len < MSG_SIZE)
                    {
                        process_cmd(buffer);
                    }
                    else
                    {
                        int echo = -1;
                        printf("cmd too long\n");
                        if (write(pipe_echo, &echo, sizeof(echo)) != sizeof(echo))
                        {
                            continue;
                        }
                    }
                }
            }
        }

        close(pipe_cmd); 
        close(pipe_echo);
    }
    return 0;
}


