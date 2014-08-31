#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "myftplib.h"

#define FTP_USER "ftptest"
#define FTP_PWD "ftptest"
#define BUF_SZ 2048

static char s_pBuf[BUF_SZ] = {0};

static int uploadFile(const char *fileName, const char *user, const char *password, const char *serverPath, int speed);

static int uploadFile(const char *fileName, const char *user, const char *password, const char *serverPath, int speed)
{
  FILE *hd_src;
  struct stat file_info;
  size_t fsize;
  int append = 0;
  int ret = 0;

  UploadInit();
  SetMaxSpeed(speed);
  SetServerPath(serverPath);
  SetUserPassword(user, password);

  if(stat(fileName, &file_info))
  {
    fprintf(stderr, "Get file:%s state error!\n", fileName);
    return -1;
  }
  fsize = file_info.st_size;

  hd_src = fopen(fileName, "rb");
  while(fsize >= 0 && !feof(hd_src))
  {
      int readSize = 0;
      readSize = fread(s_pBuf, sizeof(char), sizeof(s_pBuf), hd_src);

      //fprintf(stderr, "line:%d, bufsize:%d, char size:%d, readsize:%d\n", __LINE__, sizeof(s_pBuf.buf), sizeof(char), s_Buf.len);
      if (UploadBuffer(s_pBuf, readSize, append) < 0)
      {
          ret = -1;
          goto end;
      }
      append = 1;
      fsize -= readSize;
  }

end:
  fclose(hd_src);
  UploadUninit();
  return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc != 3)
    {
        fprintf(stderr, "Usage:%s <file path> <server path>\n", argv[0]);
        return -1;
    }

    ret = uploadFile(argv[1], FTP_USER, FTP_PWD, argv[2], 2048);
    return ret;
}
