#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <sys/ioctl.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <linux/if.h> 
#include <linux/sockios.h> 
//#include <linux/ethtool.h> 

typedef struct
{
    unsigned int cmd;
    void *data;
} emac_drv_priv_ioctl;

typedef struct
{
    __u32 phy_num;      /* phy number to be read/written */
    __u32 reg_addr;     /* register to be read/written */
    __u32 data;         /* data to be read/written */
} emac_phy_params;

int get_netlink_status(const char *if_name); 
int main(int argc, char *argv[]) 
{ 
    if(argc != 2)
    {
        fprintf(stderr, "Usage %s <net interface name>\n", argv[0]);
    }
    if(getuid() != 0) 
    { 
        fprintf(stderr, "Netlink Status Check Need Root Power.\n"); 
        return 1; 
    } 
    printf("Net link status: %d\n", get_netlink_status(argv[1])); 
    return 0; 
} 

#define EMAC_IOCTL_BASE      0
#define EMAC_PRIV_MII_READ   (EMAC_IOCTL_BASE + 22)

int get_netlink_status(const char *if_name)
{
    int skfd = -1;
    struct ifreq ifr;
    emac_drv_priv_ioctl priv_ioctl;
    emac_phy_params phy_params;

    if ((skfd = socket( AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror( "socket" );
        return -1;
    }

    bzero( &ifr, sizeof( ifr ) );
    strncpy( ifr.ifr_name, if_name, IFNAMSIZ - 1 );
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    /* DM36x private use */
    phy_params.reg_addr = 1;
    phy_params.phy_num = 1;
    priv_ioctl.cmd = EMAC_PRIV_MII_READ;
    priv_ioctl.data = &phy_params;
    ifr.ifr_data = (char *)&priv_ioctl;
    if (ioctl( skfd, SIOCDEVPRIVATE, &ifr) < 0)
    {
        close(skfd);
        return -1;
    }
    close(skfd);
    if (phy_params.data & 4)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
