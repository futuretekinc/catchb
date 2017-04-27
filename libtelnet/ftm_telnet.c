/*
 * Sean Middleditch
 * sean@sourcemud.org
 *
 * The author or authors of this code dedicate any and all copyright interest
 * in this code to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and successors. We
 * intend this dedication to be an overt act of relinquishment in perpetuity of
 * all present and future rights to this code under copyright law. 
 */

#if !defined(_POSIX_SOURCE)
#	define _POSIX_SOURCE
#endif
#if !defined(_BSD_SOURCE)
#	define _BSD_SOURCE
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#ifdef HAVE_ZLIB
#include "zlib.h"
#endif

#include "ftm_telnet.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_buffer.h"

#undef	__MODULE__
#define __MODULE__ "telnet"

static int do_echo;

static const telnet_telopt_t telopts[] = 
{
//	{ TELNET_TELOPT_ECHO,		TELNET_WONT, TELNET_DO   },
//	{ TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DONT },
	{ TELNET_TELOPT_COMPRESS2,	TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_MSSP,		TELNET_WONT, TELNET_DO   },
	{ -1, 0, 0 }
};

FTM_VOID_PTR	FTM_TELNET_CLIENT_threadMain
(
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTM_TELNET_CLIENT_recv
(
	FTM_TELNET_CLIENT_PTR	pClient,
	const FTM_CHAR _PTR_ 	pBuffer,
	FTM_UINT32				ulSize
)
{
	FTM_LOCK_set(pClient->pLock);

	FTM_BUFFER_pushBack(pClient->pBuffer, (FTM_UINT8_PTR)pBuffer, ulSize);

	FTM_LOCK_reset(pClient->pLock);

	return	FTM_RET_OK;
}

static 
FTM_RET	FTM_TELNET_CLIENT_send
(
	FTM_TELNET_CLIENT_PTR	pClient, 
	const FTM_CHAR _PTR_ pBuffer, 
	FTM_UINT32			nSize
) 
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nSendLen;

	INFO("TELNET : Send[%d] - %s", nSize, pBuffer);
	/* send data */
	while (nSize > 0) 
	{
		nSendLen = send(pClient->nSock, pBuffer, nSize, 0);
		if (nSendLen == 0) 
		{
			xRet = FTM_RET_SOCKET_CLOSED;
			ERROR(xRet, "Failed to send!");
			break;
		} 
		else if (nSendLen < 0) 
		{
			xRet = FTM_RET_SOCKET_ABNORMAL_DISCONNECTED;
			ERROR(xRet, "Failed to send!");
			break;
		}

		/* update pointer and size to see if we've got more to send */
		pBuffer += nSendLen;
		nSize -= nSendLen;
	}

	return	xRet;
}

static 
FTM_VOID	FTM_TELNET_CLIENT_eventHandler
(
	telnet_t _PTR_ telnet, 
	telnet_event_t _PTR_ ev, 
	FTM_VOID_PTR	pData
) 
{
	ASSERT(telnet != NULL);
	ASSERT(ev != NULL);
	ASSERT(pData != NULL);

	FTM_TELNET_CLIENT_PTR	pClient = (FTM_TELNET_CLIENT_PTR)pData;

	switch (ev->type) 
	{
	/* data received */
	case TELNET_EV_DATA:
		FTM_TELNET_CLIENT_recv(pClient, ev->data.buffer, ev->data.size);
		break;
	/* data must be sent */
	case TELNET_EV_SEND:
		FTM_TELNET_CLIENT_send(pClient, ev->data.buffer, ev->data.size);
		break;
	/* request to enable remote feature (or receipt) */
	case TELNET_EV_WILL:
		/* we'll agree to turn off our echo if server wants us to stop */
		if (ev->neg.telopt == TELNET_TELOPT_ECHO)
			do_echo = 0;
		break;
	/* notification of disabling remote feature (or receipt) */
	case TELNET_EV_WONT:
		if (ev->neg.telopt == TELNET_TELOPT_ECHO)
			do_echo = 1;
		break;
	/* request to enable local feature (or receipt) */
	case TELNET_EV_DO:
		break;
	/* demand to disable local feature (or receipt) */
	case TELNET_EV_DONT:
		break;
	/* respond to TTYPE commands */
	case TELNET_EV_TTYPE:
		/* respond with our terminal type, if requested */
		if (ev->ttype.cmd == TELNET_TTYPE_SEND) {
	//		telnet_ttype_is(telnet, getenv("TERM"));
		}
		break;
	/* respond to particular subnegotiations */
	case TELNET_EV_SUBNEGOTIATION:
		break;
	/* error */
	case TELNET_EV_ERROR:
		INFO("ERROR: %s\n", ev->error.msg);
		break;
	default:
		/* ignore */
		break;
	}
}

FTM_RET	FTM_TELNET_CLIENT_create
(
	FTM_TELNET_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_TELNET_CLIENT_PTR	pClient = NULL;

	pClient = (FTM_TELNET_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTM_TELNET_CLIENT));
	if (pClient == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create telnet client!");
		goto finished;
	}

	xRet = FTM_LOCK_create(&pClient->pLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create lock!");
		goto finished;
	}

	xRet = FTM_BUFFER_create(4096, &pClient->pBuffer);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create buffer!");
		goto finished;	
	}

	strcpy(pClient->pName, __MODULE__);
	pClient->bStop = FTM_TRUE;
	
	*ppClient = pClient;

finished:
	if (xRet != FTM_RET_OK)
	{
		if (pClient != NULL)
		{
			if (pClient->pBuffer != NULL)
			{
				FTM_BUFFER_destroy(&pClient->pBuffer);	
			}

			if (pClient->pLock != NULL)
			{
				FTM_LOCK_destroy(&pClient->pLock);	
			}

			FTM_MEM_free(pClient);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_TELNET_CLIENT_destroy
(
	FTM_TELNET_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_RET	xRet;

	FTM_TELNET_CLIENT_close(*ppClient);

	xRet = FTM_BUFFER_destroy(&(*ppClient)->pBuffer);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to destroy buffer!");
	}

	xRet = FTM_LOCK_destroy(&(*ppClient)->pLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to destroy lock!");
	}

	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TELNET_CLIENT_open
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pHost,
	FTM_UINT16				usPort
)
{
	FTM_RET	xRet = FTM_RET_OK;
	int rs;
	struct sockaddr_in addr;
	struct addrinfo *ai;
	struct addrinfo xHints;

	/* look up server host */
	memset(&xHints, 0, sizeof(xHints));
	xHints.ai_family = AF_UNSPEC;
	xHints.ai_socktype = SOCK_STREAM;
	if ((rs = getaddrinfo(pHost, "23", &xHints, &ai)) != 0) 
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get address info[%s:%s]\n", pHost, gai_strerror(rs));
		return xRet;
	}

	/* create server socket */
	pClient->nSock = socket(AF_INET, SOCK_STREAM, 0);
	if (pClient->nSock == -1) 
	{
		xRet = FTM_RET_SOCKET_CREATION_FAILED;
		ERROR(xRet, "Failed to create socket[%s]", strerror(errno));

		goto finished;
	}
	/* bind server socket */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (bind(pClient->nSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) 
	{
		xRet = FTM_RET_SOCKET_BIND_FAILED;
		ERROR(xRet, "Failed to bind socket[%s]", strerror(errno));

		goto finished;
	}

	/* connect */
	if (connect(pClient->nSock, ai->ai_addr, ai->ai_addrlen) == -1) 
	{
		xRet = FTM_RET_SOCKET_CONNECTION_FAILED;
		ERROR(xRet, "Failed to connect socket[%s]", strerror(errno));

		goto finished;
	}

	/* free address lookup info */
	freeaddrinfo(ai);

	/* set input echoing on by default */
	do_echo = 1;

	/* initialize telnet box */
	pClient->pHandler = telnet_init(telopts, FTM_TELNET_CLIENT_eventHandler, 0, pClient);
	if (pClient->pHandler == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to init telnet!");
		goto finished;	
	}

	/* initialize poll descriptors */
	memset(pClient->pFD, 0, sizeof(pClient->pFD));
	pClient->pFD[0].fd = pClient->nSock;
	pClient->pFD[0].events = POLLIN;

	if (pthread_create(&pClient->xThread, NULL, FTM_TELNET_CLIENT_threadMain, pClient) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		ERROR(xRet, "Failed to create thread!");

		goto finished;
	}

finished:

	if (xRet != FTM_RET_OK)
	{
		if (pClient->nSock != 0)	
		{
			if (pClient->pHandler != NULL)
			{
				telnet_free(pClient->pHandler);	
				pClient->pHandler = NULL;	
			}

			if (pClient->nSock != 0)
			{
				close(pClient->nSock);
				pClient->nSock = 0;
			}
		}
	}

	return	xRet;
}

FTM_RET	FTM_TELNET_CLIENT_close
(
	FTM_TELNET_CLIENT_PTR	pClient
)
{
	/* clean up */
	pClient->bStop = FTM_TRUE;

	if (pClient->xThread != 0)
	{
		pthread_join(pClient->xThread, NULL);
		pClient->xThread = 0;
	}

	if (pClient->pHandler != NULL)
	{
		telnet_free(pClient->pHandler);
		pClient->pHandler = NULL;
	}

	if (pClient->nSock > 0)
	{
		close(pClient->nSock);
		pClient->nSock = 0;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TELNET_CLIENT_write
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pData,
	FTM_UINT32				ulDataLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	i;
	FTM_CHAR	pBuffer[512];
	FTM_UINT32	ulBufferLen = 0;

	//static FTM_CHAR	crlf[] = { '\r', '\n' };

	if (pClient->pHandler == NULL)
	{
		xRet = FTM_RET_SOCKET_CLOSED;
		goto finished;
	}

	for(i = 0 ; (i < ulDataLen) && (i < sizeof(pBuffer) - 1) ; i++)
	{
		if ((pData[i] == '\n') || (pData[i] == '\r'))
		{
			pBuffer[ulBufferLen++] = '\r';
			pBuffer[ulBufferLen++] = '\n';
			//telnet_send(pClient->pHandler, crlf, 2);
		}
		else
		{
			pBuffer[ulBufferLen++] = pData[i];
			//telnet_send(pClient->pHandler, &pData[i], 1);
		}
	}
	pBuffer[ulBufferLen] = 0;

	telnet_send(pClient->pHandler, pBuffer, ulBufferLen);

finished:
	return	xRet;
}

FTM_RET	FTM_TELNET_CLIENT_writel
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pData,
	FTM_UINT32				ulDataLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pData != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	i;
	FTM_CHAR	pBuffer[512];
	FTM_UINT32	ulBufferLen = 0;

	//static FTM_CHAR	crlf[] = { '\r', '\n' };

	if (pClient->pHandler == NULL)
	{
		xRet = FTM_RET_SOCKET_CLOSED;
		goto finished;
	}

	for(i = 0 ; (i < ulDataLen) && (i < sizeof(pBuffer) - 3) ; i++)
	{
		if ((pData[i] == '\n') || (pData[i] == '\r'))
		{
			pBuffer[ulBufferLen++] = '\r';
			pBuffer[ulBufferLen++] = '\n';
			//telnet_send(pClient->pHandler, crlf, 2);
		}
		else
		{
			pBuffer[ulBufferLen++] = pData[i];
			//telnet_send(pClient->pHandler, &pData[i], 1);
		}
	}
	pBuffer[ulBufferLen++] = '\r';
	pBuffer[ulBufferLen++] = '\n';
	pBuffer[ulBufferLen] = 0;

	telnet_send(pClient->pHandler, pBuffer, ulBufferLen);

finished:
	return	xRet;
}

FTM_RET	FTM_TELNET_CLIENT_read
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pBuffer,
	FTM_UINT32				ulBufferSize,
	FTM_UINT32_PTR			pReadLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pBuffer != NULL);
	ASSERT(pReadLen != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	if (pClient->pHandler == NULL)
	{
		xRet = FTM_RET_SOCKET_CLOSED;
		ERROR(xRet, "Failed to read");
		goto finished;
	}

	FTM_LOCK_set(pClient->pLock);

	FTM_BUFFER_popFront(pClient->pBuffer, (FTM_UINT8_PTR)pBuffer, ulBufferSize, pReadLen);

	INFO("TELNET[%08x] : Read = %d", pClient, *pReadLen);

finished:

	FTM_LOCK_reset(pClient->pLock);
	return	xRet;
}


FTM_RET	FTM_TELNET_CLIENT_readline
(
	FTM_TELNET_CLIENT_PTR	pClient,
	FTM_CHAR_PTR			pBuffer,
	FTM_UINT32				ulBufferSize,
	FTM_UINT32_PTR			pReadLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pBuffer != NULL);
	ASSERT(pReadLen != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	ulReadLen = 0;
	FTM_UINT32	ulRcvdLen = 0;
	FTM_UINT32	ulLen = 0;
	FTM_UINT8	nData;

	if (pClient->pHandler == NULL)
	{
		xRet = FTM_RET_SOCKET_CLOSED;
		goto finished;
	}

	FTM_LOCK_set(pClient->pLock);

	FTM_BUFFER_getSize(pClient->pBuffer, &ulRcvdLen);
	if (ulRcvdLen == 0)
	{
		goto finished;
	}

	while(ulRcvdLen != 0)
	{
		xRet = FTM_BUFFER_getFirst(pClient->pBuffer, &nData);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get first data in buffer!");
			goto finished;
		}

		if ((nData != '\n') && (nData != '\r'))
		{
			break;	
		}

		xRet = FTM_BUFFER_popFront(pClient->pBuffer, &nData, 1, &ulLen);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get first data in buffer!");
			goto finished;
		}

		ulRcvdLen--;
	}

	if (ulRcvdLen == 0)
	{
		goto finished;	
	}

	xRet = FTM_BUFFER_getFirst(pClient->pBuffer, &nData);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get first data in buffer!");
		goto finished;
	}

	if (nData == 0x1b)
	{
		if (ulRcvdLen < 3)
		{
			goto finished;
		}

		xRet = FTM_BUFFER_popFront(pClient->pBuffer, (FTM_UINT8_PTR)pBuffer, 3, &ulLen);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get data from buffer!");
			goto finished;	
		}

		ulReadLen = 3;
	}
	else
	{
		FTM_UINT32	i;

		for(i = 0 ; i < ulRcvdLen  && i < ulBufferSize ; i++)
		{
			xRet = FTM_BUFFER_getFirst(pClient->pBuffer, &nData);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to get first data from buffer!");
				goto finished;	
			}

			if((nData == '\n') || (nData == '\r') || (nData == 0x1b))
			{
				break;	
			}

			xRet = FTM_BUFFER_popFront(pClient->pBuffer, (FTM_UINT8_PTR)&pBuffer[ulReadLen], 1, &ulLen);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to get first data from buffer!");
				goto finished;	
			}

			ulReadLen++;
		}
	}

finished:

	*pReadLen = ulReadLen;
	FTM_LOCK_reset(pClient->pLock);
	return	xRet;
}


FTM_VOID_PTR	FTM_TELNET_CLIENT_threadMain
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_TELNET_CLIENT_PTR	pClient = (FTM_TELNET_CLIENT_PTR)pData;

	INFO("%s started.", pClient->pName);
	pClient->bStop = FTM_FALSE;

	/* loop while both connections are open */
	while (!pClient->bStop)
	{
		if (poll(pClient->pFD, 1, 100) != -1) 
		{
			/* read from client */
			if (pClient->pFD[0].revents & POLLIN) 
			{
				FTM_CHAR	pBuffer[512];
				FTM_INT		nReadLen;

				nReadLen = recv(pClient->nSock, pBuffer, sizeof(pBuffer), 0);
				if (nReadLen > 0) 
				{
					telnet_recv(pClient->pHandler, pBuffer, nReadLen);
				} 
				else if (nReadLen == 0) 
				{
					break;
				} 
				else 
				{
					ERROR(FTM_RET_SOCKET_ABNORMAL_DISCONNECTED, "Failed to receive[%s]!", strerror(errno));
					break;
				}
			}
		}
		else
		{
			printf("poll timeout!\n");	
		}
	}

	INFO("%s stopped.", pClient->pName);
	pClient->bStop = FTM_TRUE;

	return 0;
}
