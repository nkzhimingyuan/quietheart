#ifndef MYUPLOAD_LIB_H
#define MYUPLOAD_LIB_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>

int UploadInit();
int UploadUninit();
int SetServerPath(const char *serverPath);
int SetUserPassword(const char *user, const char *password);
int SetMaxSpeed(unsigned speed);
int UploadBuffer(void *pBuf, size_t size, int append);
int DownloadFile(char *filename);

#ifdef __cplusplus
}
#endif
#endif
