// Copyright (c) 2012 tongfangcloud, Inc.  All rights reserved.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "update.h"

int Create_File(const char *pFile)
{
    int fd = open(pFile, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        if (EEXIST == errno)
        {
            printf("%s already exist\n", pFile);
            return -1;
        }
        printf("create file %s failed\n", pFile);
        return -1;
    }
    return fd;
}

int Open_File(const char *pFile)
{
    int fd = open(pFile, O_RDONLY);
    if (fd < 0)
    {
        printf("open fiel %s error\n", pFile);
        return -1;
    }
    return fd;

}

unsigned long GetFileCrc(int fd, fileheader_t *header)
{
    unsigned long crc = 0;
    unsigned char buffer[FLASH_BLOCK_SIZE] = {0};
    int retval = -1;

    crc = GenerateCrc(crc, (const char *)(&(header->devtype)), sizeof(header->devtype));
    crc = GenerateCrc(crc, (const char *)(header->version), sizeof(header->version));
    crc = GenerateCrc(crc, (const char *)(header->imginfo), sizeof(header->imginfo));
    crc = GenerateCrc(crc, (const char *)(header->img_section), sizeof(header->img_section));
    while (1)
    {
        retval = read(fd, buffer, FLASH_BLOCK_SIZE);
        if (retval < 0)
        {
            printf("read file date error\n");
            return 0;
        } 
        else if (retval == 0)
        {
            printf("read end of file :: Do firmware crcsum\n");
            break;
        }
        crc = GenerateCrc(crc, (const char *)buffer, (unsigned long)(retval));
        //Debug
        //printf("get file crcsum\n");
    }

    return crc;

}

int Create_Firmware(const char *pFirmware, const char *pKernle, const char *pFilesys, fileheader_t *header)
{
    if (NULL == pFirmware || NULL == pKernle || NULL == pFilesys)
    {
        return -1;
    }

    int fd = -1;
    int Dstfd = -1;
    int readlen = -1;
    int retval = -1;
    char buffer[FLASH_BLOCK_SIZE] = {0};
    struct stat filestat;
    const char *pName = NULL;

    memset(&filestat, 0, sizeof(filestat));

    pName = pFirmware;
    Dstfd = Create_File(pName);
    if (Dstfd < 0)
    {
        return -1;
    }

    pName = pKernle;
    fd    = Open_File(pName);
    if (fd < 0)
    {
        close(Dstfd);
        return -1;
    }
    strcpy(header->imginfo[SEC_KERNEL2].name, KERNEL2_NAME);
    header->imginfo[SEC_KERNEL2].start = sizeof(fileheader_t);
    if (stat(pName, &filestat) < 0)
    {
        printf("get %s stats error\n", pName);
        close(Dstfd);
        close(fd);
        return -1;
    }
    header->imginfo[SEC_KERNEL2].len = filestat.st_size;

    lseek(Dstfd, sizeof(fileheader_t), SEEK_SET);
    lseek(fd, 0, SEEK_SET);

    while (1)
    {
        readlen = read(fd, buffer, sizeof(buffer));
        if (readlen < 0)
        {
            printf("read %s error\n", pName);
            close(Dstfd);
            close(fd);
            return -1;
        }
        if (readlen == 0)
        {
            printf("read end of file :: %s\n", pName);
            close(fd);
            break;
        }
        retval = write(Dstfd, buffer, readlen);
        if (retval < readlen)
        {
            printf("write firmware file error");
            close(Dstfd);
            close(fd);
            return -1;
        }
    }

    pName = pFilesys;
    fd = Open_File(pName);
    if (fd < 0)
    {
        close(Dstfd);
        return -1;
    }
    strcpy(header->imginfo[SEC_FILESYS2].name, FILESYS2_NAME);
    header->imginfo[SEC_FILESYS2].start = 
    header->imginfo[SEC_KERNEL2].start + header->imginfo[SEC_KERNEL2].len;
    if (stat(pName, &filestat) < 0)
    {
        printf("get %s stats error\n", pName);
        close(Dstfd);
        close(fd);
        return -1;
    }
    header->imginfo[SEC_FILESYS2].len = filestat.st_size;

    lseek(Dstfd, header->imginfo[SEC_FILESYS2].start, SEEK_SET);
    lseek(fd, 0, SEEK_SET);

    while (1)
    {
        readlen = read(fd, buffer, sizeof(buffer));
        if (readlen < 0)
        {
            printf("read %s error\n", pName);
            close(Dstfd);
            close(fd);
            return -1;
        }
        if (readlen == 0)
        {
            printf("read end of file :: %s\n", pName);
            close(fd);
            break;
        }
        retval = write(Dstfd, buffer, readlen);
        if (retval < readlen)
        {
            printf("write firmware file error");
            close(Dstfd);
            close(fd);
            return -1;
        }
    }

    unsigned long crcsum = 0;
    lseek(Dstfd, sizeof(fileheader_t), SEEK_SET);
    crcsum = GetFileCrc(Dstfd, header);
    if (crcsum == 0)
    {
        printf("get firmware crcsum error\n");
        return -1;
    }
    header->crc = crcsum;

    //Debug
    printf("header->crc = %ld\t crcsum = %ld\n", header->crc, crcsum);

    lseek(Dstfd, 0, SEEK_SET);
    retval = write(Dstfd, header, sizeof(fileheader_t));
    if (retval < sizeof(fileheader_t))
    {
        printf("write firmware header error\n");
        return -1;
    }

    close(Dstfd);

    return 0;

}





int main(int argc, char **argv)
{

    //1.open & create a firmware file :: return file fd;
    //2.open kernel img file :: return file fd;
    //3.read kernel img and write to firmware file;
    //4.open filesys img file :: return file fd;
    //5.read filesys img and write to firmware file;
    //6.check firmware file crcsum and write to firmware file header;
    //7.create firmware file success;
    //Done

    //argv[0] :: exefile name    argv[1] :: firmware file name   
    //argv[2] :: boardtype  argv[3] :: firmware version
    //argv[4] :: kernel img name    argv[5] :: filesys img name

    int retval = -1;
    char *Firmware = argv[1];
    char *Boardtype = argv[2];
    char *Firmver =argv[3];
    char *Kernel = argv[4];
    char *Filesys = argv[5];
    fileheader_t header;

    memset(&header, 0, sizeof(header));

    header.magic = UPGRADE_PACKET_MAGIC;
    header.devtype = atoi(Boardtype);
    strncpy(header.version, Firmver, strlen(Firmver));
    memcpy(header.img_section, img_section, sizeof(img_section));

    retval = Create_Firmware(Firmware, Kernel, Filesys, &header);
    if (retval < 0)
    {
        printf("Create Firmware %s error\n", Firmware);
        return -1;
    }

    //Debug
    int i = 0;
    printf("%lx\t\t %ld\t\t %4lx\t\t %s\n", header.magic, header.crc, header.devtype, header.version);
    for (i = 0; i < SEC_BUTT; ++i)
    {
        printf("%s\t\t %lx\t\t %lx\n", header.imginfo[i].name, header.imginfo[i].start, header.imginfo[i].len);
    }
    for (i = 0; i < SEC_BUTT; ++i)
    {
        printf("%s\t\t %x\t\t %s\t\t %x\t\t %x\n",
               header.img_section[i].name, header.img_section[i].addr, header.img_section[i].mtddev,
               header.img_section[i].offset, header.img_section[i].size);
    }

    return 0;

}
