KERNEL_VERSION	:= `uname -r`
PWD		:= $(shell pwd)
TOP         :=  XXX

KERNEL_DIR      := $(TOP)/XXX
CROSS_COMPILE := arm-eabi-

#CPPFLAGS        :=   
#KBUILD_CPPFLAGS := 
#LINUXINCLUDE    :=  
#EXTRA_CFLAGS :=   

obj-m += shmmap.o

CP      := cp
STRIP   :=$(CROSS_COMPILE)strip
PREBUILT_PATH   := $(PWD)/$(TOP)device/tpvision/$(TARGET_PRODUCT)/DTV_OUT/

all:
	@echo "Building Shared memory map driver..."
	@($(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules ARCH=arm)
	if [ -e $(PWD)/shmmap.ko ]; then \
		#$(STRIP) $(PWD)/shmmap.ko; \
		$(CP) $(PWD)/shmmap.ko $(PREBUILT_PATH); \
		$(MAKE) -C $(KERNEL_DIR) M=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) clean; \
	else \
		#$(STRIP) $(OBJ_ROOT)/shmmap.ko; \
		$(CP) $(CP_FLAG) $(OBJ_ROOT)/shmmap.ko $(PREBUILT_PATH); \
	fi
clean:
	@echo "Cleaning Shared memory map driver..."
	@($(MAKE) -C $(KERNEL_DIR) M=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) clean)
