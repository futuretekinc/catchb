#ifndef	FTM_SSH_H_
#define	FTM_SSH_H_

#include "ftm_types.h"
#include "ftm_buffer.h"
#include "ftm_lock.h"
#include <libssh/libssh.h>
#include <pthread.h>

typedef	struct	FTM_SSH_STRUCT
{
	ssh_session		pSession;
}	FTM_SSH, _PTR_	FTM_SSH_PTR;

typedef	struct	FTM_SSH_CHANNEL_STRUCT
{
	FTM_SSH_PTR		pSSH;
	ssh_channel		pChannel;
	pthread_t		xThread;
	FTM_BUFFER_PTR	pBuffer;
	FTM_LOCK_PTR	pLock;

	FTM_BOOL		bStop;
}	FTM_SSH_CHANNEL, _PTR_ FTM_SSH_CHANNEL_PTR;

FTM_RET	FTM_SSH_create
(
	FTM_SSH_PTR	_PTR_ ppSSH
);

FTM_RET	FTM_SSH_destroy
(
	FTM_SSH_PTR _PTR_ ppSSH
);

FTM_RET	FTM_SSH_connect
(
	FTM_SSH_PTR	pSSH,
	const FTM_CHAR_PTR	pHost, 
	const FTM_CHAR_PTR	pUser,
	const FTM_CHAR_PTR	pPasswd
);

FTM_RET	FTM_SSH_disconnect
(
	FTM_SSH_PTR	pSSH
);

FTM_RET	FTM_SSH_authenticateConsole
(
	FTM_SSH_PTR	pSSH, 
	const FTM_CHAR_PTR	pPasswd	
);

FTM_RET	FTM_SSH_verifyKnownhost
(
	FTM_SSH_PTR pSSH
);

FTM_RET	FTM_SSH_CHANNEL_create
(
	FTM_SSH_PTR	pSSH,
	FTM_SSH_CHANNEL_PTR _PTR_ ppChannel
);


FTM_RET	FTM_SSH_CHANNEL_destroy
(
	FTM_SSH_CHANNEL_PTR _PTR_ ppChannel
);

FTM_RET	FTM_SSH_CHANNEL_open
(	
	FTM_SSH_CHANNEL_PTR	pChannel
);

FTM_RET	FTM_SSH_CHANNEL_close
(	
	FTM_SSH_CHANNEL_PTR	pChannel
);

FTM_BOOL	FTM_SSH_CHANNEL_isOpen
(
	FTM_SSH_CHANNEL_PTR	pChannel
);

FTM_BOOL	FTM_SSH_CHANNEL_isEOF
(
	FTM_SSH_CHANNEL_PTR	pChannel
);

FTM_RET	FTM_SSH_CHANNEL_read2
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_UINT32			ulTimeout,
	FTM_UINT8_PTR		pBuffer,
	FTM_UINT32			ulBufferLen,
	FTM_UINT32_PTR		pReadLen,
	FTM_UINT8_PTR		pErrorBuffer,
	FTM_UINT32			ulErrorBufferLen,
	FTM_UINT32_PTR		pErrorReadLen
);

FTM_RET	FTM_SSH_CHANNEL_read
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_CHAR_PTR		pBuffer,
	FTM_UINT32			ulBufferSize,
	FTM_UINT32_PTR		pReadLen
);

FTM_RET	FTM_SSH_CHANNEL_readLine
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_CHAR_PTR		pBuffer,
	FTM_UINT32			ulBufferLen,
	FTM_UINT32_PTR		pReadLen
);

FTM_RET	FTM_SSH_CHANNEL_write
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_UINT8_PTR		pBuffer,
	FTM_UINT32			ulBufferLen
);

FTM_RET	FTM_SSH_CHANNEL_writeLine
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_CHAR_PTR		pBuffer
);

#endif
