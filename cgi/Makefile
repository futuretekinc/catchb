ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

EXEC_PREFIX=$(TARGET)/www/cgi-bin
BIN_TARGET=catchb.cgi
BIN_CFLAGS:=$(CFLAGS)\
		-D_GNU_SOURCE\
		-I/home/xtra/ftm/build/ftm-50s/_root/usr/include/cjson\

BIN_LDFLAGS:=$(LDFLAGS)\
		-lcatchb\
		-lssh\
		-ltelnet\
		-lcjson \
		-lqdecoder\
		-lcurl\
		-lssl\
		-lcrypto\
		-lpthread \
		-lz\
		-lm\
		-ldl\
		-lrt
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

include $(TOPDIR)/IncludePost.mk

