ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

LIB_TARGET=libtelnet.a 
LIB_SRCS=$(wildcard *.c)
LIB_OBJS=$(patsubst %.c,%.o,$(LIB_SRCS))
CFLAGS:=$(CFLAGS) -DUSER_LITTLE_ENDIAN
LIB_CFLAGS=$(CFLAGS)

include $(TOPDIR)/IncludePost.mk
