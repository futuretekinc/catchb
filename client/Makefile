ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1

BIN_TARGET=cbclient
BIN_SRC=$(shell find . -name "*.c")
#BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c) $(wildcard shell/*.c))
BIN_OBJS = $(BIN_SRC:%.c=%.o)

BIN_LDFLAGS=$(LDFLAGS) -L$(TOPDIR)/lib -lz -lcatchb -lssh -ltelnet -lcurl -lssl -lcrypto -lsqlite3 -lcjson -lpcap -lpthread -lrt -lm -ldl

include $(TOPDIR)/IncludePost.mk
