#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftm_types.h"
#include "ftm_client.h"
#include "ftm_lock.h"
#include "ftm_trace.h"
#include "ftm_mem.h"

#define	MAX_FRAME_SIZE	1500

typedef struct FTM_CLIENT_STRUCT
{
	FTM_CLIENT_CONFIG	xConfig;

	FTM_INT		hSock;
	FTM_INT		nTimeout;
	FTM_LOCK	xLock;
}	FTM_CLIENT, _PTR_ FTM_CLIENT_PTR;

static FTM_RET FTM_CLIENT_request
(
	FTM_CLIENT_PTR			pClient, 
	FTM_REQ_PARAMS_PTR		pReq,
	FTM_INT					nReqLen,
	FTM_RESP_PARAMS_PTR		pResp,
	FTM_INT					nRespLen
);

FTM_RET	FTM_CLIENT_CONFIG_setDefault
(
	FTM_CLIENT_CONFIG_PTR	pConfig
)
{
	memset(pConfig, 0, sizeof(FTM_CLIENT_CONFIG));

	strncpy(pConfig->pIP, "127.0.0.1", sizeof(pConfig->pIP));
	pConfig->usPort = 8800;
	pConfig->bAutoConnect = FTM_FALSE;

	return	FTM_RET_OK;
}


FTM_RET	FTM_CLIENT_CONFIG_load
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);
	
	FTM_RET	xRet = FTM_RET_OK;
	cJSON _PTR_ 	pItem;

	pItem = cJSON_GetObjectItem(pRoot, "ip");
	if (pItem != NULL)
	{
		strncpy(pConfig->pIP, pItem->valuestring, sizeof(pConfig->pIP));
	}

	pItem = cJSON_GetObjectItem(pRoot, "port");
	if (pItem != NULL)
	{
		pConfig->usPort = pItem->valueint;
	}

	pItem = cJSON_GetObjectItem(pRoot, "auto");
	if (pItem != NULL)
	{
		if (pItem->type == cJSON_String)
		{
			if ((strcasecmp(pItem->valuestring, "yes") == 0) ||(strcasecmp(pItem->valuestring, "on") == 0))
			{
				pConfig->bAutoConnect = FTM_TRUE;
			}
			else if ((strcasecmp(pItem->valuestring, "no") == 0) ||(strcasecmp(pItem->valuestring, "off") == 0))
			{
				pConfig->bAutoConnect = FTM_FALSE;
			}
		}
		else if (pItem->type == cJSON_Number)
		{
			pConfig->bAutoConnect = (pItem->valueint != 0);
		}
	}

	return	xRet;
}


/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTM_CLIENT_create
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	FTM_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);
	FTM_RET	xRet;
	FTM_CLIENT_PTR	pClient;

	pClient = (FTM_CLIENT_PTR)FTM_MEM_malloc(sizeof(FTM_CLIENT));
	if (pClient == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create FTM client!\n");
		return	xRet;
	}

	if(pConfig != NULL)
	{
		memcpy(&pClient->xConfig, pConfig, sizeof(FTM_CLIENT_CONFIG));		
	}
	else
	{
		FTM_CLIENT_CONFIG_setDefault(&pClient->xConfig);	
	}

	*ppClient = pClient;

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTM_CLIENT_destroy
(
	FTM_CLIENT_PTR _PTR_ ppClient
)
{
	ASSERT(ppClient != NULL);

	FTM_MEM_free(*ppClient);

	*ppClient = NULL;

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTM_CLIENT_setConfig
(
	FTM_CLIENT_PTR	pClient,
	FTM_CLIENT_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	if (ulFieldFlags & FTM_CLIENT_FIELD_IP)
	{
		strncpy(pClient->xConfig.pIP, pConfig->pIP, sizeof(pClient->xConfig.pIP));
	}

	if (ulFieldFlags & FTM_CLIENT_FIELD_PORT)
	{
		pClient->xConfig.usPort = pConfig->usPort;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CLIENT_getConfig
(
	FTM_CLIENT_PTR	pClient,
	FTM_CLIENT_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &pClient->xConfig, sizeof(FTM_CLIENT_CONFIG));

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTM_CLIENT_connect
(
	FTM_CLIENT_PTR 	pClient
)
{
	int 	hSock;
	struct sockaddr_in 	xServer;

	if ( pClient == NULL )
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	FTM_LOCK_init(&pClient->xLock);
	hSock = socket(AF_INET, SOCK_STREAM, 0);
	if (hSock == -1)
	{
		ERROR(FTM_RET_COMM_SOCK_ERROR, "Could not create socket.\n");	
		return	FTM_RET_ERROR;
	}

	inet_aton(pClient->xConfig.pIP, &xServer.sin_addr);
	xServer.sin_family 		= AF_INET;
	xServer.sin_port 		= htons(pClient->xConfig.usPort);

	if (connect(hSock, (struct sockaddr *)&xServer, sizeof(xServer)) < 0)
	{
		ERROR(FTM_RET_ERROR, "Failed to connect server!\n");
		return	FTM_RET_ERROR;	
	}
	
	pClient->hSock = hSock;
	pClient->nTimeout = 5000;

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTM_CLIENT_disconnect
(
 	FTM_CLIENT_PTR	pClient
)
{
	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	close(pClient->hSock);
	FTM_LOCK_final(&pClient->xLock);
	pClient->hSock = 0;
	
	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTM_CLIENT_isConnected
(
	FTM_CLIENT_PTR		pClient,
	FTM_BOOL_PTR			pbConnected
)
{
	if ((pClient != NULL) && (pClient->hSock != 0))
	{
		*pbConnected = FTM_TRUE;	
	}
	else
	{
		*pbConnected = FTM_FALSE;	
	}

	return	FTM_RET_OK;
}

/*****************************************************************
 * Internal Functions
 *****************************************************************/
FTM_RET FTM_CLIENT_request
(
	FTM_CLIENT_PTR 	pClient, 
	FTM_REQ_PARAMS_PTR	pReq,
	FTM_INT			nReqLen,
	FTM_RESP_PARAMS_PTR	pResp,
	FTM_INT			nRespLen
)
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nTimeout;


	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	FTM_LOCK_set(&pClient->xLock);

	INFO("send(%08lx, pReq, %d, 0)", pClient->hSock, nReqLen);

	if( send(pClient->hSock, pReq, nReqLen, 0) < 0)
	{
		xRet = FTM_RET_ERROR;	
	}
	else
	{
		nTimeout = pClient->nTimeout;
		while(--nTimeout > 0)
		{
			int	nLen = recv(pClient->hSock, pResp, nRespLen, MSG_DONTWAIT);
			INFO("recv(%08lx, pResp, %d, MSG_DONTWAIT)", pClient->hSock, nLen);
			if (nLen > 0)
			{
				break;
			}

			usleep(1000);
		}

		if (nTimeout == 0)
		{
			xRet = FTM_RET_CLIENT_TIMEOUT;	
		}
		else
		{
			xRet = pResp->xRet;	
		}

	}

	FTM_LOCK_reset(&pClient->xLock);

	return	xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTM_CLIENT_addCCTV
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pComment
)
{
	FTM_RET	xRet;
	FTM_REQ_ADD_CCTV_PARAMS		xReq;
	FTM_RESP_ADD_CCTV_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ADD_CCTV;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	if (pID != NULL)
	{
		strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);
	}

	if (pIP != NULL)
	{
		strncpy(xReq.pIP, pIP, sizeof(xReq.pIP) - 1);
	}

	if (pSwitchID != NULL)
	{
		strncpy(xReq.pSwitchID, pSwitchID, sizeof(xReq.pSwitchID) - 1);
	}

	if (pComment != NULL)
	{
		strncpy(xReq.pComment, pComment, sizeof(xReq.pComment) - 1);
	}

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_delCCTV
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID
)
{
	FTM_RET	xRet;
	FTM_REQ_DEL_CCTV_PARAMS		xReq;
	FTM_RESP_DEL_CCTV_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_DEL_CCTV;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_getCCTVCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_CCTV_COUNT_PARAMS		xReq;
	FTM_RESP_GET_CCTV_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_CCTV_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	*pCount = xResp.ulCount;

	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_setCCTVProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
)
{
	FTM_RET	xRet;
	FTM_REQ_SET_CCTV_PROPERTIES_PARAMS	xReq;
	FTM_RESP_SET_CCTV_PROPERTIES_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pID == NULL) || (pConfig == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SET_CCTV_PROPERTIES;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);
	xReq.ulFieldFlags = ulFieldFlags;
	memcpy(&xReq.xConfig, pConfig, sizeof(FTM_CCTV_CONFIG));

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_getCCTVProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_CCTV_PROPERTIES_PARAMS	xReq;
	FTM_RESP_GET_CCTV_PROPERTIES_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pID == NULL) || (pConfig == NULL))
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;	
		ERROR(xRet, "Failed to get CCTV properties. Invalid arguments");
		return	xRet;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_CCTV_PROPERTIES;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	memcpy(pConfig, &xResp.xConfig, sizeof(FTM_CCTV_CONFIG));

	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_getCCTVIDList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_CCTV_ID_LIST_PARAMS	xReq;
	FTM_RESP_GET_CCTV_ID_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pIDList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	ulRespLen = sizeof(FTM_RESP_GET_CCTV_ID_LIST_PARAMS) + sizeof(FTM_ID) * ulMaxCount;
	pResp = (FTM_RESP_GET_CCTV_ID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_CCTV_ID_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	xReq.ulCount 		=   ulMaxCount;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)pResp, ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pIDList, pResp->pIDList, sizeof(FTM_ID) * pResp->ulCount);
	}

	if(pResp != NULL)
	{
		FTM_MEM_free(pResp);	
	}

	return	xRet;
}


FTM_RET	FTM_CLIENT_getCCTVList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulMaxCount,
	FTM_CCTV_CONFIG_PTR	pCCTVList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_CCTV_LIST_PARAMS	xReq;
	FTM_RESP_GET_CCTV_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pCCTVList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if ( ulMaxCount > (MAX_FRAME_SIZE - sizeof(FTM_RESP_GET_CCTV_LIST_PARAMS)) / sizeof(FTM_CCTV_CONFIG))
	{
		ulMaxCount = (MAX_FRAME_SIZE - sizeof(FTM_RESP_GET_CCTV_LIST_PARAMS)) / sizeof(FTM_CCTV_CONFIG);
	}

	ulRespLen = sizeof(FTM_RESP_GET_CCTV_LIST_PARAMS) + sizeof(FTM_CCTV_CONFIG) * ulMaxCount;
	pResp = (FTM_RESP_GET_CCTV_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_CCTV_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	xReq.ulCount 		=   ulMaxCount;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)pResp, ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pCCTVList, pResp->pCCTVList, sizeof(FTM_CCTV_CONFIG) * pResp->ulCount);
	}

	if(pResp != NULL)
	{
		FTM_MEM_free(pResp);	
	}

	return	xRet;
}
/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTM_CLIENT_addSwitch
(
 	FTM_CLIENT_PTR	pClient,
	FTM_SWITCH_CONFIG_PTR	pConfig
)
{
	FTM_RET	xRet;
	FTM_REQ_ADD_SWITCH_PARAMS		xReq;
	FTM_RESP_ADD_SWITCH_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ADD_SWITCH;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	memcpy(&xReq.xConfig, pConfig, sizeof(FTM_SWITCH_CONFIG));

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_delSwitch
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID
)
{
	FTM_RET	xRet;
	FTM_REQ_DEL_SWITCH_PARAMS		xReq;
	FTM_RESP_DEL_SWITCH_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_DEL_SWITCH;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_getSwitchCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_SWITCH_COUNT_PARAMS		xReq;
	FTM_RESP_GET_SWITCH_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_SWITCH_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	*pCount = xResp.ulCount;

	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_setSwitchProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
)
{
	FTM_RET	xRet;
	FTM_REQ_SET_SWITCH_PROPERTIES_PARAMS	xReq;
	FTM_RESP_SET_SWITCH_PROPERTIES_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pID == NULL) || (pConfig == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SET_SWITCH_PROPERTIES;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);
	xReq.ulFieldFlags = ulFieldFlags;
	memcpy(&xReq.xConfig, pConfig, sizeof(FTM_SWITCH_CONFIG));

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_getSwitchProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_SWITCH_PROPERTIES_PARAMS	xReq;
	FTM_RESP_GET_SWITCH_PROPERTIES_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pID == NULL) || (pConfig == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_SWITCH_PROPERTIES;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	memcpy(pConfig, &xResp.xConfig, sizeof(FTM_SWITCH_CONFIG));

	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_getSwitchIDList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_SWITCH_ID_LIST_PARAMS	xReq;
	FTM_RESP_GET_SWITCH_ID_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pIDList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	ulRespLen = sizeof(FTM_RESP_GET_SWITCH_ID_LIST_PARAMS) + sizeof(FTM_ID) * ulMaxCount;
	pResp = (FTM_RESP_GET_SWITCH_ID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_SWITCH_ID_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	xReq.ulCount 		=   ulMaxCount;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)pResp, ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pIDList, pResp->pIDList, sizeof(FTM_ID) * pResp->ulCount);
	}

	if(pResp != NULL)
	{
		FTM_MEM_free(pResp);	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_getSwitchList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulMaxCount,
	FTM_SWITCH_CONFIG_PTR	pSwitchList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_SWITCH_LIST_PARAMS	xReq;
	FTM_RESP_GET_SWITCH_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pSwitchList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if ( ulMaxCount > (MAX_FRAME_SIZE - sizeof(FTM_RESP_GET_SWITCH_LIST_PARAMS)) / sizeof(FTM_SWITCH_CONFIG))
	{
		ulMaxCount = (MAX_FRAME_SIZE - sizeof(FTM_RESP_GET_SWITCH_LIST_PARAMS)) / sizeof(FTM_SWITCH_CONFIG);
	}

	ulRespLen = sizeof(FTM_RESP_GET_SWITCH_LIST_PARAMS) + sizeof(FTM_SWITCH_CONFIG) * ulMaxCount;
	pResp = (FTM_RESP_GET_SWITCH_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_SWITCH_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	xReq.ulCount 		=   ulMaxCount;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)pResp, ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		*pulCount = pResp->ulCount;
		memcpy(pSwitchList, pResp->pSwitchList, sizeof(FTM_SWITCH_CONFIG) * pResp->ulCount);
	}

	if(pResp != NULL)
	{
		FTM_MEM_free(pResp);	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_getLogInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pulCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_LOG_INFO_PARAMS	xReq;
	FTM_RESP_GET_LOG_INFO_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_LOG_INFO;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;	
		*pulFirstTime = xResp.ulFirstTime;	
		*pulLastTime = xResp.ulLastTime;	
	}

	return	xRet;
}
FTM_RET	FTM_CLIENT_getLogCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_LOG_COUNT_PARAMS	xReq;
	FTM_RESP_GET_LOG_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_LOG_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_delLog
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_DEL_LOG_PARAMS	xReq;
	FTM_RESP_DEL_LOG_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_DEL_LOG;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	xReq.ulIndex		=	ulIndex;
	xReq.ulCount 		=   ulCount;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulRemainCount = xResp.ulCount;
		*pulFirstTime = xResp.ulFirstTime;
		*pulLastTime = xResp.ulLastTime;
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_getLogList
(
	FTM_CLIENT_PTR	pClient,
	FTM_LOG_TYPE	xType,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulBeginTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_LOG_PTR		pLogList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_LOG_LIST_PARAMS	xReq;
	FTM_RESP_GET_LOG_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;
	FTM_UINT32	ulAllowedNumber = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pLogList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	ulAllowedNumber = (FTM_PARAM_MAX_LEN - sizeof(FTM_RESP_GET_LOG_LIST_PARAMS)) / sizeof(FTM_LOG);
	
	if(ulAllowedNumber >= ulMaxCount)
	{
		ulRespLen = sizeof(FTM_RESP_GET_LOG_LIST_PARAMS) + sizeof(FTM_LOG) * ulMaxCount;
		pResp = (FTM_RESP_GET_LOG_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
		if (pResp == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(&xReq, 0, sizeof(xReq));

		xReq.xCommon.xCmd	=	FTM_CMD_GET_LOG_LIST;
		xReq.xCommon.ulLen	=	sizeof(xReq);
		xReq.xType			= 	xType;
		if (pID != NULL)
		{
			strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);
		}
		if (pIP != NULL)
		{
			strncpy(xReq.pIP, pIP, sizeof(xReq.pIP) - 1);
		}
		xReq.xStat			=	xStat;
		xReq.ulBeginTime	=	ulBeginTime;
		xReq.ulEndTime		=	ulEndTime;
		xReq.ulIndex		=	ulIndex;
		xReq.ulCount 		=   ulMaxCount;

		xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)pResp, ulRespLen);
		if (xRet == FTM_RET_OK)
		{
			*pulCount = pResp->ulCount;
			memcpy(pLogList, pResp->pLogList, sizeof(FTM_LOG) * pResp->ulCount);
		}

		if(pResp != NULL)
		{
			FTM_MEM_free(pResp);	
		}
	}
	else
	{
		FTM_UINT32	ulTotalCount = 0;

		while(ulMaxCount > 0)
		{
			FTM_UINT32	ulReqCount;
			FTM_UINT32	ulRespCount = 0;

			ulReqCount = (ulAllowedNumber > ulMaxCount)?ulMaxCount:ulAllowedNumber;

			xRet = FTM_CLIENT_getLogList(pClient, xType, pID, pIP, xStat, ulBeginTime, ulEndTime, ulIndex, ulReqCount, &pLogList[ulTotalCount], &ulRespCount);
			if (xRet != FTM_RET_OK)
			{
				break;	
			}

			ulIndex += ulRespCount;
			ulTotalCount += ulRespCount;
			ulMaxCount -= ulRespCount;

			if (ulRespCount == 0)
			{
				break;
			}
		}

		if (xRet == FTM_RET_OK)
		{
			*pulCount = ulTotalCount;	
		}
	}

	return	xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTM_CLIENT_addAlarm
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pEmail,
	FTM_CHAR_PTR	pMessage
)
{
	FTM_RET	xRet;
	FTM_REQ_ADD_ALARM_PARAMS		xReq;
	FTM_RESP_ADD_ALARM_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ADD_ALARM;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	if ((pName == NULL) || (pEmail == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	strncpy(xReq.pName, pName, sizeof(xReq.pName) - 1);
	strncpy(xReq.pEmail, pEmail, sizeof(xReq.pEmail) - 1);

	if (pMessage != NULL)
	{
		strncpy(xReq.pMessage, pMessage, sizeof(xReq.pMessage) - 1);
	}

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_delAlarm
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pName
)
{
	FTM_RET	xRet;
	FTM_REQ_DEL_ALARM_PARAMS	xReq;
	FTM_RESP_DEL_ALARM_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pName == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_DEL_ALARM;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	strncpy(xReq.pName, pName, sizeof(xReq.pName) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_getAlarmCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_ALARM_COUNT_PARAMS	xReq;
	FTM_RESP_GET_ALARM_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_ALARM_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	*pCount = xResp.ulCount;

	return	xResp.xCommon.xRet;
}

FTM_RET FTM_CLIENT_getAlarm
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_ALARM_PARAMS	xReq;
	FTM_RESP_GET_ALARM_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_ALARM;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.pName, pName, sizeof(xReq.pName) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	memcpy(pAlarm, &xResp.xAlarm, sizeof(FTM_ALARM));

	return	xResp.xCommon.xRet;
}

FTM_RET FTM_CLIENT_setAlarm
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm,
	FTM_UINT32		ulFieldFlags
)
{
	FTM_RET	xRet;
	FTM_REQ_SET_ALARM_PARAMS		xReq;
	FTM_RESP_SET_ALARM_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SET_ALARM;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.pName, pName, sizeof(xReq.pName) - 1);
	memcpy(&xReq.xAlarm, pAlarm, sizeof(FTM_ALARM));
	xReq.ulFieldFlags   = ulFieldFlags;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	return	xResp.xCommon.xRet;
}

FTM_RET FTM_CLIENT_getAlarmNameList
(
 	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_NAME_PTR	pNameList,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_ALARM_NAME_LIST_PARAMS	xReq;
	FTM_RESP_GET_ALARM_NAME_LIST_PARAMS_PTR	pResp;
	FTM_UINT32						ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	ulRespLen = sizeof(FTM_RESP_GET_ALARM_NAME_LIST_PARAMS)	+ sizeof(FTM_NAME)*ulMaxCount;
	pResp = (FTM_RESP_GET_ALARM_NAME_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_ALARM_NAME_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	xReq.ulIndex		=   ulIndex;
	xReq.ulCount		=   ulMaxCount;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)pResp, ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pNameList, pResp->pNameList, sizeof(FTM_NAME)*(pResp->ulCount));
		*pCount = pResp->ulCount;
	}

	FTM_MEM_free(pResp);

	return	xRet;
}

FTM_RET	FTM_CLIENT_getStatInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pulCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_STAT_INFO_PARAMS	xReq;
	FTM_RESP_GET_STAT_INFO_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_STAT_INFO;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;	
		*pulFirstTime = xResp.ulFirstTime;	
		*pulLastTime = xResp.ulLastTime;	
	}

	return	xRet;
}
FTM_RET	FTM_CLIENT_getStatCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_STAT_COUNT_PARAMS	xReq;
	FTM_RESP_GET_STAT_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_GET_STAT_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_delStat
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_DEL_STAT_PARAMS	xReq;
	FTM_RESP_DEL_STAT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_DEL_STAT;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	xReq.ulIndex		=	ulIndex;
	xReq.ulCount 		=   ulCount;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulRemainCount = xResp.ulCount;
		*pulFirstTime = xResp.ulFirstTime;
		*pulLastTime = xResp.ulLastTime;
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_delStat2
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulBeginTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_DEL_STAT2_PARAMS	xReq;
	FTM_RESP_DEL_STAT2_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_DEL_STAT2;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	xReq.ulBeginTime	=	ulBeginTime;
	xReq.ulEndTime		=   ulEndTime;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulRemainCount = xResp.ulCount;
		*pulFirstTime = xResp.ulFirstTime;
		*pulLastTime = xResp.ulLastTime;
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_getStatList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pStatList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_GET_STAT_LIST_PARAMS	xReq;
	FTM_RESP_GET_STAT_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;
	FTM_UINT32	ulAllowedNumber = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pStatList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	ulAllowedNumber = (FTM_PARAM_MAX_LEN - sizeof(FTM_RESP_GET_STAT_LIST_PARAMS)) / sizeof(FTM_STATISTICS);
	
	if(ulAllowedNumber >= ulMaxCount)
	{
		ulRespLen = sizeof(FTM_RESP_GET_STAT_LIST_PARAMS) + sizeof(FTM_STATISTICS) * ulMaxCount;
		pResp = (FTM_RESP_GET_STAT_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
		if (pResp == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(&xReq, 0, sizeof(xReq));

		xReq.xCommon.xCmd	=	FTM_CMD_GET_STAT_LIST;
		xReq.xCommon.ulLen	=	sizeof(xReq);
		xReq.ulIndex		=	ulIndex;
		xReq.ulCount 		=   ulMaxCount;

		xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)pResp, ulRespLen);
		if (xRet == FTM_RET_OK)
		{
			*pulCount = pResp->ulCount;
			memcpy(pStatList, pResp->pStatList, sizeof(FTM_STATISTICS) * pResp->ulCount);
		}

		if(pResp != NULL)
		{
			FTM_MEM_free(pResp);	
		}
	}
	else
	{
		FTM_UINT32	ulTotalCount = 0;

		while(ulMaxCount > 0)
		{
			FTM_UINT32	ulReqCount;
			FTM_UINT32	ulRespCount = 0;

			ulReqCount = (ulAllowedNumber > ulMaxCount)?ulMaxCount:ulAllowedNumber;

			xRet = FTM_CLIENT_getStatList(pClient, ulIndex, ulReqCount, &pStatList[ulTotalCount], &ulRespCount);
			if (xRet != FTM_RET_OK)
			{
				break;	
			}

			ulTotalCount += ulRespCount;
			ulMaxCount -= ulRespCount;
		}

		if (xRet == FTM_RET_OK)
		{
			*pulCount = ulTotalCount;	
		}
	}

	return	xRet;
}
