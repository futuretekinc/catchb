MODEL=i686

TOPDIR=$(shell pwd)
SUBDIR=libssh lib catchb

export TOPDIR

all: subdirs

subdirs:
	@for dir in $(SUBDIR); do\
		make -C $$dir;\
	done

install:
	@for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done

dev_install:
	@for dir in $(SUBDIR); do\
		make -C $$dir dev_install;\
	done

clean:
	@for dir in $(SUBDIR); do\
		make -C $$dir clean;\
	done

