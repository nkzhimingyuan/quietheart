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
#include <ifaddrs.h>

struct in6_ifreq
{
    struct in6_addr ifr6_addr;
    unsigned int    ifr6_prefixlen;
    int             ifr6_ifindex;
};

static int net_get_ifv6addr(const char * ifname, char *addr, int *mask);
static int getV6MaskLen(const char* netmask);

int main(int argc, char *argv[])
{
    fprintf(stderr, "argc:%d\n", argc);
    if (argc != 3)
    {
        fprintf(stderr, "Usage:%s <ifacename>\n",argv[0]);
    }

    char address[256] = {0};
    int mask = 0;
    int ret = 0;
    ret = net_get_ifv6addr(argv[1], address, &mask);
    fprintf(stderr, "address:%s, mask length:%d, ret:%d\n", address, mask, ret);
    return 0;
}

//int net_get_ifv6addr(const char * ifname, char *ipv6addr, const int addrlen)
int net_get_ifv6addr(const char * ifname, char *address, int *mask)
{
    int ret = -1;
    struct ifaddrs * ifap;
    struct ifaddrs * ife;
    const struct sockaddr_in6 * addr;
    const struct sockaddr_in6 * netmask;
    char addrbuf[256] = {0};
    char maskbuf[256] = {0};

    if ((address == NULL) || (ifname == NULL) || (mask == NULL))
    {
        return -1;
    }

    if (getifaddrs(&ifap) < 0)
    {
        perror("getifaddrs");
        return -1;
    }

    for (ife = ifap; ife; ife = ife->ifa_next)
    {
        if (ifname && (0 != strcmp(ifname, ife->ifa_name)))
        {
            continue;
        }
        if (ife->ifa_addr->sa_family == AF_INET6)
        {
            addr = (const struct sockaddr_in6 *) ife->ifa_addr;
            if (!IN6_IS_ADDR_LOOPBACK(&addr->sin6_addr) && !IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr))
            {
                inet_ntop(ife->ifa_addr->sa_family, &addr->sin6_addr, addrbuf, sizeof(addrbuf));
                strcpy(address, addrbuf);

                netmask = (const struct sockaddr_in6 *) ife->ifa_netmask;
                inet_ntop(ife->ifa_netmask->sa_family, &netmask->sin6_addr, maskbuf, sizeof(maskbuf));

                *mask = getV6MaskLen(maskbuf);
                if (0 == strlen(address))
                {
                    ret = -1;
                    break;
                }
                else
                {
                    ret = 0;
                }
            }
        }
    }
    freeifaddrs(ifap);
    return ret;
}

static int getV6MaskLen(const char* netmask)
{
    int mask = 0;
    int hitEnd = 0;
    const char *p = netmask;
    while(p)
    {
        if (':' == *p)
        {
            if(hitEnd == 1)
            {
                break;
            }
            hitEnd = 1;
        }
        else if ('8' == *p)
        {
            mask+=1;
            hitEnd = 0;
        }
        else if ('c' == *p || 'C' == *p)
        {
            mask+=2;
            hitEnd = 0;
        }
        else if ('e' == *p || 'E' == *p)
        {
            mask+=3;
            hitEnd = 0;
        }
        else if ('f' == *p || 'F' == *p)
        {
            mask+=4;
            hitEnd = 0;
        }
        else
        {
            if(hitEnd == 1)
            {
                break;
            }
            hitEnd = 1;
        }
        ++p;
    }
    return mask;
}
