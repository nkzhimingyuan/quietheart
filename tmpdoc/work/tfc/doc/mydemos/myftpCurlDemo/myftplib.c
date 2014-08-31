#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "include/curl/curl.h"

#define NAME_LEN 256

typedef struct tagBuffer
{
    char *pBuf;
    size_t len;
}Buffer_t;

typedef struct tagFtpFile {
  char *filename;
  FILE *stream;
}FtpFile_t;

static char s_serverPath[NAME_LEN] = {0};
static char s_userPassword[NAME_LEN] = {0};
static curl_off_t s_maxSpeed;
static CURL *s_curl;
static int s_append = 1;
static Buffer_t s_data;
static FtpFile_t s_download;

static int setupUploadHandle(CURL *curl);
static int setupDownloadHandle(CURL *curl);
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
static size_t write_callback(void *buffer, size_t size, size_t nmemb, void *stream);

int UploadInit()
{
    curl_global_init(CURL_GLOBAL_ALL);
    return 0;
}


int UploadUninit()
{
    curl_global_cleanup();
    return 0;
}


int SetServerPath(const char *serverPath)
{
    memset(s_serverPath, 0x00, sizeof(s_serverPath));
    snprintf(s_serverPath, sizeof(s_serverPath), "%s", serverPath);
    return 0;
}


int SetUserPassword(const char *user, const char *password)
{
    memset(s_userPassword, 0x00, sizeof(s_userPassword));
    snprintf(s_userPassword, sizeof(s_userPassword), "%s:%s", user, password);
    return 0;
}


int SetMaxSpeed(unsigned speed)
{
  s_maxSpeed = (curl_off_t)(speed);
}


int UploadBuffer(void *pBuf, size_t size, int append)
{
    int ret = 0;
    CURLcode res;

    s_data.pBuf = pBuf;
    s_data.len = size;
    s_append = append;

    s_curl = curl_easy_init();
    if (!s_curl)
    {
        ret = -1;
        goto end;
    }

    setupUploadHandle(s_curl);
    res = curl_easy_perform(s_curl);

    if(res != CURLE_OK)
    {
        ret = -1;
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(s_curl);

end:
    return ret;
}


int DownloadFile(char *filename)
{
    int ret = 0;
    CURLcode res;

    s_download.filename = filename;
    s_download.stream=fopen(s_download.filename, "wb");
    s_curl = curl_easy_init();
    if (!s_curl)
    {
        ret = -1;
        goto end;
    }

    setupDownloadHandle(s_curl);
    res = curl_easy_perform(s_curl);

    if(res != CURLE_OK)
    {
        ret = -1;
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(s_curl);

end:
    if(s_download.stream)
    {
        fclose(s_download.stream);
    }
    return ret;
}


static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *buf)
{
    memcpy(ptr, ((Buffer_t*)buf)->pBuf, ((Buffer_t*)buf)->len);
    return ((Buffer_t*)buf)->len;
}


static size_t write_callback(void *buffer, size_t size, size_t nmemb, void *stream)
{
  FtpFile_t *out=(FtpFile_t *)stream;
  if(out)
  {
    if(!out->stream)
    {
      return -1;
    }
  }
  return fwrite(buffer, size, nmemb, out->stream);
}


static int setupUploadHandle(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_URL, s_serverPath);
    curl_easy_setopt(curl, CURLOPT_USERPWD, s_userPassword);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &s_data);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);
    //curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl, CURLOPT_MAX_SEND_SPEED_LARGE, s_maxSpeed);

    if(1 == s_append)
    {
        curl_easy_setopt(curl, CURLOPT_APPEND, 1);
    }
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)(s_data.len));
}


static int setupDownloadHandle(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_URL, s_serverPath);
    curl_easy_setopt(curl, CURLOPT_USERPWD, s_userPassword);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s_download);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl, CURLOPT_MAX_SEND_SPEED_LARGE, s_maxSpeed);
}
