# (c) Texas Instruments 
include ../Rules.make
	
libs:
ifeq ($(CPU), X86)
	make -fMAKEFILE.MK -C./mkfirmware MODULE=mkfirmware $(MAKE_TARGET)
endif
exe:
ifeq ($(CPU), X86)
	make -fMAKEFILE.MK -C./mkfirmware MODULE=mkfirmware exe
	make  -C./http_load 	
endif

all:
	make -fMAKEFILE.MK depend
	make -fMAKEFILE.MK clean
	make -fMAKEFILE.MK libs
	
clean:
	make -fMAKEFILE.MK libs MAKE_TARGET=clean
	
depend:
	make -fMAKEFILE.MK libs MAKE_TARGET=depend  

install:
	make -fMAKEFILE.MK libs MAKE_TARGET=install  
	
.PHONY : install clean depend all libs 
	
