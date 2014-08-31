/*根据ipv4的ip,mask得出网络地址，mask长度，以及主机长度（便于确认是否是广播地址还是网络地址）。*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

static int isValidMask(char *mask);

int main(int argc, char *argv[])
{
    int i;
    //char *mask = "255.255.255.255";//32
    //char *mask = "255.255.255.254";//31
    //char *mask = "255.255.255.252";//30
    //char *mask = "255.255.255.248";//29
    //char *mask = "255.255.255.240";//28
    //char *mask = "255.255.255.224";//27
    //char *mask = "255.255.255.192";//26
    //char *mask = "255.255.255.128";//25
    //char *mask = "255.255.255.0";//24
    //char *mask = "255.255.254.0";//23
    //char *mask = "255.255.252.0";//22
    //char *mask = "128.0.0.0";//1
    //char *mask = "0.0.0.0";//0
    char *mask[] = {
        "255.255.255.255",//32
        "255.255.255.254",//31
        "255.255.255.252",//30
        "255.255.255.248",//29
        "255.255.255.240",//28
        "255.255.255.224",//27
        "255.255.255.192",//26
        "255.255.255.128",//25
        "255.255.255.0",//24
        "255.255.254.0",//23
        "255.255.252.0",//22
        "255.255.248.0",//21
        "255.255.240.0",//20
        "255.255.224.0",//19
        "255.255.192.0",//18
        "255.255.128.0",//17
        "255.255.0",//16
        "255.254.0.0",//15
        "255.252.0.0",//14
        "255.248.0.0",//13
        "255.240.0.0",//12
        "255.224.0.0",//11
        "255.192.0.0",//10
        "255.128.0.0",//9
        "255.0",//8
        "254.0.0.0",//7
        "252.0.0.0",//6
        "248.0.0.0",//5
        "240.0.0.0",//4
        "224.0.0.0",//3
        "192.0.0.0",//2
        "128.0.0.0",//1
        "0.0.0.0",//0
    };

    for (i = 0; i < 32; ++i)
    {
        int ret = isValidMask(mask[i]);
        fprintf(stderr, "mask check ret:%d\n", ret);
    }
    return 0;
}

static int isValidMask(char *mask)
{
    struct in_addr maskBin;
    in_addr_t localMask;
    int hits = 0;
    int bit = 0;
    int maskRet = 0;
    int scanLen = 0;
    int maskLen = 0;

    maskRet = inet_aton(mask, &maskBin);
    if (0 == maskRet)
    {
        fprintf(stderr, "error mask!\n");
        return -1;
    }
    localMask = ntohl(maskBin.s_addr);

    scanLen = 1;
    while (scanLen < 33)
    {
        bit = (1 << (scanLen - 1)) & localMask;

        if (bit)
        {
            hits += 1;
        }
        else
        {
            if (hits)
            {
                maskLen = -1;
                break;
            }
        }
        ++scanLen;
    }

    maskLen = hits;
    return maskLen;
}
