#ifndef __CPU1COMM_DRV_H
#define __CPU1COMM_DRV_H

/*XXX what will be the major?*/
#define CPU1COMMDRV_MAJOR 252
#define CPU1COMM_DEVICE  "cpu1comm"
//#define cpu1commdrv_SIZE 0x1000

typedef struct
{
	char *data;
	int length;
}IOCTL_BUF;
/******Commands for ioctl.******/
#define CPU1COMM_MAGIC 'C'
/*no input and output for ioctl.*/
#define CPU1COMM_NOTIFY_READ _IO(CPU1COMM_MAGIC,8)

/*input and output for ioctl.*/
#define CPU1COMM_FAKE_WWRITE _IO(CPU1COMM_MAGIC,14,IOCTL_BUF)
#define CPU1COMM_FAKE_WREAD _IO(CPU1COMM_MAGIC,15,IOCTL_BUF)

#if 0
#define CPU1COMM_INIT _IOWR(CPU1COMM_MAGIC,0,CPU_COMM_RAM)
#define CPU1COMM_RELEASE _IOWR(CPU1COMM_MAGIC,1,CPU_COMM_RAM)
#define CPU1COMM_WKRAM_RCVWAIT _IO(CPU1COMM_MAGIC,2)
//XXX parameters type
#define CPU1COMM_SEND _IOWR(CPU1COMM_MAGIC,3,IOCTL_BUF)
#define CPU1COMM_RECEIVE _IOWR(CPU1COMM_MAGIC,4,IOCTL_BUF)
#define CPU1COMM_WKRAM_SBUF _IOWR(CPU1COMM_MAGIC,5,char*)
#define CPU1COMM_WKRAM_RBUF _IOWR(CPU1COMM_MAGIC,6,char*)
#define CPU1COMM_WKRAM_STAT _IOWR(CPU1COMM_MAGIC,7,int)

#define CPU1COMM_NOTIFY_READ _IO(CPU1COMM_MAGIC,8)
#define CPU1COMM_WAIT_FOR_SEND _IO(CPU1COMM_MAGIC,9)
#define CPU1COMM_NOTIFY_WRITE _IO(CPU1COMM_MAGIC,10)
#define CPU1COMM_WAIT_FOR_RECEIVE _IO(CPU1COMM_MAGIC,11)
#define CPU1COMM_SYNC_KRAM _IOWR(CPU1COMM_MAGIC,12,CPU_COMM_RAM)
#define CPU1COMM_SYNC_URAM _IOWR(CPU1COMM_MAGIC,13,CPU_COMM_RAM)

//XXX for debug
#define CPU1COMM_FAKE_WWRITE _IO(CPU1COMM_MAGIC,14)
#define CPU1COMM_FAKE_WREAD _IO(CPU1COMM_MAGIC,15)
#define CPU1COMM_WWRITE_TEST _IO(CPU1COMM_MAGIC,16)
#define CPU1COMM_RWRITE_TEST _IO(CPU1COMM_MAGIC,17)
#define WWRITE_CHAR1 'A'
#define WWRITE_CHAR2 'B'
#endif

/******Memory map offset******/
#define START_OFF 0
#define WORKRAM_SIZE (1<<13) /*8K*/
#define BUF0_OFF 0x0800 
#define BUF0_SHIFT 11
#define BUF0_SIZE (1<<BUF0_SHIFT)
/*XXX cpu0 buffer.
#define BUF0_OFF 0x1000
#define BUF0_SIZE (1<<10)
*/
#define BUF1_OFF 0x1000
#define BUF1_SHIFT 11
#define BUF1_SIZE (1<<BUF1_SHIFT)
/*XXX cpu1 buffer.
#define BUF1_OFF 0x1400
#define BUF1_SIZE (1<<10)
*/

#if 0
/******Buffer structure******/
/*
*Communication process on cpu0:
*1,send data to cpu1:
*(1)on cpu0, read the cpu1 status, if its write buffer is full then wait, or write.
*(2)on cpu0, generate an interrupt to cpu1 after after write its buffer to notify cpu1 to read.
*(3)on cpu1,after cpu1 read, generate interrupt to cpu0 to tell it the data have been read.
*(4)do (1)-(3) until all the data be sent.
*2,receive data from cpu1:
*(1)on cpu1,read the cpu1 status, if its read buffer is full then wait, or write.
*(2)on cpu1, generate an interrupt to cpu0 after write to notify cpu0 to read.
*(3)on cpu0,in the interrupt handler, receive the data, generate interrupt to cpu1 the data have been read.
*(4)do (1)-(3) until all the data be received.
*
*status check for write and read ring buffer:
*(1)if the two buffer is empty, then their read pointer equals to  write pointer.
*(2)on cpu0, after write to write buffer, its write pointer updated.
*If the read pointer equals to  write pointer again, it means full.
*(3)on cpu1, after read from write buffer, its read pointer updated.
*If it is full, then clear the full flag.
*(4)on cpu1, after write to read buffer, its write pointer updated.
*If the read pointer equals to  write pointer again, it means full.
*(5)on cpu0, after read from read buffer, its read pointer updated.
*If it is full, then clear the full flag.
*/
typedef struct
{
	/*volatile int status;*/
	struct WB_Inner
	{/*XXX uint_32, 32 bits for info!!! */
		unsigned total_len:13;/*XXX 8K max for request.*/
		unsigned sent_len:BUF0_SHIFT;
		unsigned reserved:7;
		unsigned status:1;
	}info;
	volatile char data[BUF0_SIZE-4]; /*XXX not all used*/
}W_BUF;

typedef struct
{
	/*volatile int status;*/
	struct RB_Inner
	{/*XXX uint_32, 32 bits for info!!! */
		unsigned total_len:13;/*XXX 8K max for request.*/
		unsigned sent_len:BUF1_SHIFT;
		unsigned reserved:7;
		unsigned status:1;
	}info;
	volatile char data[BUF1_SIZE-4];
}R_BUF;
typedef struct 
{
	volatile char *write_buf_wp;/*The write pointer of write ring buffer of cpu1.[0,4],write by cpu0.*/
	volatile char *write_buf_rp;/*The read pointer of write ring buffer of cpu1.[0,4],read by cpu1.*/
	volatile struct WB_Inner *diablo_cpu1_status;/*cpu1 write ring buffer status.(Bit[0]--0:not full, 1:full),other bits for len*/
	volatile W_BUF *write_buf;
	//char *write_buf_start;
	//char *write_buf_end;

	volatile char *read_buf_wp;/*The write pointer of read ring buffer of cpu1.[0,1],write by cpu1.*/
	volatile char *read_buf_rp;/*The read pointer of read ring buffer of cpu1.[0,1],read by cpu0.*/
	volatile struct RB_Inner *diablo_cpu0_status;/*cpu1 read ring buffer status.(Bit[0]--0:not full, 1:full),other bits for len*/
	volatile R_BUF *read_buf;
	//char *read_buf_start;
	//char *read_buf_end;
} CPU_COMM_RAM;
#endif

#endif
