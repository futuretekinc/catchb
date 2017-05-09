#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include "ftm_types.h"
#include "ftm_ret.h"
#include "ftm_params.h"
#include "ftm_server.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_cctv.h"
#include "ftm_catchb.h"

#undef	__MODULE__
#define	__MODULE__	"server"

typedef struct
{
	FTM_CMD				xCmd;
	FTM_CHAR_PTR			pCmdString;
	FTM_SERVICE_CALLBACK	fService;
}	FTM_SERVER_CMD_SET, _PTR_ FTM_SERVER_CMD_SET_PTR;

#define	MK_CMD_SET(CMD,FUN)	{CMD, #CMD, (FTM_SERVICE_CALLBACK)FUN }

static FTM_VOID_PTR FTM_SERVER_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTM_SERVER_service(FTM_VOID_PTR pData);
static FTM_BOOL		FTM_SERVER_SESSION_LIST_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static FTM_SERVER_CMD_SET	pCmdSet[] =
{
	MK_CMD_SET(FTM_CMD_ADD_CCTV,				FTM_SERVER_addCCTV),
	MK_CMD_SET(FTM_CMD_DEL_CCTV,				FTM_SERVER_delCCTV),
	MK_CMD_SET(FTM_CMD_GET_CCTV_COUNT,			FTM_SERVER_getCCTVCount),
	MK_CMD_SET(FTM_CMD_GET_CCTV_PROPERTIES,		FTM_SERVER_getCCTVProperties),
	MK_CMD_SET(FTM_CMD_SET_CCTV_PROPERTIES,		FTM_SERVER_setCCTVProperties),
	MK_CMD_SET(FTM_CMD_GET_CCTV_ID_LIST,		FTM_SERVER_getCCTVIDList),
	MK_CMD_SET(FTM_CMD_GET_CCTV_LIST,			FTM_SERVER_getCCTVList),
	MK_CMD_SET(FTM_CMD_SET_CCTV_POLICY,			FTM_SERVER_setCCTVPolicy),
	MK_CMD_SET(FTM_CMD_RESET_CCTV,				FTM_SERVER_resetCCTV),

	MK_CMD_SET(FTM_CMD_ADD_SWITCH,				FTM_SERVER_addSwitch),
	MK_CMD_SET(FTM_CMD_DEL_SWITCH,				FTM_SERVER_delSwitch),
	MK_CMD_SET(FTM_CMD_GET_SWITCH_COUNT,		FTM_SERVER_getSwitchCount),
	MK_CMD_SET(FTM_CMD_GET_SWITCH_PROPERTIES,	FTM_SERVER_getSwitchProperties),
	MK_CMD_SET(FTM_CMD_SET_SWITCH_PROPERTIES,	FTM_SERVER_setSwitchProperties),
	MK_CMD_SET(FTM_CMD_GET_SWITCH_ID_LIST,		FTM_SERVER_getSwitchIDList),
	MK_CMD_SET(FTM_CMD_GET_SWITCH_LIST,			FTM_SERVER_getSwitchList),

	MK_CMD_SET(FTM_CMD_GET_LOG_COUNT,			FTM_SERVER_getLogCount),
	MK_CMD_SET(FTM_CMD_GET_LOG_INFO,			FTM_SERVER_getLogInfo),
	MK_CMD_SET(FTM_CMD_GET_LOG_LIST,			FTM_SERVER_getLogList),
	MK_CMD_SET(FTM_CMD_DEL_LOG,					FTM_SERVER_delLog),
	
	MK_CMD_SET(FTM_CMD_ADD_ALARM,				FTM_SERVER_addAlarm),
	MK_CMD_SET(FTM_CMD_DEL_ALARM,				FTM_SERVER_delAlarm),
	MK_CMD_SET(FTM_CMD_GET_ALARM_COUNT,			FTM_SERVER_getAlarmCount),
	MK_CMD_SET(FTM_CMD_GET_ALARM,				FTM_SERVER_getAlarm),
	MK_CMD_SET(FTM_CMD_SET_ALARM,				FTM_SERVER_setAlarm),
	MK_CMD_SET(FTM_CMD_GET_ALARM_NAME_LIST,		FTM_SERVER_getAlarmNameList),

	MK_CMD_SET(FTM_CMD_GET_STAT_COUNT,			FTM_SERVER_getStatCount),
	MK_CMD_SET(FTM_CMD_GET_STAT_INFO,			FTM_SERVER_getStatInfo),
	MK_CMD_SET(FTM_CMD_GET_STAT_LIST,			FTM_SERVER_getStatList),
	MK_CMD_SET(FTM_CMD_DEL_STAT,				FTM_SERVER_delStat),
	MK_CMD_SET(FTM_CMD_DEL_STAT2,				FTM_SERVER_delStat2),
	
	MK_CMD_SET(FTM_CMD_UNKNOWN, 				NULL)
};


FTM_RET	FTM_SERVER_create
(
	FTM_CATCHB_PTR			pCatchB,
	FTM_SERVER_PTR	_PTR_	ppServer
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(ppServer != NULL);

	FTM_SERVER_PTR	pServer;

	pServer = (FTM_SERVER_PTR)FTM_MEM_malloc(sizeof(FTM_SERVER));
	if(pServer == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	strcpy(pServer->pName, __MODULE__);
	pServer->pCatchB = pCatchB;
	pServer->xConfig.usPort 		= FTM_SERVER_DEFAULT_PORT;
	pServer->xConfig.ulMaxSession 	= FTM_SERVER_DEFAULT_MAX_SESSION;
	pServer->xConfig.ulBufferLen 	= FTM_SERVER_DEFAULT_BUFFER_LEN;

	FTM_LIST_init(&pServer->xSessionList);
	FTM_LIST_setSeeker(&pServer->xSessionList, FTM_SERVER_SESSION_LIST_seeker);

	*ppServer = pServer;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_destroy
(
	FTM_SERVER_PTR _PTR_	ppServer
)
{
	ASSERT(ppServer != NULL);

	FTM_SERVER_stop(*ppServer);

	FTM_LIST_final(&(*ppServer)->xSessionList);

	FTM_MEM_free((*ppServer));

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_loadConfig
(
	FTM_SERVER_PTR			pServer,
	FTM_SERVER_CONFIG_PTR	pConfig
)
{
	ASSERT(pServer != NULL);
	ASSERT(pConfig != NULL);
#if 0
	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "server", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_USHORT	usPort;
		FTM_UINT32	ulSession;
		FTM_UINT32	ulBufferLen;

		xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xSection, "port", &usPort);
		if (xRet == FTM_RET_OK)
		{
			pServer->xConfig.usPort = usPort;
		}

		xRet = FTM_CONFIG_ITEM_getItemUINT32(&xSection, "session_count", &ulSession);
		if (xRet == FTM_RET_OK)
		{
			pServer->xConfig.ulMaxSession = ulSession;
		}

		xRet = FTM_CONFIG_ITEM_getItemUINT32(&xSection, "buffer_len", &ulBufferLen);
		if (xRet == FTM_RET_OK)
		{
			pServer->xConfig.ulBufferLen = ulBufferLen;
		}

	}
#endif
	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_start
(
	FTM_SERVER_PTR pServer
)
{
	ASSERT(pServer != NULL);

	if (pthread_create(&pServer->xThread, NULL, FTM_SERVER_process, (void *)pServer) < 0)
	{
		ERROR(FTM_RET_THREAD_CREATION_ERROR, "Failed to create pthread.");
		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_stop
(
	FTM_SERVER_PTR 		pServer
)
{
	ASSERT(pServer != NULL);

	if (!pServer->bStop)
	{
		pServer->bStop = FTM_TRUE;

		return	FTM_SERVER_waitingForFinished(pServer);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_waitingForFinished
(
	FTM_SERVER_PTR			pServer
)
{
	ASSERT(pServer != NULL);

	pthread_join(pServer->xThread, NULL);

	pServer->xThread = 0;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_SERVER_process(FTM_VOID_PTR pData)
{
	FTM_INT				nRet;
	struct sockaddr_in	xServer, xClient;
	FTM_SERVER_PTR		pServer =(FTM_SERVER_PTR)pData;

	ASSERT(pData != NULL);

	INFO("%s started.", pServer->pName);

	if (sem_init(&pServer->xSemaphore, 0, pServer->xConfig.ulMaxSession) < 0)
	{
		ERROR(FTM_RET_NOT_ENOUGH_MEMORY, "Can't alloc semaphore!\n");
		goto error;
	}

	pServer->hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pServer->hSocket == -1)
	{
		ERROR(FTM_RET_COMM_SOCK_ERROR, "Could not create socket\n");
		goto error;
	}

 	if( fcntl(pServer->hSocket, F_SETFL, O_NONBLOCK) == -1 )
	{
       ERROR(FTM_RET_COMM_CTRL_ERROR, "Listen socket nonblocking\n");
       goto error;
	}
				 

	xServer.sin_family 		= AF_INET;
	xServer.sin_addr.s_addr = INADDR_ANY;
	xServer.sin_port 		= htons( pServer->xConfig.usPort );

	nRet = bind( pServer->hSocket, (struct sockaddr *)&xServer, sizeof(xServer));
	if (nRet < 0)
	{
		ERROR(FTM_RET_COMM_SOCKET_BIND_FAILED, "bind failed.[nRet = %d]\n", nRet);
		goto error;
	}

	listen(pServer->hSocket, 3);


	while(!pServer->bStop)
	{
		FTM_INT	hClient;
		FTM_INT	nSockAddrInLen = sizeof(struct sockaddr_in);	

		hClient = accept(pServer->hSocket, (struct sockaddr *)&xClient, (socklen_t *)&nSockAddrInLen);
		if (hClient > 0)
		{
			FTM_RET	xRet;

			INFO("Accept new connection.[ %s:%d ]\n", inet_ntoa(xClient.sin_addr), ntohs(xClient.sin_port));

			
			FTM_SESSION_PTR pSession = NULL;
			
			xRet = FTM_SERVER_createSession(pServer, hClient, (struct sockaddr *)&xClient, &pSession);
			if (xRet != FTM_RET_OK)
			{
				close(hClient);
				INFO("The session(%08x) was closed.\n", hClient);
			}
			else
			{
				FTM_INT	nRet;

				nRet = pthread_create(&pSession->xThread, NULL, FTM_SERVER_service, pSession);
				if (nRet != 0)
				{
					ERROR(FTM_RET_CANT_CREATE_THREAD, "Can't create a thread[%d]\n", nRet);
					FTM_SERVER_destroySession(pServer, &pSession);
				}
			}
		}
		usleep(10000);
	}

	FTM_SESSION_PTR pSession;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		pSession->bStop = FTM_TRUE;
		shutdown(pSession->hSocket, SHUT_RD);
		pthread_join(pSession->xThread, 0);

		FTM_SERVER_destroySession(pServer, &pSession);
	}

	close(pServer->hSocket);

	INFO("Server stopped.");
error:

	return	0;
}

FTM_VOID_PTR FTM_SERVER_service(FTM_VOID_PTR pData)
{
	FTM_RET					xRet;
	FTM_SERVER_PTR			pServer;
	FTM_SESSION_PTR		pSession= (FTM_SESSION_PTR)pData;
	FTM_REQ_PARAMS_PTR		pReq 	= (FTM_REQ_PARAMS_PTR)pSession->pReqBuff;
	FTM_RESP_PARAMS_PTR	pResp 	= (FTM_RESP_PARAMS_PTR)pSession->pRespBuff;
	struct timespec			xTimeout;

	pServer = pSession->pServer;

	clock_gettime(CLOCK_REALTIME, &xTimeout);
	xTimeout.tv_sec += 2;
	if (sem_timedwait(&pSession->pServer->xSemaphore, &xTimeout) < 0)
	{
		INFO("The session(%08x) was closed\n", pSession->hSocket);
		shutdown(pSession->hSocket, SHUT_RD);
		return	0;	
	}

	while(!pSession->bStop)
	{
		int	nLen;

		nLen = recv(pSession->hSocket, pReq, pSession->ulReqBufferLen, 0);
		if (nLen == 0)
		{
			INFO("The connection is terminated.\n");
			break;	
		}
		else if (nLen < 0)
		{
			ERROR(FTM_RET_COMM_RECV_ERROR, "recv failed[%d]\n", -nLen);
			break;	
		}

		FTM_TIME_getCurrent(&pSession->xLastTime);

		pResp->ulLen = pSession->ulRespBufferLen;
		xRet = FTM_SERVER_serviceCall(pSession->pServer, pReq, pResp);
		if (FTM_RET_OK != xRet)
		{
			pResp->xCmd = pReq->xCmd;
			pResp->xRet = xRet;
			pResp->ulLen = sizeof(FTM_RESP_PARAMS);
		}

		INFO("send(%x, pResp, %d, MSG_DONTWAIT)", pSession->hSocket, pResp->ulLen);
		nLen = send(pSession->hSocket, pResp, pResp->ulLen, MSG_DONTWAIT);
		if (nLen < 0)
		{
			ERROR(FTM_RET_COMM_SEND_ERROR, "send failed[%d]\n", -nLen);	
			break;
		}
	}

	FTM_SERVER_destroySession(pServer, &pSession);

	sem_post(&pServer->xSemaphore);

	return	0;
}

FTM_RET	FTM_SERVER_serviceCall
(
	FTM_SERVER_PTR			pServer,
	FTM_REQ_PARAMS_PTR		pReq,
	FTM_RESP_PARAMS_PTR	pResp
)
{
	FTM_RET				xRet;
	FTM_SERVER_CMD_SET_PTR	pSet = pCmdSet;
	
	while(pSet->xCmd != FTM_CMD_UNKNOWN)
	{
		if (pSet->xCmd == pReq->xCmd)
		{
			INFO("Servce called : %s", pSet->pCmdString);
			xRet = pSet->fService(pServer, pReq, pResp);
			INFO("Servce returned : %d", xRet);

			return	xRet;
		}

		pSet++;
	}

	ERROR(FTM_RET_FUNCTION_NOT_SUPPORTED, "Function[%d] not supported.\n", pReq->xCmd);
	return	FTM_RET_FUNCTION_NOT_SUPPORTED;
}

FTM_BOOL	FTM_SERVER_SESSION_LIST_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTM_SESSION_PTR	pSession = (FTM_SESSION_PTR)pElement;
	pthread_t 			*pPThread = (pthread_t *)pIndicator;	

	return (pSession->xThread == *pPThread);
}

FTM_RET	FTM_SERVER_createSession
(
	FTM_SERVER_PTR pServer,
	FTM_INT			hClient,
	struct sockaddr *pSockAddr,
	FTM_SESSION_PTR _PTR_ ppSession
)
{
	ASSERT(pServer != NULL);
	ASSERT(pSockAddr != NULL);
	ASSERT(ppSession != NULL);
	
	FTM_RET	xRet;

	FTM_SESSION_PTR pSession = (FTM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTM_SESSION));
	if (pSession == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory!\n");
		goto error;

	}

	pSession->ulReqBufferLen = pServer->xConfig.ulBufferLen;
	pSession->pReqBuff = (FTM_UINT8_PTR)FTM_MEM_malloc(pServer->xConfig.ulBufferLen);
	if (pSession->pReqBuff == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory!\n");
		goto error;
	}

	pSession->ulRespBufferLen = pServer->xConfig.ulBufferLen;
	pSession->pRespBuff = (FTM_UINT8_PTR)FTM_MEM_malloc(pServer->xConfig.ulBufferLen);
	if (pSession->pRespBuff == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory!\n");
		goto error;
	}

	pSession->pServer = pServer;
	pSession->hSocket = hClient;
	memcpy(&pSession->xPeer, pSockAddr, sizeof(struct sockaddr));

	if (sem_init(&pSession->xSemaphore, 0, 1) < 0)
	{
		FTM_MEM_free(pSession);
		ERROR(FTM_RET_CANT_CREATE_SEMAPHORE, "Can't alloc semaphore!\n");

		return	FTM_RET_CANT_CREATE_SEMAPHORE;
	}

	xRet = FTM_LIST_append(&pServer->xSessionList, pSession);	
	if (xRet != FTM_RET_OK)
	{
		sem_destroy(&pSession->xSemaphore);	
		FTM_MEM_free(pSession);

		return	FTM_RET_LIST_NOT_INSERTABLE;
	}

	FTM_TIME_getCurrent(&pSession->xStartTime);

	*ppSession = pSession;
	
	return	FTM_RET_OK;

error:

	if (pSession != NULL)
	{
		if (pSession->pReqBuff != NULL)
		{
			FTM_MEM_free(pSession->pReqBuff);
			pSession->pReqBuff = NULL;
		}

		if (pSession->pRespBuff != NULL)
		{
			FTM_MEM_free(pSession->pRespBuff);
			pSession->pRespBuff = NULL;
		}
	}

	return	xRet;
}

FTM_RET	FTM_SERVER_destroySession
(
	FTM_SERVER_PTR pServer,
	FTM_SESSION_PTR _PTR_ ppSession
)
{
	ASSERT(pServer != NULL);
	ASSERT(ppSession != NULL);

	if ((*ppSession)->hSocket != 0)
	{
		INFO("The session(%08x) was closed\n", (*ppSession)->hSocket);
		close((*ppSession)->hSocket);
		(*ppSession)->hSocket = 0;
	}

	sem_destroy(&(*ppSession)->xSemaphore);

	FTM_LIST_remove(&pServer->xSessionList, (FTM_VOID_PTR)*ppSession);	

	if ((*ppSession)->pReqBuff != NULL)
	{
		FTM_MEM_free((*ppSession)->pReqBuff);
		(*ppSession)->pReqBuff = NULL;
	}

	if ((*ppSession)->pRespBuff != NULL)
	{
		FTM_MEM_free((*ppSession)->pRespBuff);
		(*ppSession)->pRespBuff = NULL;
	}

	FTM_MEM_free(*ppSession);

	*ppSession = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_getSessionCount(FTM_SERVER_PTR pServer, FTM_UINT32_PTR pulCount)
{
	ASSERT(pServer != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pServer->xSessionList, pulCount);
}

FTM_RET	FTM_SERVER_getSessionInfo(FTM_SERVER_PTR pServer, FTM_UINT32 ulIndex, FTM_SESSION_PTR pSession)
{
	ASSERT(pServer != NULL);
	ASSERT(pSession != NULL);
	FTM_RET				xRet;
	FTM_SESSION_PTR	pElement;

	xRet = FTM_LIST_getAt(&pServer->xSessionList, ulIndex, (FTM_VOID_PTR _PTR_)&pElement);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pSession, pElement, sizeof(FTM_SESSION));
	}

	return	xRet;
}

FTM_RET	FTM_SERVER_addCCTV
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ADD_CCTV_PARAMS_PTR		pReq,
	FTM_RESP_ADD_CCTV_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTM_CCTV_CONFIG	xConfig;

	memset(&xConfig, 0, sizeof(xConfig));

	strncpy(xConfig.pID, pReq->pID, sizeof(xConfig.pID) - 1);
	strncpy(xConfig.pIP, pReq->pIP, sizeof(xConfig.pIP) - 1);
	strncpy(xConfig.pSwitchID, pReq->pSwitchID, sizeof(xConfig.pSwitchID) - 1);
	strncpy(xConfig.pComment, pReq->pComment, sizeof(xConfig.pComment) - 1);
		
	xRet = FTM_CATCHB_addCCTV(pServer->pCatchB, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create CCTV[%s]", pReq->pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_ADD_CCTV_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_delCCTV
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_DEL_CCTV_PARAMS_PTR	pReq,
	FTM_RESP_DEL_CCTV_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	xRet = FTM_CATCHB_delCCTV(pServer->pCatchB, pReq->pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to destroy CCTV[%s]", pReq->pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_DEL_CCTV_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_getCCTVProperties
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_GET_CCTV_PROPERTIES_PARAMS_PTR	pReq,
	FTM_RESP_GET_CCTV_PROPERTIES_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTM_CCTV_CONFIG xConfig;

	xRet = FTM_CATCHB_getCCTVProperties(pServer->pCatchB, pReq->pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV[%s] properties", pReq->pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_GET_CCTV_PROPERTIES_PARAMS);
	memcpy(&pResp->xConfig, &xConfig, sizeof(FTM_CCTV_CONFIG));

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_setCCTVProperties
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_SET_CCTV_PROPERTIES_PARAMS_PTR	pReq,
	FTM_RESP_SET_CCTV_PROPERTIES_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	INFO("Set CCTV Properties(%s, %x)", pReq->pID, pReq->ulFieldFlags);
	xRet = FTM_CATCHB_setCCTVProperties(pServer->pCatchB, pReq->pID, &pReq->xConfig, pReq->ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set switch[%s] properties", pReq->pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SET_CCTV_PROPERTIES_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_getCCTVCount
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_GET_CCTV_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_GET_CCTV_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_GET_CCTV_COUNT_PARAMS);
	pResp->xCommon.xRet	= FTM_CATCHB_getCCTVCount(pServer->pCatchB, &pResp->ulCount);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_getCCTVIDList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_CCTV_ID_LIST_PARAMS_PTR		pReq,
	FTM_RESP_GET_CCTV_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;

	pCatchB = pServer->pCatchB;

	INFO("CCTV get id list !");
	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_GET_CCTV_ID_LIST_PARAMS)) / sizeof(FTM_ID);

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = FTM_CATCHB_getCCTVIDList(pCatchB, pResp->pIDList, ulMaxCount ,&pResp->ulCount);
	if (pResp->xCommon.xRet == FTM_RET_OK)
	{
		pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_CCTV_ID_LIST_PARAMS) + sizeof(FTM_ID) * pResp->ulCount;	
	}
	else
	{
		pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_CCTV_ID_LIST_PARAMS);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_getCCTVList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_CCTV_LIST_PARAMS_PTR	pReq,
	FTM_RESP_GET_CCTV_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount = 0;
	FTM_CCTV_PTR 	pCCTVList;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_GET_CCTV_LIST_PARAMS)) / sizeof(FTM_CCTV_CONFIG);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	pCCTVList = (FTM_CCTV_PTR)FTM_MEM_malloc(sizeof(FTM_CCTV)*ulMaxCount);
	if (pCCTVList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
	}
	else
	{
		xRet = FTM_CATCHB_getCCTVList(pCatchB, ulMaxCount, pCCTVList,&ulCount);
		if (xRet == FTM_RET_OK)
		{
			FTM_UINT32	i;

			for(i = 0 ; i < ulCount ; i++)
			{
				memcpy(&pResp->pCCTVList[i], &pCCTVList[i].xConfig, sizeof(FTM_CCTV_CONFIG));
			}
		}

		FTM_MEM_free(pCCTVList);
	}


	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_CCTV_LIST_PARAMS) + sizeof(FTM_CCTV_CONFIG) * ulCount;	
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_setCCTVPolicy
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SET_CCTV_POLICY_PARAMS_PTR	pReq,
	FTM_RESP_SET_CCTV_POLICY_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_CCTV_PTR	pCCTV;
	FTM_SWITCH_PTR	pSwitch;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getCCTV(pCatchB, pReq->pID, &pCCTV);
	if (xRet == FTM_RET_OK)
	{
		FTM_SWITCH_AC_PTR pAC;

		xRet = FTM_CATCHB_getSwitch(pCatchB, pCCTV->xConfig.pSwitchID, &pSwitch);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_SWITCH_addAC(pSwitch, pCCTV->xConfig.pIP, pReq->xPolicy, &pAC);
		}
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_SET_CCTV_POLICY_PARAMS);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_resetCCTV
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_RESET_CCTV_PARAMS_PTR	pReq,
	FTM_RESP_RESET_CCTV_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_CCTV_PTR	pCCTV;
	FTM_SWITCH_PTR	pSwitch;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getCCTV(pCatchB, pReq->pID, &pCCTV);
	if (xRet == FTM_RET_OK)
	{	
		if (strlen(pCCTV->xConfig.pHash) != 0)
		{
			if (strcasecmp(pCCTV->xConfig.pHash, pReq->pHash) != 0)
			{
				xRet = FTM_RET_INVALID_ARGUMENTS;
				ERROR(xRet, "Failed to reset cctv because invalid hash!");
				ERROR(xRet, "CCTV Hash : %s", pCCTV->xConfig.pHash);
				ERROR(xRet, " Req Hash : %s", pReq->pHash);
			}
		}

		if (xRet == FTM_RET_OK)
		{
			if (pCCTV->xConfig.xStat == FTM_CCTV_STAT_ABNORMAL)
			{
				FTM_SWITCH_AC_PTR pAC;

				xRet = FTM_CATCHB_getSwitch(pCatchB, pCCTV->xConfig.pSwitchID, &pSwitch);
				if (xRet == FTM_RET_OK)
				{
					xRet = FTM_SWITCH_addAC(pSwitch, pCCTV->xConfig.pID, FTM_SWITCH_AC_POLICY_ALLOW, &pAC);
				}
				else
				{
					INFO("Failed to get switch[%s]", pCCTV->xConfig.pSwitchID);	
					xRet = FTM_RET_OK;
				}
			}
		
			if (xRet == FTM_RET_OK)
			{
				xRet = FTM_CATCHB_resetCCTV(pCatchB, pReq->pID);
				if (xRet != FTM_RET_OK)
				{
					ERROR(xRet, "Failed to reset CCTV[%s]", pReq->pID);
				}
			}
		}
	}
	else
	{
		ERROR(xRet, "Failed to reset cctv because cctv[%s] not found!", pReq->pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_RESET_CCTV_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_addSwitch
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ADD_SWITCH_PARAMS_PTR	pReq,
	FTM_RESP_ADD_SWITCH_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	xRet = FTM_CATCHB_addSwitch(pServer->pCatchB, &pReq->xConfig, NULL);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create SWITCH[%s]", pReq->xConfig.pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_ADD_SWITCH_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_delSwitch
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_DEL_SWITCH_PARAMS_PTR	pReq,
	FTM_RESP_DEL_SWITCH_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	xRet = FTM_CATCHB_delSwitch(pServer->pCatchB, pReq->pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to destroy SWITCH[%s]", pReq->pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_DEL_SWITCH_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_getSwitchCount
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_GET_SWITCH_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_GET_SWITCH_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_GET_SWITCH_COUNT_PARAMS);
	pResp->xCommon.xRet	= FTM_CATCHB_getSwitchCount(pServer->pCatchB, &pResp->ulCount);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_getSwitchProperties
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_GET_SWITCH_PROPERTIES_PARAMS_PTR	pReq,
	FTM_RESP_GET_SWITCH_PROPERTIES_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;
	FTM_SWITCH_CONFIG xConfig;

	xRet = FTM_CATCHB_getSwitchProperties(pServer->pCatchB, pReq->pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get switch[%s] properties", pReq->pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_GET_SWITCH_PROPERTIES_PARAMS);
	memcpy(&pResp->xConfig, &xConfig, sizeof(FTM_SWITCH_CONFIG));

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_setSwitchProperties
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_SET_SWITCH_PROPERTIES_PARAMS_PTR	pReq,
	FTM_RESP_SET_SWITCH_PROPERTIES_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	INFO("Set Switch Properties(%s, %x)", pReq->pID, pReq->ulFieldFlags);
	xRet = FTM_CATCHB_setSwitchProperties(pServer->pCatchB, pReq->pID, &pReq->xConfig, pReq->ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set switch[%s] properties", pReq->pID);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SET_SWITCH_PROPERTIES_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_getSwitchIDList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_SWITCH_ID_LIST_PARAMS_PTR	pReq,
	FTM_RESP_GET_SWITCH_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_GET_SWITCH_ID_LIST_PARAMS)) / sizeof(FTM_ID);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	INFO("GetSwitchIDList(%d)", ulMaxCount);

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = FTM_CATCHB_getSwitchIDList(pCatchB, ulMaxCount, pResp->pIDList,&pResp->ulCount);
	if (pResp->xCommon.xRet == FTM_RET_OK)
	{
		pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_SWITCH_ID_LIST_PARAMS) + sizeof(FTM_ID) * pResp->ulCount;	
	}
	else
	{
		ERROR(xRet, "Failed to get switch ID list");
		pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_SWITCH_ID_LIST_PARAMS);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_getSwitchList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_SWITCH_LIST_PARAMS_PTR	pReq,
	FTM_RESP_GET_SWITCH_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount = 0;
	FTM_SWITCH_PTR _PTR_	ppSwitchList;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_GET_SWITCH_LIST_PARAMS)) / sizeof(FTM_SWITCH_CONFIG);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	ppSwitchList = (FTM_SWITCH_PTR _PTR_)FTM_MEM_malloc(sizeof(FTM_SWITCH_PTR)*ulMaxCount);
	if (ppSwitchList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
	}
	else
	{
		xRet = FTM_CATCHB_getSwitchList(pCatchB, ppSwitchList, ulMaxCount ,&ulCount);
		if (xRet == FTM_RET_OK)
		{
			FTM_UINT32	i;

			for(i = 0 ; i < ulCount ; i++)
			{
				memcpy(&pResp->pSwitchList[i], &ppSwitchList[i]->xConfig, sizeof(FTM_SWITCH_CONFIG));
			}
		}

		FTM_MEM_free(ppSwitchList);
	}


	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_SWITCH_LIST_PARAMS) + sizeof(FTM_SWITCH_CONFIG) * ulCount;	
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_getLogList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_LOG_LIST_PARAMS_PTR	pReq,
	FTM_RESP_GET_LOG_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount = 0;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_GET_LOG_LIST_PARAMS)) / sizeof(FTM_LOG);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	INFO("Start : %u, End : %u", pReq->ulBeginTime, pReq->ulEndTime);
	xRet = FTM_CATCHB_getLogList2(pCatchB, pReq->xType, ((pReq->pID[0] == 0)?NULL:pReq->pID), ((pReq->pIP[0] == 0)?NULL:pReq->pIP), 
								pReq->xStat, pReq->ulBeginTime, pReq->ulEndTime, pReq->ulIndex, ulMaxCount ,pResp->pLogList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log list!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_LOG_LIST_PARAMS) + sizeof(FTM_LOG) * ulCount;	

	INFO("GetLogList Response : %d", pResp->xCommon.ulLen);
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_delLog
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_DEL_LOG_PARAMS_PTR	pReq,
	FTM_RESP_DEL_LOG_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;
	FTM_UINT32		ulFirstTime= 0;
	FTM_UINT32		ulLastTime = 0;

	pCatchB = pServer->pCatchB;


	xRet = FTM_CATCHB_delLog(pCatchB, pReq->ulIndex, pReq->ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to delete log list[%u:%u]!", pReq->ulIndex, pReq->ulCount);
	}
	else
	{
		xRet = FTM_CATCHB_getLogInfo(pCatchB, &ulCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get log info!");
		}
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_DEL_LOG_PARAMS);
	pResp->ulCount 		= ulCount;
	pResp->ulFirstTime	= ulFirstTime;
	pResp->ulLastTime	= ulLastTime;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_getLogCount
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_LOG_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_GET_LOG_COUNT_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getLogCount(pCatchB, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log count!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_LOG_COUNT_PARAMS);
	pResp->ulCount = ulCount;

	return	xRet;
}

FTM_RET	FTM_SERVER_getLogInfo
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_LOG_INFO_PARAMS_PTR	pReq,
	FTM_RESP_GET_LOG_INFO_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;
	FTM_UINT32		ulFirstTime= 0;
	FTM_UINT32		ulLastTime = 0;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getLogInfo(pCatchB, &ulCount, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log info!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_LOG_INFO_PARAMS);
	pResp->ulCount = ulCount;
	pResp->ulFirstTime= ulFirstTime;
	pResp->ulLastTime= ulLastTime;

	return	xRet;
}


FTM_RET	FTM_SERVER_addAlarm
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ADD_ALARM_PARAMS_PTR	pReq,
	FTM_RESP_ADD_ALARM_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_ALARM_PTR	pAlarm;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_addAlarm(pCatchB, pReq->pName, pReq->pEmail, pReq->pMessage, &pAlarm);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add alarm[%s]!", pReq->pName);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_ADD_ALARM_PARAMS);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_delAlarm
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_DEL_ALARM_PARAMS_PTR	pReq,
	FTM_RESP_DEL_ALARM_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;

	pCatchB = pServer->pCatchB;


	xRet = FTM_CATCHB_delAlarm(pCatchB, pReq->pName);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to delete alarm[%s]!", pReq->pName);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_DEL_ALARM_PARAMS);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_getAlarmCount
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_ALARM_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_GET_ALARM_COUNT_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getAlarmCount(pCatchB, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log count!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_ALARM_COUNT_PARAMS);
	pResp->ulCount = ulCount;

	return	xRet;
}

FTM_RET	FTM_SERVER_getAlarm
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_ALARM_PARAMS_PTR	pReq,
	FTM_RESP_GET_ALARM_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getAlarm(pCatchB, pReq->pName, &pResp->xAlarm);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm[%s]!", pReq->pName);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_GET_ALARM_PARAMS);

	return	xRet;
}

FTM_RET	FTM_SERVER_setAlarm
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SET_ALARM_PARAMS_PTR	pReq,
	FTM_RESP_SET_ALARM_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_setAlarm(pCatchB, pReq->pName, &pReq->xAlarm, pReq->ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set alarm[%s]!", pReq->pName);
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SET_ALARM_PARAMS);

	return	xRet;
}

FTM_RET	FTM_SERVER_getAlarmNameList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_ALARM_NAME_LIST_PARAMS_PTR	pReq,
	FTM_RESP_GET_ALARM_NAME_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_GET_ALARM_NAME_LIST_PARAMS)) / sizeof(FTM_NAME);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;	
	}

	pResp->ulCount = 0;
	xRet = FTM_CATCHB_getAlarmNameList(pCatchB, pReq->ulIndex, ulMaxCount, pResp->pNameList, &pResp->ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm list!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_GET_ALARM_NAME_LIST_PARAMS) + sizeof(FTM_NAME) * pResp->ulCount;;

	return	xRet;
}

FTM_RET	FTM_SERVER_getStatList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_STAT_LIST_PARAMS_PTR	pReq,
	FTM_RESP_GET_STAT_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount = 0;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_GET_STAT_LIST_PARAMS)) / sizeof(FTM_STATISTICS);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	xRet = FTM_CATCHB_getStatisticsList(pCatchB, pReq->ulIndex, ulMaxCount ,pResp->pStatList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log list!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_STAT_LIST_PARAMS) + sizeof(FTM_STATISTICS) * ulCount;	
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_delStat
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_DEL_STAT_PARAMS_PTR	pReq,
	FTM_RESP_DEL_STAT_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;
	FTM_UINT32		ulFirstTime= 0;
	FTM_UINT32		ulLastTime = 0;

	pCatchB = pServer->pCatchB;


	xRet = FTM_CATCHB_delStatistics(pCatchB, pReq->ulIndex, pReq->ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to delete log list[%u:%u]!", pReq->ulIndex, pReq->ulCount);
	}
	else
	{
		xRet = FTM_CATCHB_getStatisticsInfo(pCatchB, &ulCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get log info!");
		}
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_DEL_STAT_PARAMS);
	pResp->ulCount 		= ulCount;
	pResp->ulFirstTime	= ulFirstTime;
	pResp->ulLastTime	= ulLastTime;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_delStat2
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_DEL_STAT2_PARAMS_PTR	pReq,
	FTM_RESP_DEL_STAT2_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;
	FTM_UINT32		ulFirstTime= 0;
	FTM_UINT32		ulLastTime = 0;

	pCatchB = pServer->pCatchB;


	xRet = FTM_CATCHB_delStatistics2(pCatchB, pReq->ulBeginTime, pReq->ulEndTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to delete log list[%s ~ %s]!", FTM_TIME_printf2(pReq->ulBeginTime, NULL), FTM_TIME_printf2(pReq->ulEndTime, NULL));
	}
	else
	{
		xRet = FTM_CATCHB_getStatisticsInfo(pCatchB, &ulCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get log info!");
		}
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_DEL_STAT2_PARAMS);
	pResp->ulCount 		= ulCount;
	pResp->ulFirstTime	= ulFirstTime;
	pResp->ulLastTime	= ulLastTime;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_getStatCount
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_STAT_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_GET_STAT_COUNT_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getStatisticsCount(pCatchB, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log count!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_STAT_COUNT_PARAMS);
	pResp->ulCount = ulCount;

	return	xRet;
}

FTM_RET	FTM_SERVER_getStatInfo
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_GET_STAT_INFO_PARAMS_PTR	pReq,
	FTM_RESP_GET_STAT_INFO_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;
	FTM_UINT32		ulFirstTime= 0;
	FTM_UINT32		ulLastTime = 0;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getStatisticsInfo(pCatchB, &ulCount, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log info!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_GET_STAT_INFO_PARAMS);
	pResp->ulCount = ulCount;
	pResp->ulFirstTime= ulFirstTime;
	pResp->ulLastTime= ulLastTime;

	return	xRet;
}

