/*根据ipv4的ip,mask得出网络地址，mask长度，以及主机长度（便于确认是否是广播地址还是网络地址）。*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[])
{
    //char *ip = "219.134.241.173";
    //char *mask = "255.255.255.255";//32
    //char *ip = "219.134.241.173";
    //char *mask = "255.255.255.254";//31
    //char *ip = "219.134.241.173";
    //char *mask = "255.255.255.252";//30
    //char *ip = "219.134.241.173";
    //char *mask = "255.255.255.248";//29
    //char *ip = "219.134.241.173";
    //char *mask = "255.255.255.240";//28
    //char *ip = "192.168.0.120";
    //char *mask = "255.255.255.224";//27
    //char *ip = "192.168.0.120";
    //char *mask = "255.255.255.192";//26
    //char *ip = "192.168.0.120";
    //char *mask = "255.255.255.128";//25
    //char *ip = "192.168.0.120";
    //char *mask = "255.255.255.0";//24
    //char *ip = "192.168.0.120";
    //char *mask = "255.255.254.0";//23
    //char *ip = "192.168.0.120";
    //char *mask = "255.255.252.0";//22
    char *ip = "192.168.0.120";
    char *mask = "255.255.248.0";//21
    char net[33] = {0};
    char broadcast[33] = {0};
    int maskLen = 0;
    int hostLen = 0;
    maskLen = getNetAddr(ip, mask, net);
    hostLen = getBroadcastAddr(ip, mask, broadcast);
    fprintf(stderr, "ip address:%s, net mask:%s\n", ip, mask);
    fprintf(stderr, "mask len:%d, net address:%s\n", maskLen, net);
    fprintf(stderr, "host len:%d, broadcast address:%s\n", hostLen, broadcast);

    return 0;
}

/*get the net mask len and net address for address 'addr' with net mask 'mask'.*/
int getNetAddr(const char *addr, const char *mask, char *net)
{
    int addrRet = 0;
    int maskRet = 0;
    struct in_addr addrBin;
    struct in_addr maskBin;

    addrRet = inet_aton(addr, &addrBin);
    maskRet = inet_aton(mask, &maskBin);
    if (0 == addrRet || 0 == maskRet)
    {
        return -1;
    }

    fprintf(stderr, "maskbin:%08x\n", maskBin.s_addr);
    fprintf(stderr, "htonl.maskbin:%08x\n", ntohl(maskBin.s_addr));
    int maskLen = 0;
    while ((ntohl(maskBin.s_addr) << maskLen) && (maskLen < 32))
    {
        ++maskLen;
    }

    //get net addr
    struct in_addr netBin;
    netBin.s_addr = maskBin.s_addr & addrBin.s_addr;
    char *netString = inet_ntoa(netBin);
    if(NULL == netString)
    {
        return -1;
    }

    strcpy(net, netString);
    return maskLen;
}

/*get the host len and broadcast address for address 'addr' with net mask 'mask'.*/
int getBroadcastAddr(const char *addr, const char *mask, char *broadcast)
{
    int addrRet = 0;
    int maskRet = 0;
    struct in_addr addrBin;
    struct in_addr maskBin;

    addrRet = inet_aton(addr, &addrBin);
    maskRet = inet_aton(mask, &maskBin);
    if (0 == addrRet || 0 == maskRet)
    {
        return -1;
    }

    int maskLen = 0;
    int hostLen = 0;
    while ((ntohl(maskBin.s_addr) << maskLen) && (maskLen < 32))
    {
        ++maskLen;
    }
    hostLen = 32 - maskLen;

    //get broadcast addr
    struct in_addr broadcastBin;
    broadcastBin.s_addr = ~maskBin.s_addr | addrBin.s_addr;
    char *broadcastString = inet_ntoa(broadcastBin);
    strcpy(broadcast, broadcastString);
    return hostLen;
}
