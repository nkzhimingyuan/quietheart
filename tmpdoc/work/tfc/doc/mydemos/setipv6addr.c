#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
//#include <syslog.h>
//#include <unistd.h>
#include <sys/ioctl.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <net/if.h>
//#include <arpa/inet.h>
#include <netinet/in.h>
//#include <ifaddrs.h>

struct in6_ifreq
{
    struct in6_addr ifr6_addr;
    unsigned int    ifr6_prefixlen;
    int             ifr6_ifindex;
};

static int net_set_ifv6addr(const char *ifname, const char *addr, int mask);

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage:%s <ifacename> <ip> <prefix len>\n",argv[0]);
        return -1;
    }

    net_set_ifv6addr(argv[1], argv[2], atoi(argv[3]));
    return 0;
}

int net_set_ifv6addr(const char *ifname, const char *addr, int mask)
{
    struct in6_ifreq ifr6;
    struct ifreq ifr;
    int fd = -1;

    if ((ifname == NULL) || (addr == NULL))
    {
        return -1;
    }

    if (inet_pton(AF_INET6, addr, (void *)&ifr6.ifr6_addr) < 0)
    {
        perror("interface address inet_pton");
        return -1;
    }

    fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("No support for INET6 on this system");
        return -1;
    }

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(fd, SIOGIFINDEX, &ifr) < 0)
    {
        perror("Getting interface index error");
        close(fd);
        return -1;
    }
    ifr6.ifr6_ifindex   = ifr.ifr_ifindex;
    ifr6.ifr6_prefixlen = mask;
    if (ioctl(fd, SIOCSIFADDR, &ifr6) < 0)
    {
        perror("Setting interface address error");
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}
