#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <arpa/inet.h>

static int get_iface_ipaddr(const char *ifname, char *ip);

int main(int argc, char **argv)
{
    char ipaddr[128] = { 0 };

    if (argc != 2)
    {
        fprintf(stderr, "Usage:%s <ifacename>\n", argv[0]);
        return -1;
    }


    memset(ipaddr, 0, sizeof(ipaddr));
    if (get_iface_ipaddr(argv[1], ipaddr) < 0)
    {
        printf("Get ipaddr error!\n");
    }
    else
    {
        printf("ipaddr is:%s\n", ipaddr);
    }

    return 0;
}

static int get_iface_ipaddr(const char *ifname, char *ip)
{
    struct ifreq ifr;
    int skfd = -1;
    struct sockaddr_in *saddr;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return -1;
    }

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0)
    {
        close(skfd);
        return -1;
    }
    close(skfd);

    saddr = (struct sockaddr_in *) &ifr.ifr_addr;
    sprintf(ip, "%s", inet_ntoa(saddr->sin_addr));
    return 0;
}
