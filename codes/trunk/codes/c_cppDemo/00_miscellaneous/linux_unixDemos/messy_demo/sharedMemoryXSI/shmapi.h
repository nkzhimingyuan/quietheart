#ifndef __SHMAPI_H__
#define __SHMAPI_H__

#include <sys/types.h>

#define MSG_SIZE 126
#define SHM_SIZE (MSG_SIZE+1)
#define SHM_MODE 0600

typedef char Byte;
int ShmCreate();
int ShmDestroy(int shmid);

int ShmInit();
int ShmUninit();

int ShmReceive(Byte* recv);
int ShmSend(Byte* send, size_t size);
#endif
