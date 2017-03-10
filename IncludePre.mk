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
				-I$(TOPDIR)/include\
				-I$(TOPDIR)/lib\
				-I$(TOPDIR)/libssh\
				-I$(TOPDIR)/maind\
				-I/home/xtra/ftm/build/ftm-50s/_root/usr/include\
				-I/home/xtra/ftm/build/ftm-50s/_root/usr/local/include

LDFLAGS:= -Wl,--no-as-needed\
				-L.\
				-L$(TOPDIR)/lib\
				-L$(TOPDIR)/libssh\
				-L/home/xtra/ftm/build/ftm-50s/_root/usr/lib\
				-L/home/xtra/ftm/build/ftm-50s/_root/usr/local/lib\
				-L/home/xtra/tmp/libssh/build-armv5/src


#CC=gcc
CC=armv5-linux-gcc
#CC=arm-openwrt-linux-uclibcgnueabi-gcc
AR=armv5-linux-ar

CFLAGS:= -fPIC \
	$(CFLAGS) \
		-I/usr/local/include \
		-I/usr/local/include/cjson 

LDFLAGS:= $(LDFLAGS)\
		-L/usr/local/lib


BIN_CFLAGS=$(CFLAGS)
BIN_LDFLAGS=$(LDFLAGS)
LIB_CFLAGS=$(CFLAGS)
LIB_LDFLAGS=$(LDFLAGS)
