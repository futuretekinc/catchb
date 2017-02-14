ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1

BIN_TARGET=detection
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
BIN_CFLAGS=$(CFLAGS) -I$(TOPDIR)/alarm
BIN_LDFLAGS=$(LDFLAGS) -L$(TOPDIR)/libssh -L$(TOPDIR)/lib -lssh -lcatchb -lz -lcrypto -pthread -ldl -lrt
include $(TOPDIR)/IncludePost.mk
