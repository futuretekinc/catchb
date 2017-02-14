#ifndef	TRACE_H_
#define	TRACE_H_

#include <stdio.h>

#define	TRACE(format, ...)	printf(format, ## __VA_ARGS__)
#define	ERROR(errno, format, ...)	printf(format, ## __VA_ARGS__)

#endif
