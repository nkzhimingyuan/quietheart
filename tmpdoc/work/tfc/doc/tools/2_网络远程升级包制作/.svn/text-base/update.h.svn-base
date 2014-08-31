// Copyright (c) 2012 tongfangcloud, Inc.  All rights reserved.


#ifndef __IMG_H__
#define __IMG_H__

#define UPGRADE_PACKET_MAGIC    0x32725913
#define SOFTWARE_VERSION_LEN    24
#define NAME_LEN                32
#define FLASH_BLOCK_SIZE        (0x20000)   //block size : 128k
#define TEMP_PATH               "/tmp"


#define UBL_ADDR           0x00000000  //3MB
#define UBOOT_ADDR         0x00300000  //2MB
#define KERNEL1_ADDR       0x00500000  //4MB
#define FILESYS1_ADDR      0x00900000  //20MB
#define KERNEL2_ADDR       0x02900000  //4MB
#define FILESYS2_ADDR      0x02D00000  //20MB
#define APPPARAM_ADDR      0x04D00000  //128MB - 77MB = 51MB

#define UBL_NAME            "ubl"
#define UBOOT_NAME          "uboot"
#define KERNEL1_NAME        "kernel1"
#define FILESYS1_NAME       "filesys1"
#define KERNEL2_NAME        "kernel2"
#define FILESYS2_NAME       "filesys2"
#define APPPARAM_NAME       "appparam"

#define UBL                 "/dev/mtd0"
#define UBOOT               "/dev/mtd1"
#define KERNEL1             "/dev/mtd2"
#define FILESYS1            "/dev/mtd3"
#define KERNEL2             "/dev/mtd4"
#define FILESYS2            "/dev/mtd5"
#define APPPARAM            "/dev/mtd6"

typedef enum
{
    SEC_UBL = 0,
    SEC_UBOOT,
    SEC_KERNEL1,
    SEC_FILESYS1,
    SEC_KERNEL2,
    SEC_FILESYS2,
    SEC_APPPARAM,
    SEC_BUTT,
} SECTION_E;

typedef struct
{
    char            name[NAME_LEN];    /*section name*/
    unsigned int    addr;               /*phy addr*/
    char            mtddev[NAME_LEN];  /*mtd dev*/
    unsigned int    offset;             /*offset of mtd dev*/
    unsigned int    size;               /*img section size*/
} img_section_t;

typedef struct
{
    char            name[NAME_LEN];
    unsigned long   start;
    unsigned long   len;
} img_info_t;

typedef struct
{
    unsigned long   magic;
    unsigned long   crc;
    unsigned long   devtype;      /* board type */
    char            version[SOFTWARE_VERSION_LEN];
    img_info_t      imginfo[SEC_BUTT];
    img_section_t   img_section[SEC_BUTT];
} fileheader_t;

img_section_t img_section[SEC_BUTT];

unsigned long GenerateCrc(unsigned long crc, const char *pBuffer, unsigned long len);
int CheckUpgadeFile(const char *pName);
int DoUpgade(const char *pName);
int update_func(const char *pName);

#endif

