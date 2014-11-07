#ifndef __MYUNIX_SOCKET_H__
#define __MYUNIX_SOCKET_H__

#include <sys/un.h> 

#define NO_WAIT             0 
#define WAIT_FOREVER       -1 
#define DEFAULT_DIR "/tmp"

int UnixDomainInit(struct sockaddr_un *sockAddr, char *socketPath);
int UnixDomainSocket(int *socketFd);
int UnixDomainSocketBind(int socketFd, struct sockaddr_un *bindAddr);
int UnixDomainSocketRecvFrom(int sockFd, struct sockaddr_un *remoteAddr, void *pBuf, unsigned int bufSize, int timeout);
int UnixDomainSocketSendTo(int sockFd, struct sockaddr_un *remoteAddr, void *pBuf, unsigned int bufSize, int timeout);
int UnixDomainUnInit(struct sockaddr_un *sockAddr);

#endif
