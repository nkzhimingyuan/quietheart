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
#include <errno.h>
#include <linux/watchdog.h>

#define MSG_SIZE         256
#define PIPE_IPNC        "/tmp/pip_ipnc"
#define CHECK_REBOOT_GAP 10

int g_watchdogEnablePort = 46;
int g_watchdogFeedPort = 65;

int gio_write(int gio_id, int IsOn)
{
    int     fd_gio = -1;
    char    data[5];
    int     result = -1;
    char    dev_name[25];
    unsigned int cnt = 0;
    int     retv = -1;

    sprintf(dev_name, "/proc/gio/gio%d", gio_id );

    fd_gio = open(dev_name, O_RDWR);

    if (fd_gio < 0)
    {
        fprintf(stderr, "open device error !! gio = %d\n", gio_id);
        goto GIO_QUIT;
    }

    if (IsOn == 1)
    {
        for (cnt = 0 ; cnt < sizeof(data) ; cnt ++)
        {
            data[cnt] = '1';
        }
    }
    else if (IsOn == 0)
    {
        for (cnt = 0 ; cnt < sizeof(data) ; cnt ++)
        {
            data[cnt] = '0';
        }
    }
    else
    {
        goto GIO_QUIT;
    }

    result = write(fd_gio,  data, 1);
    if (result <= 0)
    {
        fprintf(stderr, "write device error !! gio = %d\n", gio_id);
        close(fd_gio);
        goto GIO_QUIT;
    }
    retv = 0;
GIO_QUIT:
    if (fd_gio >= 0)
    {
        close(fd_gio);
    }
    return retv;
}

void mSleep(const int mseconds)
{
    struct timeval tv;
    tv.tv_sec  = mseconds / 1000;
    tv.tv_usec = (mseconds % 1000) * 1000;
    select(0, NULL, NULL, NULL, &tv);
}

void EnableWatchDog(void)
{
    if (g_watchdogEnablePort > 0)
    {
        gio_write(g_watchdogEnablePort, 1);
    }
}

void FeedWatchDog(void)
{
    if (g_watchdogFeedPort > 0)
    {
        gio_write(g_watchdogFeedPort, 1);
        mSleep(80);
        gio_write(g_watchdogFeedPort, 0);
        mSleep(70);
    } 
    else
    {
        mSleep(100);
    }
}

int main(int argc, char *argv[])
{
    int pipe_ipnc = -1;
    int rebootIndex = 0;
    fd_set rds;
    struct timeval step;
    char buffer[MSG_SIZE] = {0};
    int fd = -1;

    EnableWatchDog();
    
    fd = open("/dev/watchdog", O_WRONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open /dev/watchdog!!!\n");
    }
    else
    {
        printf("----DaVinci Watchdog Enable.\n");
        int timeout = 0; 
        ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
        printf("----DaVinci Watchdog Timer: heartbeat %d sec.\n", timeout);
    }

    while (access(PIPE_IPNC, F_OK) == -1)
    {
        sleep(1);
    }

    if ((pipe_ipnc = open(PIPE_IPNC, O_RDONLY | O_NONBLOCK)) < 0)
    {
        fprintf(stderr, "Failed to open /tmp/pip_ipnc !!!\n");
        return -1;
    }

    step.tv_sec  = 1;
    step.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&rds);
        FD_SET(pipe_ipnc, &rds);
        rebootIndex++;

        if (rebootIndex > CHECK_REBOOT_GAP)
        {
            fprintf(stderr, "daemon ipnc timeout(%d s)!!!\n", rebootIndex);
            break;
        }

        if (select(pipe_ipnc + 1, &rds, NULL, NULL, &step) < 0)
        {
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        if (read(pipe_ipnc, buffer, MSG_SIZE) > 0)
        {
            //step1: soft dog
            rebootIndex = 0;

            //step2:mcu dog
            FeedWatchDog();
            
            //step3:davinci dog
            if (fd >= 0)
            {
                write(fd, "\0", 1);
                fsync(fd);
            }
        }
        else
        {
            if (errno == ENOENT)
            {
                fprintf(stderr, "daemon ipnc exit!!!\n");
                break;
            }
        }
        sleep(1);
    }
    close(pipe_ipnc);
    
    printf("daemon ipnc reboot system\n");

    return 0;
}


