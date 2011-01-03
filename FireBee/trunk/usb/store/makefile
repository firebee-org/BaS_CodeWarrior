
ifeq ($(CROSS),yes)
CC			= m68k-atari-mint-gcc
STRIP			= m68k-atari-mint-strip
STACK			= m68k-atari-mint-stack
INCLUDE_GEM_PATH	= /usr/local/cross-mint/m68k-atari-mint/include
LIB_GEM_PATH		= /usr/local/cross-mint/m68k-atari-mint/lib
else
CC			= gcc
STRIP			= /usr/bin/strip
STACK			= stack
COMPRESS		= upx
INCLUDE_GEM_PATH	= /usr/GEM/include
LIB_GEM_PATH		= /usr/GEM/lib
endif


ifeq ($(MAKECMDGOALS), aranym)
HCD_S			= ./host/aranym/natfeat_asm.S
HCD_C			= ./host/aranym/aranym-hcd.c ./host/aranym/natfeat.c
HCD_H			= ./host/aranym/nf_ops.h ./host/aranym/usbhost_nfapi.h
PROGRAM			= stor_ara.tos
endif
ifeq ($(MAKECMDGOALS), netusbee)
HCD_C			= ./host/netusbee/isp116x-hcd.c
HCD_H			= ./host/netusbee/isp116x.h
PROGRAM			= stor_ntu.tos
endif
ifeq ($(MAKECMDGOALS), ethernat)
HCD_C			= ./host/ethernat/isp116x-hcd.c
HCD_H			= ./host/ethernat/isp116x.h
PROGRAM 		= stor_etn.tos
endif
ifeq ($(MAKECMDGOALS), ohci-pci)
HCD_C			= ./host/ohci-pci/ohci-hcd.c ./host/ohci-pci/ltoa.c
HCD_H			= ./host/ohci-pci/ohci.h ./host/ohci-pci/pcixbios.h
DEFS			= -DPCI_XBIOS
PROGRAM 		= stor_pci.tos
endif

STACKSIZE		= 64k
OPTIMISATION		= -O -fomit-frame-pointer
CPU			= -m68020-60
LIB			= 
ASFLAGS			= $(CPU)
CFLAGS			= $(CPU) $(OPTIMISATION) -Wall -Wshadow -I$(INCLUDE_GEM_PATH) $(DEFS) -g
LFLAGS			= -L$(LIB_GEM_PATH)
COBJS 			= main.c udelay.c cmd_usb.c usb.c usb_mem.c usb_storage.c debug.c $(HCD_C)
SOBJS 			= debug2.S bios.S $(HCD_S)
HSRC			= config.h debug.h part.h scsi.h super.h usb.h usb_defs.h vars.h $(HCD_H)
COBJECTS 		= $(COBJS:.c=.o)
SOBJECTS 		= $(SOBJS:.S=.o)

all:
	make ethernat
	make netusbee
	make aranym
	rm -f *.o
	make ohci-pci
	
ethernat: $(PROGRAM)
	
aranym: $(PROGRAM)
	
netusbee: $(PROGRAM)

ohci-pci: $(PROGRAM)
	
$(PROGRAM): $(COBJECTS) $(SOBJECTS) $(HSRC)
	$(CC) -o $@ $(COBJECTS) $(SOBJECTS)

strip:
	$(STRIP) $(PROGRAM)

stack:
	$(STACK) -S $(STACKSIZE) $(PROGRAM)

compress:
	$(COMPRESS) $(PROGRAM)
	
clean:
	rm -f *.tos *.log
	find ./ -type f -name "*.o" -exec rm -f {} \;


