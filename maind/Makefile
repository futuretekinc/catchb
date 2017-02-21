ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1

BIN_TARGET=catchb
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c) $(wildcard shell/*.c))
BIN_LDFLAGS=$(LDFLAGS) -L$(TOPDIR)/lib -lcatchb -lsqlite3 -lcjson -pthread -lm -ldl
include $(TOPDIR)/IncludePost.mk
