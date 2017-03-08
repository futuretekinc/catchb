#ifndef	FTM_TELNET_H_
#define	FTM_TELNET_H_

#include "ftm_types.h"
#include "libtelnet.h"
#include "ftm_lock.h"
#include "ftm_buffer.h"
#include <pthread.h>
#include <poll.h>

typedef	struct	FTM_TELNET_CLIENT_STRUCT
{
	FTM_CHAR		pName[FTM_NAME_LEN+1];

	telnet_t _PTR_	pHandler;
	pthread_t		xThread;

	FTM_INT			nSock;
	struct pollfd 	pFD[1];

	FTM_BOOL		bStop;
	FTM_LOCK_PTR	pLock;

	FTM_BUFFER_PTR	pBuffer;
}	FTM_TELNET_CLIENT, _PTR_ FTM_TELNET_CLIENT_PTR;

FTM_RET	FTM_TELNET_CLIENT_create
(
	FTM_TELNET_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_TELNET_CLIENT_destroy
(
	FTM_TELNET_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_TELNET_CLIENT_open
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pHost,
	FTM_UINT16				usPort
);

FTM_RET	FTM_TELNET_CLIENT_close
(
	FTM_TELNET_CLIENT_PTR	pClient
);

FTM_RET	FTM_TELNET_CLIENT_write
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pData,
	FTM_UINT32				ulDataLen
);

FTM_RET	FTM_TELNET_CLIENT_read
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pBuffer,
	FTM_UINT32				ulBufferSize,
	FTM_UINT32_PTR			pReadLen
);


FTM_RET	FTM_TELNET_CLIENT_readline
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pBuffer,
	FTM_UINT32				ulBufferSize,
	FTM_UINT32_PTR			pReadLen
);

#endif
