/*
This equals to:
 ifconfig |grep UP.....
 get the UP and RUNNING flag for an net device.
*/
#include<stdio.h>
#include<string.h>

#include <fcntl.h>
//#include <errno.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
//#include <linux/ethtool.h>

static int check_nic(const char *nic);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage:%s <ifacename>\n", argv[0]);
        return -1;
    }
    check_nic(argv[1]);
    return 0;
}

static int check_nic(const char *nic)
{
    int ret = -1;
    struct ifreq ifr;
    int skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, nic);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        close(skfd);
        printf("Get flags fail.\n");
        return -1;
    }

    if (ifr.ifr_flags & IFF_RUNNING)
    {
        printf("Get flags Running.\n");
        ret = 1;
    }

    if (ifr.ifr_flags & IFF_UP)
    {
        printf("Get flags UP.\n");
        ret = 1;
    }

    close(skfd);
    return ret;
}


