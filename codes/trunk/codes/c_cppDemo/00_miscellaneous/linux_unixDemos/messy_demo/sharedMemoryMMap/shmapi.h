#ifndef __SHMAPI_H__
#define __SHMAPI_H__
#include <sys/types.h>

#define SHM_SIZE 4096
#define MSG_SIZE (SHM_SIZE-1)

typedef char Byte;
int MmapCreate1();
int MmapCreate2();
int MmapDestroy();

int MmapInit();
int MmapUninit();

int MmapReceive(Byte* recv);
int MmapSend(Byte* send, size_t size);
#endif
