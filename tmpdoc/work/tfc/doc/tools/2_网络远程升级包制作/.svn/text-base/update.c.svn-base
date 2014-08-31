// Copyright (c) 2012 tongfangcloud, Inc.  All rights reserved.

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "update.h"

#define FLASH_LOG  "flash_log"
#define BOOTCMD_LOG "bootcmd_log"


/*return the bits of major*/
static inline int vernum(char *p, int *pnum)
{
    if (p[0] >= '0' && p[0] <= '9')
    {
        if (p[1] >= '0' && p[1] <= '9')
        {
            *pnum = atoi(p);
            return 2;
        }
        else if (p[1] == '.' || p[1] == 0)
        {
            *pnum = atoi(p);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static inline int get_vernum(char *version, int n[4])
{
    char *p = version + 1;// skip 'v'
    int i = 0;
    for (i = 0; i < 4; i++)
    {
        int bits = vernum(p, &n[i]);
        if (bits == 0)
        {
            return -1;
        }
        p += (bits + 1);
    }
    return 0;
}

static inline int check_ver(char *version)
{
    unsigned int i = 0;
    if (version)
    {
        if (version[0] == 'V' || version[0] == 'v')
        {
            int n[4] = {0};
            for (i = 1; i < strlen(version); i++)
            {
                if (!( ((version[i] >= '0') && (version[i] <= '9')) || (version[i] == '.') ))
                {
                    return -1;
                }
            }

            if (get_vernum(version,n) != 0)
            {
                return -1;
            }
            return 0;
        }
    }

    return -1;
}

static inline int compare_ver(char *v1, char *v2)
{
    int n1[4] = {0};
    int n2[4] = {0};
    int i = 0;

    if (check_ver(v1) == 0 && check_ver(v2) == 0)
    {
        get_vernum(v1, n1);
        get_vernum(v2, n2);
        for (i = 0; i < 4; i++)
        {
            if (n1[i] > n2[i])
            {
                return 1;
            }
            if (n1[i] < n2[i])
            {
                return -1;
            }
        }
        return 0;
    }
    else
    {
        return -1;
    }

    return 0;
}


/**
*@function CheckFileCrc
*@brief Check the firmware crcsum
*@param fd fireware fd
*@param header fireware header
*@return 0 success , -1 failed.
*/
int CheckFileCrc(int fd, fileheader_t header)
{

    unsigned long crc = 0;
    unsigned char buffer[FLASH_BLOCK_SIZE] = {0};
    int retval = -1;

    crc = GenerateCrc(crc, (const char *)(&(header.devtype)), sizeof(header.devtype));
    crc = GenerateCrc(crc, (const char *)(header.version), sizeof(header.version));
    crc = GenerateCrc(crc, (const char *)(header.imginfo), sizeof(header.imginfo));
    crc = GenerateCrc(crc, (const char *)(header.img_section), sizeof(header.img_section));
    while (1)
    {
        retval = read(fd, buffer, FLASH_BLOCK_SIZE);
        if (retval < 0)
        {
            printf("read file date error\n");
            return -1;
        } else if (retval == 0)
        {
            printf("end of file\n");
            break;
        }
        crc = GenerateCrc(crc, (const char *)buffer, (unsigned long)(retval));
    }
    //Debug
    printf("header.crc = %ld\t\t crc = %ld\n", header.crc, crc);

    if (crc != header.crc)
    {
        return -1;
    }

    return 0;

}


/*
*@function FirmwareVer
*@brief Get firmware version
*@param void
*@return ptr success , NULL failed.
*/
int GetBoardType(void)
{
    int devtype = 0x06;
    return devtype;
}


/**
*@function CheckBoardType
*@brief check device type
*@param header firmware header
*@return 0 success , -1 failed.
*/
int CheckBoardType(fileheader_t header)
{

    unsigned long devtype = 0;

    devtype = GetBoardType();    

    if (devtype != header.devtype)
    {
        printf("device type incorrect\n");
        return -1;
    }

    return 0;

}


/*
*@function FirmwareVer
*@brief Get firmware version
*@param void
*@return ptr success , NULL failed.
*/
char *GetFirmwareVer(void)
{
    char *FirmwareVer="v12.06.11.09";
    return FirmwareVer;
}


/**
*@function CheckFirmwareVer
*@brief check firmware version
*@param header firmware header
*@return 0 success , -1 failed.
*/
int CheckFirmwareVer(fileheader_t header)
{
    char version[SOFTWARE_VERSION_LEN] = {0};
    int retval = -1;
    char *ptmp = GetFirmwareVer();
    if (ptmp == NULL)
    {
        return -1;
    }
    strncpy(version, ptmp, sizeof(version));

    // Debug
    printf("%s\t %s\n", header.version, version);

    retval = compare_ver(header.version, version);
    if (retval <= 0)
    {
        printf("firmware version incorrect\n");
        return -1;
    }

    return 0;

}


/**
*@function CheckUpgadeFile
*@brief Check the firmware is used for equipment
*@param pName firmware file name
*@return 0 success , -1 failed.
*/
int CheckUpgadeFile(const char *pName)
{

    fileheader_t header;
    int retval = -1;

    if(pName == NULL)
    {
        printf("%s\t %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset(&header, 0, sizeof(fileheader_t));

    int fd = open(pName, O_RDONLY);
    if (fd < 0)
    {
        printf("openfile error %s\n", pName);
        return -1;
    }
    retval = read(fd, &header, sizeof(fileheader_t));
    if (retval != sizeof(fileheader_t))
    {
        printf("read file header error\n");
        close(fd);
        return -1;
    }

    if (header.magic != UPGRADE_PACKET_MAGIC)
    {
        printf("incorrect file type\n");
        close(fd);
        return -1;
    }
    retval = CheckFileCrc(fd, header);
    if (retval < 0)
    {
        printf("firmware crc error\n");
        close(fd);
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

    retval = CheckBoardType(header);
    if (retval < 0)
    {
        printf("device type incorrect\n");
        close(fd);
        return -1;
    }
    retval = CheckFirmwareVer(header);
    if (retval < 0)
    {
        printf("firmware is not new\n");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;

}


/**
*@function DoKernelUpgade
*@brief write kernel img to kernel partition
*@param fd firmware file fd 
*@param header firmware file header
*@return 0 success , -1 failed.
*/
int DoKernelUpgade(int fd, fileheader_t header, int partiontag)
{

    char buffer[FLASH_BLOCK_SIZE] = {0};
    char tmp[NAME_LEN] = {0};
    char name[NAME_LEN] = {0};
    int retval = -1;
    int name_len = strlen(header.imginfo[SEC_KERNEL2].name);

    strncpy(tmp, header.imginfo[SEC_KERNEL2].name, name_len);
    sprintf(name, "%s/%s", TEMP_PATH, tmp);
    //Debug
    printf("%s\t %s\n", tmp, name);

    int Dstfd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (Dstfd < 0)
    {
        if (EEXIST == errno)
        {
            printf("%s already exist\n", name);
            return -1;
        }
        printf("create file error %s\n", name);
        return -1;
    }

    off_t offset = header.imginfo[SEC_KERNEL2].start;
    retval = lseek(fd, offset, SEEK_SET);
    if (retval < 0)
    {
        printf("lseek error\n");
        return -1;
    }

    unsigned long len = header.imginfo[SEC_KERNEL2].len;
    unsigned long readlen = 0;
    while (1)
    {
        readlen = sizeof(buffer);
        if (len < sizeof(buffer))
        {
            readlen = len;
        }
        retval = read(fd, buffer, readlen);
        if (retval < 0)
        {
            printf("read firmware file error\n");
            return -1;
        }
        retval = write(Dstfd, buffer, retval);
        if (retval < readlen)
        {
            printf("write kernel img error\n");
            return -1;
        }
        if (len < sizeof(buffer))
        {
            printf("create kernel img finished\n");
            close(Dstfd);
            break;
        }
        len -= sizeof(buffer);
    }
    //Debug
    printf("create kernel img finished\n");
//    return 0;

    char cmd[128] = {0};
    if(partiontag == 1)
    {
        sprintf(cmd, "/usr/sbin/flash_eraseall -j %s > %s/%s\n", 
                header.img_section[SEC_KERNEL2].mtddev,
                TEMP_PATH, FLASH_LOG);
        if (system(cmd))
        {
            printf("flash_erase failed : %s", 
                   header.img_section[SEC_KERNEL2].mtddev);
            return -1;
        }
        sprintf(cmd, "/usr/sbin/nandwrite -s %d -f -p -j %s %s > %s/%s\n",
                header.img_section[SEC_KERNEL2].offset,
                header.img_section[SEC_KERNEL2].mtddev, 
                name, TEMP_PATH, FLASH_LOG);
        if (system(cmd))
        {
            printf("flash write failed : %s", 
                   header.img_section[SEC_KERNEL2].mtddev);
            return -1;
        }
    }
    else
    {
        sprintf(cmd, "/usr/sbin/flash_eraseall -j %s > %s/%s\n", 
                header.img_section[SEC_KERNEL1].mtddev,
                TEMP_PATH, FLASH_LOG);
        if (system(cmd))
        {
            printf("flash_erase failed : %s", 
                   header.img_section[SEC_KERNEL1].mtddev);
            return -1;
        }
        sprintf(cmd, "/usr/sbin/nandwrite -s %d -f -p -j %s %s > %s/%s\n",
                header.img_section[SEC_KERNEL1].offset,
                header.img_section[SEC_KERNEL1].mtddev, 
                name, TEMP_PATH, FLASH_LOG);
        if (system(cmd))
        {
            printf("flash write failed : %s", 
                   header.img_section[SEC_KERNEL1].mtddev);
            return -1;
        }
    }
    
    return 0;
}


/**
*@function DoFileSysUpgade
*@brief write filesys img to filesys partition
*@param fd firmware file fd
*@param header firmware file header
*@return.0 success , -1 failed.
*/
int DoFileSysUpgade(int fd, fileheader_t header, int partiontag)
{

    char buffer[FLASH_BLOCK_SIZE] = {0};
    char tmp[NAME_LEN] = {0};
    char img_name[NAME_LEN] = {0};
    int retval = -1;
    int name_len = strlen(header.imginfo[SEC_FILESYS2].name);

    strncpy(tmp, header.imginfo[SEC_FILESYS2].name, name_len);
    sprintf(img_name, "%s/%s", TEMP_PATH, tmp);
    //Debug
    printf("%s\t %s\n", tmp, img_name);


    int Dstfd = open(img_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (Dstfd < 0)
    {
        if (EEXIST == errno)
        {
            printf("%s already exist\n", img_name);
            return -1;
        }
        printf("create file error %s\n", img_name);
        return -1;
    }

    off_t offset = header.imginfo[SEC_FILESYS2].start;
    retval = lseek(fd, offset, SEEK_SET);
    if (retval < 0)
    {
        printf("lseek error\n");
        return -1;
    }

    unsigned long len = header.imginfo[SEC_FILESYS2].len;
    unsigned long readlen = 0;
    while (1)
    {
        readlen = sizeof(buffer);
        if (len < sizeof(buffer))
        {
            readlen = len;
        }
        retval = read(fd, buffer, readlen);
        if (retval < 0)
        {
            printf("read firmware file error\n");
            return -1;
        }
        retval = write(Dstfd, buffer, retval);
        if (retval < readlen)
        {
            printf("write filesys img error\n");
            return -1;
        }
        if (len < sizeof(buffer))
        {
            printf("create filesys img finished\n");
            close(Dstfd);
            break;
        }
        len -= sizeof(buffer);
    }
    //Debug
    printf("create filesys img finished\n");
//    return 0;

    char cmd[128] = {0};
    if(partiontag == 1)
    {
        sprintf(cmd, "/usr/sbin/flash_eraseall -j %s > %s/%s\n", 
                header.img_section[SEC_FILESYS2].mtddev,
                TEMP_PATH, FLASH_LOG);
        if (system(cmd))
        {
            printf("flash_erase failed : %s", 
                   header.img_section[SEC_FILESYS2].mtddev);
            return -1;
        }
        sprintf(cmd, "/usr/sbin/nandwrite -s %d -f -p -j %s %s > %s/%s\n", 
                header.img_section[SEC_FILESYS2].offset,
                header.img_section[SEC_FILESYS2].mtddev,
                img_name, TEMP_PATH, FLASH_LOG);
        if (system(cmd))
        {
            printf("flash write failed : %s", 
                   header.img_section[SEC_FILESYS2].mtddev);
            return -1;
        }
    }
    else
    {
        sprintf(cmd, "/usr/sbin/flash_eraseall -j %s > %s/%s\n", 
                header.img_section[SEC_FILESYS1].mtddev,
                TEMP_PATH, FLASH_LOG);
        if (system(cmd))
        {
            printf("flash_erase failed : %s", 
                   header.img_section[SEC_FILESYS1].mtddev);
            return -1;
        }
        sprintf(cmd, "/usr/sbin/nandwrite -s %d -f -p -j %s %s > %s/%s\n", 
                header.img_section[SEC_FILESYS1].offset,
                header.img_section[SEC_FILESYS1].mtddev,
                img_name, TEMP_PATH, FLASH_LOG);
        if (system(cmd))
        {
            printf("flash write failed : %s", 
                   header.img_section[SEC_FILESYS1].mtddev);
            return -1;
        }
    }
    
    return 0;
}


/**
*@function DoChangeUbootParam
*@brief change uboot bootargs
*@param void
*@return 0 success , -1 failed.
*/
int DoChangeUbootParam(int partiontag)
{
    //Debug
//    return 0;

    char cmd[2048] = {0};

    if(partiontag == 1)
    {
        sprintf(cmd, "/usr/sbin/fw_setenv bootcmd 'nboot 0x80700000 0 0x2900000;bootm 0x80700000'\n");
        if (system(cmd))
        {
            printf("change uboot param error\n");
            return -1;
        }
        sprintf(cmd, "/usr/sbin/fw_setenv bootargs 'mem=48M console=ttyS0,115200n8 init=/init root=/dev/mtdblock5 rootfstype=jffs2 rw cmemk.phys_start=\"0x83000000\" cmemk.phys_end=\"0x88000000\" cmemk.phys_start_1=\"0x00001000\" cmemk.phys_end_1=\"0x00008000\" cmemk.pools_1=\"1x28672\" cmemk.allowOverlap=1 ip=192.168.3.199:192.168.3.226:192.168.3.1:255.255.255.0::eth0:off eth=$(ethaddr)'\n");
        if (system(cmd))
        {
            printf("%s\t %d\n", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    else
    {
        sprintf(cmd, "/usr/sbin/fw_setenv bootcmd 'nboot 0x80700000 0 0x500000;bootm 0x80700000'\n");
        if (system(cmd))
        {
            printf("change uboot param error\n");
            return -1;
        }
        sprintf(cmd, "/usr/sbin/fw_setenv bootargs 'mem=48M console=ttyS0,115200n8 init=/init root=/dev/mtdblock3 rootfstype=jffs2 rw cmemk.phys_start=\"0x83000000\" cmemk.phys_end=\"0x88000000\" cmemk.phys_start_1=\"0x00001000\" cmemk.phys_end_1=\"0x00008000\" cmemk.pools_1=\"1x28672\" cmemk.allowOverlap=1 ip=192.168.3.199:192.168.3.226:192.168.3.1:255.255.255.0::eth0:off eth=$(ethaddr)'\n");
        if (system(cmd))
        {
            printf("%s\t %d\n", __FUNCTION__, __LINE__);
            return -1;
        }
    }

    return 0;
}


/**
*@function DoUpgade
*@brief write firmware to flash
*@param pName firmware name
*@return 0 success , -1 failed.
*/
int DoUpgade(const char *pName)
{
    fileheader_t header;
    int retval = -1;
    char cmd[256] = {0};
    char filepath[128] = {0};
    int partiontag = -1;
    
    if(pName == NULL)
    {
        printf("%s\t %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    sprintf(cmd,"/usr/sbin/fw_printenv bootcmd > %s/%s\n", TEMP_PATH, BOOTCMD_LOG);
    if(system(cmd))
    {
        printf("%s\t %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
    sprintf(filepath, "%s/%s", TEMP_PATH, BOOTCMD_LOG);
    int bootfd = open(filepath, O_RDONLY);
    if(bootfd < 0)
    {
        printf("%s\t %d\n", __FUNCTION__, __LINE__);
        return -1;
    }
    memset(cmd, 0, sizeof(cmd));
    read(bootfd, cmd, sizeof(cmd));
    printf("%s\t %d\n %s\n", __FUNCTION__, __LINE__, cmd);
    retval = strncmp(cmd, "bootcmd=nboot 0x80700000 0 0x500000;bootm 0x80700000",
                   strlen("bootcmd=nboot 0x80700000 0 0x500000;bootm 0x80700000"));
    if(retval == 0)
    {
        partiontag = 1;
    }
    else
    {
        partiontag = 2;
    }
    printf("%s\t %d\t %d\n", __FUNCTION__, __LINE__, partiontag);

    memset(&header, 0, sizeof(header));

    int fd = open(pName, O_RDONLY);
    if (fd < 0)
    {
        printf("openfile error %s\n", pName);
        return -1;
    }

    retval = read(fd, &header, sizeof(fileheader_t));
    if (retval != sizeof(fileheader_t))
    {
        printf("read file header error\n");
        close(fd);
        return -1;
    }

    retval = DoKernelUpgade(fd, header, partiontag);
    if (retval < 0)
    {
        printf("do kernel update error\n");
        close(fd);
        return -1;
    }

    retval = DoFileSysUpgade(fd, header, partiontag);
    if (retval < 0)
    {
        printf("do filesystem update error\n");
        close(fd);
        return -1;
    }

    retval = DoChangeUbootParam(partiontag);
    if (retval < 0)
    {
        printf("change uboot param error\n");
        return -1;
    }

    return 0;
}


/**
*@function Main for update_func
*@brief Main for update_func
*@param pName firmware name
*@return 0 success , -1 failed.
*/
int update_func(const char *pName)
{

    int retval = -1;

    if(pName == NULL)
    {
        printf("%s\t %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    retval = CheckUpgadeFile(pName);
    if (retval < 0)
    {
        printf("firmware incorrect\n");
        return -1;
    }
    //Debug
    printf("check firmware finished\n");

    retval = DoUpgade(pName);
    if (retval < 0)
    {
        printf("do update error\n");
        return -1;
    }
    //Debug
    printf("Do upgrade finished\n");

    return 0;
}


/**
*@function Main for Test update function
*@brief Main for update_func
*@param 
*@return 0 success , -1 failed.
*/
int main(int argc, char **argv)
{

    int retval = -1;
    const char *pName = argv[1];

    if(pName == NULL)
    {
        printf("%s\t %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

    retval = CheckUpgadeFile(pName);
    if (retval < 0)
    {
        printf("firmware incorrect\n");
        return -1;
    }
    //Debug
    printf("check firmware finished\n");

    retval = DoUpgade(pName);
    if (retval < 0)
    {
        printf("do update error\n");
        return -1;
    }
    //Debug
    printf("Do upgrade finished\n");

    return 0;
}
