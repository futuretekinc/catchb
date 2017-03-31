ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=catchb.cgi
BIN_CFLAGS:=$(CFLAGS)\
		-D_GNU_SOURCE

BIN_LDFLAGS:=$(LDFLAGS)\
		-lz\
		-lcatchb\
		-lssh\
		-lcurl\
		-lssl\
		-lcrypto\
		-lpthread \
		-lcjson \
		-lmxml\
		-lqdecoder\
		-lm\
		-lrt
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

include $(TOPDIR)/IncludePost.mk

