#updatefirmware Makefile by zhw 2012.09.13 

include firmware.config

CC = gcc
ARM_LINUX_CC = arm_v5t_le-gcc

CFLAGS += -Wall -g


all:firmware update


firmware:firmware.c common.c update.h
	$(CC) $(CFLAGS) -o $@  $^

update:update.c common.c update.h
	$(ARM_LINUX_CC) $(CFLAGS) -o $@  $^
    
mkfirmware:
	mkfs.cramfs $(rootfs) rootfs.cramfs
	./firmware $(firmware_name) 6 $(version) $(kernel) rootfs.cramfs
	
	cp $(firmware_name) version/
	rm rootfs.cramfs
	rm $(firmware_name)

clean:
	rm -f firmware
	rm -f update
	
.PHONY:clean mkfirmware
