# makefile for asmx multi-assembler
# this should work with GNU make, gcc, and bash shell

# version number for zipfile name
VERSION = 2.0a3

# C compiler target architecture
#   this is the setting for OS X to create a universal PPC/Intel binary
#   comment it out if you aren't running OS X for Intel
TARGET_ARCH = -arch ppc -arch i686

# C compiler flags
CFLAGS = -Wall -O2

# install directory in ~/bin or wherever you want it
INSTALL_DIR = ~/bin

OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

#OBJS = asm1802.o asm6502.o asm6809.o asm68hc11.o asm68hc16.o \
#       asm68k.o asm8048.o asm8051.o asm8085.o asmf8.o asmz80.o asmx.o

.PHONY: all
all: asmx

asmx: $(OBJS)

$(OBJS): asmx.h

.PHONY: strip
strip: asmx
	strip asmx

.PHONY: install
install: asmx
	rm -f $(INSTALL_DIR)/asmx
	cp asmx $(INSTALL_DIR)
# select aliases for your favorite assemblers here:
#	ln -sf asmx $(INSTALL_DIR)/asm1802
	ln -sf asmx $(INSTALL_DIR)/asm6502
	ln -sf asmx $(INSTALL_DIR)/asm6809
#	ln -sf asmx $(INSTALL_DIR)/asm68hc11
#	ln -sf asmx $(INSTALL_DIR)/asm68hc16
	ln -sf asmx $(INSTALL_DIR)/asm68k
#	ln -sf asmx $(INSTALL_DIR)/asm8048
#	ln -sf asmx $(INSTALL_DIR)/asm8051
#	ln -sf asmx $(INSTALL_DIR)/asm8085
#	ln -sf asmx $(INSTALL_DIR)/asmf8
	ln -sf asmx $(INSTALL_DIR)/asmz80
#	ln -sf asmx $(INSTALL_DIR)/asmgbz80

.PHONY: install-strip
install-strip: strip install

.PHONY: uninstall
uninstall:
	rm -f $(INSTALL_DIR)/asmx
	rm -f $(INSTALL_DIR)/asm1802
	rm -f $(INSTALL_DIR)/asm6502
	rm -f $(INSTALL_DIR)/asm6809
	rm -f $(INSTALL_DIR)/asm68hc11
	rm -f $(INSTALL_DIR)/asm68hc16
	rm -f $(INSTALL_DIR)/asm68k
	rm -f $(INSTALL_DIR)/asm8051
	rm -f $(INSTALL_DIR)/asm8085
	rm -f $(INSTALL_DIR)/asmf8
	rm -f $(INSTALL_DIR)/asmz80
	rm -f $(INSTALL_DIR)/asmgbz80

# do everything
.PHONY: it
it: clean install-strip zip

.PHONY: zip
zip: asmx # build everything first to check for errors
	rm -f ../src/.DS_Store ../test/.DS_Store ../test/rel/.DS_Store
	-mv -f ../asmx-$(VERSION).zip ../asmx-$(VERSION).zip.old
	cd .. && zip -rq asmx-$(VERSION).zip Makefile README.txt src/*.c src/*.h src/Makefile test

.PHONY: test
test:
# note: asmx must be installed first!
	cd ../test && testit

.PHONY: clean
clean:
	rm -f $(OBJS) asmx
