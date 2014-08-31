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

static int downloadFile(const char *serverFile, const char *user, const char *password, char *localPath, int speed);

static int downloadFile(const char *serverFile, const char *user, const char *password, char *localPath, int speed)
{
    int ret = 0;

    UploadInit();
    SetMaxSpeed(speed);
    SetServerPath(serverFile);
    SetUserPassword(user, password);

    DownloadFile(localPath);

end:
    UploadUninit();
    return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc != 3)
    {
        fprintf(stderr, "Usage:%s <server path> <file path>\n", argv[0]);
        return -1;
    }

    ret = downloadFile(argv[1], FTP_USER, FTP_PWD, argv[2], 2048);
    return ret;
}
