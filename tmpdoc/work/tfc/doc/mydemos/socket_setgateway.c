#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <unistd.h>
#include <signal.h>
#include <asm/types.h>
#include <linux/sockios.h>
#include <paths.h>
#include <net/if_arp.h>

static struct sockaddr_in sa =
{
sin_family: PF_INET,
            sin_port:   0
};

int net_search_gateway(char *buf, in_addr_t *gate_addr, const char *ifrname)
{
    char iface[16];
    unsigned long dest, gate;
    int iflags = 0;

    sscanf(buf, "%s\t%08lX\t%08lX\t%8X\t", iface, &dest, &gate, &iflags);
    if ( (iflags & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY) )
    {
        if (strcmp(iface, ifrname) == 0)
        {
            *gate_addr = gate;
            return 0;
        }
    }
    return -1;
}

int net_del_gateway(const in_addr_t addr, const char* ifname)
{
    struct rtentry rt;
    int skfd = -1;

	char szlog[128] = {0};
    struct in_addr tempaddr;
	tempaddr.s_addr = addr;
	char * ptipaddr = inet_ntoa(tempaddr);

    /* Clean out the RTREQ structure. */
    memset((char *)&rt, 0, sizeof(struct rtentry));

    /* Fill in the other fields. */
    rt.rt_flags             = (RTF_UP | RTF_GATEWAY);
    rt.rt_dst.sa_family     = PF_INET;
    rt.rt_genmask.sa_family = PF_INET;
    rt.rt_dev = ifname;

    sa.sin_addr.s_addr = addr;
    memcpy((char *) &rt.rt_gateway, (char *) &sa, sizeof(struct sockaddr));

    /* Create a socket to the INET kernel. */
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "socket error!\n");
        return -1;
    }
    /* Tell the kernel to accept this route. */
    if (ioctl(skfd, SIOCDELRT, &rt) < 0)
    {
        close(skfd);
		snprintf(szlog, sizeof(szlog), "del gateway [%s] failed", ptipaddr);
        fprintf(stderr, "%s\n", szlog);
        return -1;
    }
    /* Close the socket. */
    close(skfd);
	
	snprintf(szlog, sizeof(szlog), "del gateway [%s] succeed", ptipaddr);
    printf("%s\n",szlog);

    return (0);
}

int net_add_gateway(const in_addr_t addr, char* ifname)
{
    struct rtentry rt;
    int skfd = -1;

	char szlog[128] = {0};
    struct in_addr tempaddr;
	tempaddr.s_addr = addr;
	char * ptipaddr = inet_ntoa(tempaddr);

    /* Clean out the RTREQ structure. */
    memset((char *)&rt, 0, sizeof(struct rtentry));

    /* Fill in the other fields. */
    rt.rt_flags             = (RTF_UP | RTF_GATEWAY);
    rt.rt_dst.sa_family     = PF_INET;
    rt.rt_genmask.sa_family = PF_INET;
    rt.rt_dev = ifname;

    sa.sin_addr.s_addr = addr;
    memcpy((char *) &rt.rt_gateway, (char *)&sa, sizeof(struct sockaddr));

    /* Create a socket to the INET kernel. */
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stderr, "socket error");
        return -1;
    }
    /* Tell the kernel to accept this route. */
    if (ioctl(skfd, SIOCADDRT, &rt) < 0)
    {
        close(skfd);
		
		snprintf(szlog, sizeof(szlog), "add gateway [%s] failed", ptipaddr);
        fprintf(stderr, "%s\n", szlog);
        return -1;
    }
    /* Close the socket. */
    close(skfd);

	snprintf(szlog, sizeof(szlog), "add gateway [%s] succeed", ptipaddr);
    printf("%s\n", szlog);
    return (0);
}

int net_set_gateway(const in_addr_t addr, const char *ifrname)
{
    in_addr_t gate_addr;
    char buff[132];
    FILE *fp = fopen("/proc/net/route", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "no proc file.\n");
        return -1;
    }
    fgets(buff, 130, fp);
    while (fgets(buff, 130, fp) != NULL)
    {
        if (net_search_gateway(buff, &gate_addr, ifrname) == 0)
        {
            net_del_gateway(gate_addr, ifrname);
        }
    }
    fclose(fp);

    char szlog[128] = {0};
    struct in_addr tempaddr;
    tempaddr.s_addr = addr;
    char * ptipaddr = inet_ntoa(tempaddr);
    snprintf(szlog, sizeof(szlog), "set gateway [%s:%s] ok", ifrname, ptipaddr );
    printf("%s\n", szlog);

    return net_add_gateway(addr, ifrname);
}


int main(int argc, char *argv[])
{
    return net_set_gateway(inet_addr("192.168.0.2"), "wlan0");
}
