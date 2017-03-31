MODEL=i686
TARGET=$(TOPDIR)/target/$(MODEL)
EXEC_PREFIX=$(TARGET)/usr/bin
INC_PREFIX=$(TARGET)/usr/include
LIB_PREFIX=$(TARGET)/usr/lib
CONF_PREFIX=$(TARGET)/etc
CGI_PREFIX=$(TARGET)/www/cgi-bin
export MODEL TOPDIR

CFLAGS:= -fPIC -c -g -Wall \
				-I.\
				-I$(TOPDIR)\
				-I$(TOPDIR)/include\
				-I$(TOPDIR)/lib\
				-I$(TOPDIR)/maind

LDFLAGS:= -Wl,--no-as-needed -rdynamic\
				-L.\
				-L$(TOPDIR)\
				-L$(TOPDIR)/lib\
				-L$(TOPDIR)/libssh



ifeq ($(MODEL), i686)
CC=gcc

CFLAGS:= $(CFLAGS) \
		-I/usr/local/include \
		-I/usr/local/include/cjson 

LDFLAGS:= $(LDFLAGS)\
		-L/usr/local/lib
endif

ifeq ($(MODEL), ftm-50)
CC=armv5-linux-gcc
AR=armv5-linux-ar

CFLAGS:= $(CFLAGS) \
		-I/home/xtra/ftm/build/ftm-50s/_root/usr/include\
		-I/home/xtra/ftm/build/ftm-50s/_root/usr/local/include
		
LDFLAGS:= $(LDFLAGS)\
		-L/home/xtra/ftm/build/ftm-50s/_root/usr/lib\
		-L/home/xtra/ftm/build/ftm-50s/_root/usr/local/lib\
		-L/home/xtra/tmp/libssh/build-armv5/src
endif

ifeq ($(MODEL), ftm-100)
CC=arm-openwrt-linux-uclibcgnueabi-gcc
AR=arm-openwrt-linux-uclibcgnueabi-ar

CFLAGS:= $(CFLAGS) \
		-I/home/xtra/ftm/build/ftm-50s/_root/usr/include\
		-I/home/xtra/ftm/build/ftm-50s/_root/usr/local/include
		
LDFLAGS:= $(LDFLAGS)\
		-L/home/xtra/ftm/build/ftm-50s/_root/usr/lib\
		-L/home/xtra/ftm/build/ftm-50s/_root/usr/local/lib\
		-L/home/xtra/tmp/libssh/build-armv5/src
endif

BIN_CFLAGS=$(CFLAGS)
BIN_LDFLAGS=$(LDFLAGS)
LIB_CFLAGS=$(CFLAGS)
LIB_LDFLAGS=$(LDFLAGS)
