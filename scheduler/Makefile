ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=scheduler
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
BIN_CFLAGS=$(CFLAGS) -I$(TOPDIR)/operation
BIN_LDFLAGS=$(LDFLAGS) -L$(TOPDIR)/lib -lcatchb -lpcap -pthread -ldl
include $(TOPDIR)/IncludePost.mk
