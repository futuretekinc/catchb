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
#include "ftm_ssid.h"

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
	MK_CMD_SET(FTM_CMD_CCTV_ADD,				FTM_SERVER_CCTV_add),
	MK_CMD_SET(FTM_CMD_CCTV_DEL,				FTM_SERVER_CCTV_del),
	MK_CMD_SET(FTM_CMD_CCTV_GET_COUNT,			FTM_SERVER_CCTV_getCount),
	MK_CMD_SET(FTM_CMD_CCTV_GET_PROPERTIES,		FTM_SERVER_CCTV_getProperties),
	MK_CMD_SET(FTM_CMD_CCTV_SET_PROPERTIES,		FTM_SERVER_CCTV_setProperties),
	MK_CMD_SET(FTM_CMD_CCTV_GET_ID_LIST,		FTM_SERVER_CCTV_getIDList),
	MK_CMD_SET(FTM_CMD_CCTV_GET_LIST,			FTM_SERVER_CCTV_getList),
	MK_CMD_SET(FTM_CMD_CCTV_SET_POLICY,			FTM_SERVER_CCTV_setPolicy),
	MK_CMD_SET(FTM_CMD_CCTV_RESET,				FTM_SERVER_CCTV_reset),

	MK_CMD_SET(FTM_CMD_SWITCH_ADD,				FTM_SERVER_SWITCH_add),
	MK_CMD_SET(FTM_CMD_SWITCH_DEL,				FTM_SERVER_SWITCH_del),
	MK_CMD_SET(FTM_CMD_SWITCH_GET_COUNT,		FTM_SERVER_SWITCH_getCount),
	MK_CMD_SET(FTM_CMD_SWITCH_GET_PROPERTIES,	FTM_SERVER_SWITCH_getProperties),
	MK_CMD_SET(FTM_CMD_SWITCH_SET_PROPERTIES,	FTM_SERVER_SWITCH_setProperties),
	MK_CMD_SET(FTM_CMD_SWITCH_GET_ID_LIST,		FTM_SERVER_SWITCH_getIDList),
	MK_CMD_SET(FTM_CMD_SWITCH_GET_LIST,			FTM_SERVER_SWITCH_getList),

	MK_CMD_SET(FTM_CMD_LOG_GET_COUNT,			FTM_SERVER_LOG_getCount),
	MK_CMD_SET(FTM_CMD_LOG_GET_COUNT2,			FTM_SERVER_LOG_getCount2),
	MK_CMD_SET(FTM_CMD_LOG_GET_INFO,			FTM_SERVER_LOG_getInfo),
	MK_CMD_SET(FTM_CMD_LOG_GET_LIST,			FTM_SERVER_LOG_getList),
	MK_CMD_SET(FTM_CMD_LOG_DEL,					FTM_SERVER_LOG_del),
	
	MK_CMD_SET(FTM_CMD_ALARM_ADD,				FTM_SERVER_ALARM_add),
	MK_CMD_SET(FTM_CMD_ALARM_DEL,				FTM_SERVER_ALARM_del),
	MK_CMD_SET(FTM_CMD_ALARM_GET_COUNT,			FTM_SERVER_ALARM_getCount),
	MK_CMD_SET(FTM_CMD_ALARM_GET,				FTM_SERVER_ALARM_get),
	MK_CMD_SET(FTM_CMD_ALARM_SET,				FTM_SERVER_ALARM_set),
	MK_CMD_SET(FTM_CMD_ALARM_GET_NAME_LIST,		FTM_SERVER_ALARM_getNameList),

	MK_CMD_SET(FTM_CMD_SMTP_SET,				FTM_SERVER_SMTP_set),
	MK_CMD_SET(FTM_CMD_SMTP_GET,				FTM_SERVER_SMTP_get),

	MK_CMD_SET(FTM_CMD_STAT_GET_COUNT,			FTM_SERVER_STAT_getCount),
	MK_CMD_SET(FTM_CMD_STAT_SET_INFO,			FTM_SERVER_STAT_setInfo),
	MK_CMD_SET(FTM_CMD_STAT_GET_INFO,			FTM_SERVER_STAT_getInfo),
	MK_CMD_SET(FTM_CMD_STAT_GET_LIST,			FTM_SERVER_STAT_getList),
	MK_CMD_SET(FTM_CMD_STAT_DEL,				FTM_SERVER_STAT_del),
	MK_CMD_SET(FTM_CMD_STAT_DEL2,				FTM_SERVER_STAT_del2),
	
	MK_CMD_SET(FTM_CMD_SSID_CREATE,				FTM_SERVER_SSID_create),
	MK_CMD_SET(FTM_CMD_SSID_DEL,				FTM_SERVER_SSID_del),
	MK_CMD_SET(FTM_CMD_SSID_VERIFY,				FTM_SERVER_SSID_verify),

	MK_CMD_SET(FTM_CMD_UNKNOWN, 				NULL)
};

FTM_RET	FTM_SERVER_CONFIG_setDefault
(
	FTM_SERVER_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	pConfig->usPort 		= FTM_SERVER_DEFAULT_PORT;
	pConfig->ulMaxSession 	= FTM_SERVER_DEFAULT_MAX_SESSION;
	pConfig->ulBufferLen 	= FTM_SERVER_DEFAULT_BUFFER_LEN;
	pConfig->ulSSIDTimeout	= FTM_SERVER_DEFAULT_SSID_TIMEOUT;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_CONFIG_load
(
	FTM_SERVER_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);
	
	FTM_RET	xRet = FTM_RET_OK;
	cJSON _PTR_ 	pSection;
	cJSON _PTR_ 	pItem;

	pItem = cJSON_GetObjectItem(pRoot, "port");
	if (pItem != NULL)
	{
		pConfig->usPort = pItem->valueint;
	}

	pItem = cJSON_GetObjectItem(pRoot, "session_count");
	if (pItem != NULL)
	{
		pConfig->ulMaxSession = pItem->valueint;
	}

	pItem = cJSON_GetObjectItem(pRoot, "buffer_size");
	if (pItem != NULL)
	{
		pConfig->ulBufferLen= pItem->valueint;
	}

	pSection = cJSON_GetObjectItem(pRoot, "ssid");
	if (pSection != NULL)
	{
		pItem = cJSON_GetObjectItem(pSection, "timeout");
		if (pItem != NULL)
		{
			pConfig->ulSSIDTimeout= pItem->valueint;
		}
	}

	return	xRet;
}

FTM_RET	FTM_SERVER_CONFIG_save
(
	FTM_SERVER_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
)
{
	cJSON_AddNumberToObject(pRoot, "port", 			pConfig->usPort);
	cJSON_AddNumberToObject(pRoot, "session_count", pConfig->ulMaxSession);
	cJSON_AddNumberToObject(pRoot, "buffer_size", 	pConfig->ulBufferLen);

	cJSON _PTR_ pSection = cJSON_CreateObject();
	cJSON_AddNumberToObject(pSection, "timeout", 		pConfig->ulSSIDTimeout);
	cJSON_AddItemToObject(pRoot, "ssid", 		pSection);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_CONFIG_show
(
	FTM_SERVER_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
)
{
	printf("\n[ Server ]\n");
	printf("%16s : %d\n", "Port", pConfig->usPort);
	printf("%16s : %d\n",	"Session Count",pConfig->ulMaxSession);
	printf("%16s : %d\n",	"Buffer Size", 	pConfig->ulBufferLen);
	printf("%16s : %d\n",	"SSID Timeout",	pConfig->ulSSIDTimeout);

	return	FTM_RET_OK;
}

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

	FTM_SERVER_CONFIG_setDefault(&pServer->xConfig);

	FTM_LIST_init(&pServer->xSessionList);
	FTM_LIST_init(&pServer->xReleaseSessionList);
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

	FTM_LIST_final(&(*ppServer)->xReleaseSessionList);
	FTM_LIST_final(&(*ppServer)->xSessionList);

	FTM_MEM_free((*ppServer));

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_setConfig
(
	FTM_SERVER_PTR			pServer,
	FTM_SERVER_CONFIG_PTR	pConfig
)
{
	ASSERT(pServer != NULL);
	ASSERT(pConfig != NULL);

	INFO("Set Config!");
	memcpy(&pServer->xConfig, pConfig, sizeof(FTM_SERVER_CONFIG));

	FTM_SERVER_CONFIG_show(&pServer->xConfig, 0);
	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_getConfig
(
	FTM_SERVER_PTR			pServer,
	FTM_SERVER_CONFIG_PTR	pConfig
)
{
	ASSERT(pServer != NULL);
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &pServer->xConfig, sizeof(FTM_SERVER_CONFIG));

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
		FTM_UINT32	ulCount = 0;

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


		FTM_LIST_count(&pServer->xReleaseSessionList, &ulCount);
		if (ulCount != 0)
		{
			FTM_SESSION_PTR	pSession = NULL;

			while(1)
			{
				FTM_LIST_count(&pServer->xReleaseSessionList, &ulCount);
				
				if (ulCount == 0)
				{
					break;
				}

				if (FTM_LIST_getFirst(&pServer->xReleaseSessionList, (FTM_VOID_PTR _PTR_)&pSession) != FTM_RET_OK)
				{
					break;
				}

				FTM_LIST_removeAt(&pServer->xReleaseSessionList, 0);
				

				pSession->bStop = FTM_TRUE;
				shutdown(pSession->hSocket, SHUT_RD);
				pthread_join(pSession->xThread, 0);

				FTM_SERVER_destroySession(pServer, &pSession);
			}

			FTM_LIST_count(&pServer->xSessionList, &ulCount);
			INFO("Session Count : %d", ulCount);
		}

		usleep(1000);
	}

	FTM_SESSION_PTR pSession;

	FTM_LIST_iteratorStart(&pServer->xSessionList);
	while(FTM_LIST_iteratorNext(&pServer->xSessionList, (FTM_VOID_PTR _PTR_)&pSession) == FTM_RET_OK)
	{
		INFO("Session rlease(%x)", pSession->hSocket);
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
	FTM_SERVER_PTR		pServer;
	FTM_SESSION_PTR		pSession= (FTM_SESSION_PTR)pData;
	FTM_REQ_PARAMS_PTR	pReq 	= (FTM_REQ_PARAMS_PTR)pSession->pReqBuff;
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

	FTM_LIST_append(&pServer->xReleaseSessionList, pSession);
	FTM_UINT32	ulCount = 0;	
	FTM_LIST_count(&pServer->xReleaseSessionList, &ulCount);
//	FTM_SERVER_destroySession(pServer, &pSession);

	sem_post(&pServer->xSemaphore);

//	pthread_exit(0);

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

	if (pReq->xCmd != FTM_CMD_SSID_CREATE)
	{
		xRet = FTM_SSID_isValid(pReq->pSSID, pServer->xConfig.ulSSIDTimeout);
		
		if (xRet != FTM_RET_OK)
		{
			INFO("Invalid SSID[%d]\n", pReq->xCmd);
			return	xRet;
		}
	}

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

FTM_RET	FTM_SERVER_CCTV_add
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_CCTV_ADD_PARAMS_PTR		pReq,
	FTM_RESP_CCTV_ADD_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_CCTV_ADD_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_CCTV_del
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_CCTV_DEL_PARAMS_PTR	pReq,
	FTM_RESP_CCTV_DEL_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_CCTV_DEL_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_CCTV_getProperties
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_CCTV_GET_PROPERTIES_PARAMS_PTR	pReq,
	FTM_RESP_CCTV_GET_PROPERTIES_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_CCTV_GET_PROPERTIES_PARAMS);
	memcpy(&pResp->xConfig, &xConfig, sizeof(FTM_CCTV_CONFIG));

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_CCTV_setProperties
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_CCTV_SET_PROPERTIES_PARAMS_PTR	pReq,
	FTM_RESP_CCTV_SET_PROPERTIES_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_CCTV_SET_PROPERTIES_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_CCTV_getCount
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_CCTV_GET_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_CCTV_GET_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_CCTV_GET_COUNT_PARAMS);
	pResp->xCommon.xRet	= FTM_CATCHB_getCCTVCount(pServer->pCatchB, &pResp->ulCount);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_CCTV_getIDList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_CCTV_GET_ID_LIST_PARAMS_PTR		pReq,
	FTM_RESP_CCTV_GET_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;

	pCatchB = pServer->pCatchB;

	INFO("CCTV get id list !");
	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_CCTV_GET_ID_LIST_PARAMS)) / sizeof(FTM_ID);

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = FTM_CATCHB_getCCTVIDList(pCatchB, pResp->pIDList, ulMaxCount ,&pResp->ulCount);
	if (pResp->xCommon.xRet == FTM_RET_OK)
	{
		pResp->xCommon.ulLen = sizeof(FTM_RESP_CCTV_GET_ID_LIST_PARAMS) + sizeof(FTM_ID) * pResp->ulCount;	
	}
	else
	{
		pResp->xCommon.ulLen = sizeof(FTM_RESP_CCTV_GET_ID_LIST_PARAMS);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_CCTV_getList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_CCTV_GET_LIST_PARAMS_PTR	pReq,
	FTM_RESP_CCTV_GET_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount = 0;
	FTM_CCTV_PTR 	pCCTVList;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_CCTV_GET_LIST_PARAMS)) / sizeof(FTM_CCTV_CONFIG);
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_CCTV_GET_LIST_PARAMS) + sizeof(FTM_CCTV_CONFIG) * ulCount;	
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_CCTV_setPolicy
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_CCTV_SET_POLICY_PARAMS_PTR	pReq,
	FTM_RESP_CCTV_SET_POLICY_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_CCTV_SET_POLICY_PARAMS);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_CCTV_reset
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_CCTV_RESET_PARAMS_PTR	pReq,
	FTM_RESP_CCTV_RESET_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_CCTV_RESET_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_SWITCH_add
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SWITCH_ADD_PARAMS_PTR	pReq,
	FTM_RESP_SWITCH_ADD_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SWITCH_ADD_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_SWITCH_del
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_SWITCH_DEL_PARAMS_PTR	pReq,
	FTM_RESP_SWITCH_DEL_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SWITCH_DEL_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_SWITCH_getCount
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_SWITCH_GET_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_SWITCH_GET_COUNT_PARAMS_PTR	pResp
)
{
	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SWITCH_GET_COUNT_PARAMS);
	pResp->xCommon.xRet	= FTM_CATCHB_getSwitchCount(pServer->pCatchB, &pResp->ulCount);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_SWITCH_getProperties
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_SWITCH_GET_PROPERTIES_PARAMS_PTR	pReq,
	FTM_RESP_SWITCH_GET_PROPERTIES_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SWITCH_GET_PROPERTIES_PARAMS);
	memcpy(&pResp->xConfig, &xConfig, sizeof(FTM_SWITCH_CONFIG));

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_SWITCH_setProperties
(
	FTM_SERVER_PTR	pServer,
	FTM_REQ_SWITCH_SET_PROPERTIES_PARAMS_PTR	pReq,
	FTM_RESP_SWITCH_SET_PROPERTIES_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SWITCH_SET_PROPERTIES_PARAMS);

	return	FTM_RET_OK;	
}

FTM_RET	FTM_SERVER_SWITCH_getIDList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SWITCH_GET_ID_LIST_PARAMS_PTR	pReq,
	FTM_RESP_SWITCH_GET_ID_LIST_PARAMS_PTR	pResp
)
{
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_SWITCH_GET_ID_LIST_PARAMS)) / sizeof(FTM_ID);
	if (ulMaxCount > pReq->ulCount)
	{
		ulMaxCount = pReq->ulCount;
	}

	INFO("GetSwitchIDList(%d)", ulMaxCount);

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = FTM_CATCHB_getSwitchIDList(pCatchB, ulMaxCount, pResp->pIDList,&pResp->ulCount);
	if (pResp->xCommon.xRet == FTM_RET_OK)
	{
		pResp->xCommon.ulLen = sizeof(FTM_RESP_SWITCH_GET_ID_LIST_PARAMS) + sizeof(FTM_ID) * pResp->ulCount;	
	}
	else
	{
		ERROR(xRet, "Failed to get switch ID list");
		pResp->xCommon.ulLen = sizeof(FTM_RESP_SWITCH_GET_ID_LIST_PARAMS);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_SWITCH_getList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SWITCH_GET_LIST_PARAMS_PTR	pReq,
	FTM_RESP_SWITCH_GET_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount = 0;
	FTM_SWITCH_PTR _PTR_	ppSwitchList;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_SWITCH_GET_LIST_PARAMS)) / sizeof(FTM_SWITCH_CONFIG);
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_SWITCH_GET_LIST_PARAMS) + sizeof(FTM_SWITCH_CONFIG) * ulCount;	
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_LOG_getList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_LOG_GET_LIST_PARAMS_PTR	pReq,
	FTM_RESP_LOG_GET_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount = 0;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_LOG_GET_LIST_PARAMS)) / sizeof(FTM_LOG);
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_LOG_GET_LIST_PARAMS) + sizeof(FTM_LOG) * ulCount;	

	INFO("GetLogList Response : %d", pResp->xCommon.ulLen);
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_LOG_getCount2
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_LOG_GET_COUNT2_PARAMS_PTR	pReq,
	FTM_RESP_LOG_GET_COUNT2_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;

	pCatchB = pServer->pCatchB;

	INFO("Start : %u, End : %u", pReq->ulBeginTime, pReq->ulEndTime);
	xRet = FTM_CATCHB_getLogCount2(pCatchB, pReq->xType, ((pReq->pID[0] == 0)?NULL:pReq->pID), ((pReq->pIP[0] == 0)?NULL:pReq->pIP), 
								pReq->xStat, pReq->ulBeginTime, pReq->ulEndTime, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log list!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_LOG_del
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_LOG_DEL_PARAMS_PTR	pReq,
	FTM_RESP_LOG_DEL_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_LOG_DEL_PARAMS);
	pResp->ulCount 		= ulCount;
	pResp->ulFirstTime	= ulFirstTime;
	pResp->ulLastTime	= ulLastTime;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_LOG_getCount
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_LOG_GET_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_LOG_GET_COUNT_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_LOG_GET_COUNT_PARAMS);
	pResp->ulCount = ulCount;

	return	xRet;
}

FTM_RET	FTM_SERVER_LOG_getInfo
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_LOG_GET_INFO_PARAMS_PTR	pReq,
	FTM_RESP_LOG_GET_INFO_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_LOG_GET_INFO_PARAMS);
	pResp->ulCount = ulCount;
	pResp->ulFirstTime= ulFirstTime;
	pResp->ulLastTime= ulLastTime;

	return	xRet;
}


FTM_RET	FTM_SERVER_ALARM_add
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ALARM_ADD_PARAMS_PTR	pReq,
	FTM_RESP_ALARM_ADD_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_ALARM_ADD_PARAMS);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_ALARM_del
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ALARM_DEL_PARAMS_PTR	pReq,
	FTM_RESP_ALARM_DEL_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_ALARM_DEL_PARAMS);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_ALARM_getCount
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ALARM_GET_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_ALARM_GET_COUNT_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_ALARM_GET_COUNT_PARAMS);
	pResp->ulCount = ulCount;

	return	xRet;
}

FTM_RET	FTM_SERVER_ALARM_get
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ALARM_GET_PARAMS_PTR	pReq,
	FTM_RESP_ALARM_GET_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_ALARM_GET_PARAMS);

	return	xRet;
}

FTM_RET	FTM_SERVER_ALARM_set
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ALARM_SET_PARAMS_PTR	pReq,
	FTM_RESP_ALARM_SET_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_ALARM_SET_PARAMS);

	return	xRet;
}

FTM_RET	FTM_SERVER_ALARM_getNameList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_ALARM_GET_NAME_LIST_PARAMS_PTR	pReq,
	FTM_RESP_ALARM_GET_NAME_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_ALARM_GET_NAME_LIST_PARAMS)) / sizeof(FTM_NAME);
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_ALARM_GET_NAME_LIST_PARAMS) + sizeof(FTM_NAME) * pResp->ulCount;;

	return	xRet;
}

FTM_RET	FTM_SERVER_SMTP_get
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SMTP_GET_PARAMS_PTR	pReq,
	FTM_RESP_SMTP_GET_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_getSMTP(pCatchB, &pResp->xSMTP);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm list!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SMTP_GET_PARAMS);

	return	xRet;
}

FTM_RET	FTM_SERVER_SMTP_set
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SMTP_SET_PARAMS_PTR	pReq,
	FTM_RESP_SMTP_SET_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;

	pCatchB = pServer->pCatchB;

	xRet = FTM_CATCHB_setSMTP(pCatchB, &pReq->xSMTP);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CATCHB_getSMTP(pCatchB, &pResp->xSMTP);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get alarm list!");
		}
		else
		{
			FTM_CONFIG	xConfig;

			memset(&xConfig, 0, sizeof(xConfig));

			FTM_CATCHB_getConfig(pCatchB, &xConfig);

			FTM_CONFIG_save(&xConfig, NULL);
		}
	}
	else
	{
		ERROR(xRet, "Failed to get alarm list!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SMTP_SET_PARAMS);

	return	xRet;
}

FTM_RET	FTM_SERVER_STAT_getList
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_STAT_GET_LIST_PARAMS_PTR	pReq,
	FTM_RESP_STAT_GET_LIST_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount = 0;

	pCatchB = pServer->pCatchB;

	ulMaxCount = (pResp->xCommon.ulLen - sizeof(FTM_RESP_STAT_GET_LIST_PARAMS)) / sizeof(FTM_STATISTICS);
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_STAT_GET_LIST_PARAMS) + sizeof(FTM_STATISTICS) * ulCount;	
	pResp->ulCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_STAT_del
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_STAT_DEL_PARAMS_PTR	pReq,
	FTM_RESP_STAT_DEL_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_STAT_DEL_PARAMS);
	pResp->ulCount 		= ulCount;
	pResp->ulFirstTime	= ulFirstTime;
	pResp->ulLastTime	= ulLastTime;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_STAT_del2
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_STAT_DEL2_PARAMS_PTR	pReq,
	FTM_RESP_STAT_DEL2_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen= sizeof(FTM_RESP_STAT_DEL2_PARAMS);
	pResp->ulCount 		= ulCount;
	pResp->ulFirstTime	= ulFirstTime;
	pResp->ulLastTime	= ulLastTime;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_STAT_getCount
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_STAT_GET_COUNT_PARAMS_PTR	pReq,
	FTM_RESP_STAT_GET_COUNT_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_STAT_GET_COUNT_PARAMS);
	pResp->ulCount = ulCount;

	return	xRet;
}

FTM_RET	FTM_SERVER_STAT_getInfo
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_STAT_GET_INFO_PARAMS_PTR	pReq,
	FTM_RESP_STAT_GET_INFO_PARAMS_PTR	pResp
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
	pResp->xCommon.ulLen = sizeof(FTM_RESP_STAT_GET_INFO_PARAMS);
	pResp->xInfo.ulFields =	FTM_SYSTEM_FIELD_STATISTICS_INTERVAL
						| 	FTM_SYSTEM_FIELD_STATISTICS_MAX_COUNT
						|	FTM_SYSTEM_FIELD_STATISTICS_COUNT	
						|	FTM_SYSTEM_FIELD_STATISTICS_FIRST_TIME
						|	FTM_SYSTEM_FIELD_STATISTICS_LAST_TIME;
	pResp->xInfo.xStatistics.ulMaxCount	= pCatchB->pConfig->xSystem.xStatistics.ulCount;
	pResp->xInfo.xStatistics.ulInterval = pCatchB->pConfig->xSystem.xStatistics.ulInterval;
	pResp->xInfo.xStatistics.ulCount 	= ulCount;
	pResp->xInfo.xStatistics.ulFirstTime= ulFirstTime;
	pResp->xInfo.xStatistics.ulLastTime	= ulLastTime;

	return	xRet;
}

FTM_RET	FTM_SERVER_STAT_setInfo
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_STAT_SET_INFO_PARAMS_PTR	pReq,
	FTM_RESP_STAT_SET_INFO_PARAMS_PTR	pResp
)
{
	FTM_RET			xRet;
	FTM_CATCHB_PTR	pCatchB;
	FTM_UINT32		ulCount = 0;
	FTM_UINT32		ulFirstTime= 0;
	FTM_UINT32		ulLastTime = 0;

	pCatchB = pServer->pCatchB;

	if (pReq->xInfo.ulFields & FTM_SYSTEM_FIELD_STATISTICS_INTERVAL)
	{
		FTM_CATCHB_setStatisticsUpdateInterval(pCatchB, pReq->xInfo.xStatistics.ulInterval);
	}

	if (pReq->xInfo.ulFields & FTM_SYSTEM_FIELD_STATISTICS_MAX_COUNT)
	{
		FTM_CATCHB_setStatisticsMaxCount(pCatchB, pReq->xInfo.xStatistics.ulMaxCount);
	}

	FTM_CONFIG	xConfig;

	xRet = FTM_CATCHB_getConfig(pCatchB, &xConfig);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_save(&xConfig, NULL);
		xRet = FTM_CATCHB_getStatisticsInfo(pCatchB, &ulCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get log info!");
		}
	}
	else
	{
		ERROR(xRet, "Failed to get configuration!");
	}

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet = xRet;
	pResp->xCommon.ulLen = sizeof(FTM_RESP_STAT_SET_INFO_PARAMS);
	pResp->xInfo.xStatistics.ulMaxCount	= pCatchB->pConfig->xSystem.xStatistics.ulCount;
	pResp->xInfo.xStatistics.ulInterval 	= pCatchB->pConfig->xSystem.xStatistics.ulInterval;
	pResp->xInfo.xStatistics.ulCount = ulCount;
	pResp->xInfo.xStatistics.ulFirstTime= ulFirstTime;
	pResp->xInfo.xStatistics.ulLastTime= ulLastTime;

	return	xRet;
}

FTM_RET	FTM_SERVER_SSID_create
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SSID_CREATE_PARAMS_PTR	pReq,
	FTM_RESP_SSID_CREATE_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN + 1];

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_SSID_create(pReq->pID, pReq->pPasswd, pSSID);

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SSID_CREATE_PARAMS);
	strncpy(pResp->pSSID, pSSID, FTM_SSID_LEN);

	return	xRet;
}

FTM_RET	FTM_SERVER_SSID_del
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SSID_DEL_PARAMS_PTR	pReq,
	FTM_RESP_SSID_DEL_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	xRet = FTM_SSID_destroy(pReq->xCommon.pSSID);

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SSID_DEL_PARAMS);

	return	xRet;
}

FTM_RET	FTM_SERVER_SSID_verify
(
	FTM_SERVER_PTR pServer, 
	FTM_REQ_SSID_VERIFY_PARAMS_PTR	pReq,
	FTM_RESP_SSID_VERIFY_PARAMS_PTR	pResp
)
{
	FTM_RET	xRet;

	xRet = FTM_SSID_isValid(pReq->xCommon.pSSID, pServer->xConfig.ulSSIDTimeout);

	pResp->xCommon.xCmd = pReq->xCommon.xCmd;
	pResp->xCommon.xRet	= xRet;
	pResp->xCommon.ulLen= sizeof(FTM_RESP_SSID_VERIFY_PARAMS);

	return	xRet;
}


