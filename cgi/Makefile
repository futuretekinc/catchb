ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

EXEC_PREFIX=$(TARGET)/www/cgi-bin
BIN_TARGET=catchb.cgi
BIN_CFLAGS:=$(CFLAGS)\
		-D_GNU_SOURCE

BIN_LDFLAGS:=$(LDFLAGS)\
		-lcatchb\
		-lsqlite3\
		-lpcap\
		-lssh\
		-ltelnet\
		-lcJSON \
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

