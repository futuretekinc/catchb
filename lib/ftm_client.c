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

#undef	__MODULE__
#define	__MODULE__	"client"

#define	MAX_FRAME_SIZE					(FTM_PARAM_MAX_LEN)	

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
	strncpy(pConfig->pTmpPath, FTM_CATCHB_CLIENT_TMP_PATH, sizeof(pConfig->pTmpPath) - 1);

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

	pItem = cJSON_GetObjectItem(pRoot, "tmp_path");
	if (pItem != NULL)
	{
		strncpy(pConfig->pTmpPath, pItem->valuestring, sizeof(pConfig->pTmpPath) - 1);
	}
	return	xRet;
}

FTM_RET	FTM_CLIENT_CONFIG_loadFromFile
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	FILE *pFile; 
	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR_PTR	pData = NULL;
	FTM_UINT32	ulFileLen;
	FTM_UINT32	ulReadSize;
	cJSON _PTR_		pRoot = NULL;

	pFile = fopen(pFileName, "rt");
	if (pFile == NULL)
	{         
		xRet = FTM_RET_CONFIG_LOAD_FAILED; 
		ERROR(xRet, "Can't open file[%s]\n", pFileName);
		return  xRet; 
	}    

	fseek(pFile, 0L, SEEK_END);
	ulFileLen = ftell(pFile);
	fseek(pFile, 0L, SEEK_SET);

	if (ulFileLen > 0)
	{
		pData = (FTM_CHAR_PTR)FTM_MEM_malloc(ulFileLen);
		if (pData != NULL)
		{
			memset(pData, 0, ulFileLen);
			ulReadSize = fread(pData, 1, ulFileLen, pFile); 
			if (ulReadSize != ulFileLen)
			{    
				xRet = FTM_RET_FAILED_TO_READ_FILE;
				ERROR(xRet, "Failed to read configuration file[%u:%u]\n", ulFileLen, ulReadSize);
				goto finished;
			}    
		}
		else
		{    
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;  
			ERROR(xRet, "Failed to alloc buffer[size = %u]\n", ulFileLen);
			goto finished;
		}    

	}
	fclose(pFile);
	pFile = NULL;

	pRoot = cJSON_Parse(pData);
	if (pRoot == NULL)
	{    
		xRet = FTM_RET_INVALID_JSON_FORMAT;
		ERROR(xRet, "Invalid json format!\n");
		goto finished;
	}    

	xRet = FTM_CLIENT_CONFIG_load(pConfig, pRoot);

finished:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}

	if (pData != NULL)
	{
		FTM_MEM_free(pData);
		pData = NULL;
	}

	if (pFile != NULL)
	{
		fclose(pFile);	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_CONFIG_save
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
)
{
	cJSON_AddStringToObject(pRoot, "ip", 	pConfig->pIP);
	cJSON_AddNumberToObject(pRoot, "port", 	pConfig->usPort);
	cJSON_AddNumberToObject(pRoot, "auto", 	pConfig->bAutoConnect);
	cJSON_AddStringToObject(pRoot, "tmp_path", 	pConfig->pTmpPath);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CLIENT_CONFIG_show
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
)
{
	printf("%16s : %s\n", "ip", 	pConfig->pIP);
	printf("%16s : %d\n", "port", 	pConfig->usPort);
	printf("%16s : %s\n", "auto", 	(pConfig->bAutoConnect)?"on":"off");
	printf("%16s : %s\n", "Tmp Path",pConfig->pTmpPath);

	return	FTM_RET_OK;
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
		xRet = FTM_RET_CLIENT_HANDLE_INVALID;	
		ERROR(xRet, "Failed to request!");
		return	xRet;
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
FTM_RET FTM_CLIENT_CCTV_add
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pComment
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_ADD_PARAMS		xReq;
	FTM_RESP_CCTV_ADD_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_ADD;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

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

FTM_RET	FTM_CLIENT_CCTV_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_DEL_PARAMS		xReq;
	FTM_RESP_CCTV_DEL_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_DEL;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_CCTV_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_GET_COUNT_PARAMS		xReq;
	FTM_RESP_CCTV_GET_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_GET_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	*pCount = xResp.ulCount;

	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_CCTV_setProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_SET_PROPERTIES_PARAMS	xReq;
	FTM_RESP_CCTV_SET_PROPERTIES_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pID == NULL) || (pConfig == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_SET_PROPERTIES;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

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

FTM_RET	FTM_CLIENT_CCTV_getProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_GET_PROPERTIES_PARAMS	xReq;
	FTM_RESP_CCTV_GET_PROPERTIES_PARAMS	xResp;

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

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_GET_PROPERTIES;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	memcpy(pConfig, &xResp.xConfig, sizeof(FTM_CCTV_CONFIG));

	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_CCTV_getIDList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_GET_ID_LIST_PARAMS	xReq;
	FTM_RESP_CCTV_GET_ID_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pIDList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	ulRespLen = sizeof(FTM_RESP_CCTV_GET_ID_LIST_PARAMS) + sizeof(FTM_ID) * ulMaxCount;
	pResp = (FTM_RESP_CCTV_GET_ID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_GET_ID_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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


FTM_RET	FTM_CLIENT_CCTV_getList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_CCTV_CONFIG_PTR	pCCTVList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_GET_LIST_PARAMS	xReq;
	FTM_RESP_CCTV_GET_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pCCTVList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if ( ulMaxCount > (MAX_FRAME_SIZE - sizeof(FTM_RESP_CCTV_GET_LIST_PARAMS)) / sizeof(FTM_CCTV_CONFIG))
	{
		ulMaxCount = (MAX_FRAME_SIZE - sizeof(FTM_RESP_CCTV_GET_LIST_PARAMS)) / sizeof(FTM_CCTV_CONFIG);
	}

	ulRespLen = sizeof(FTM_RESP_CCTV_GET_LIST_PARAMS) + sizeof(FTM_CCTV_CONFIG) * ulMaxCount;
	pResp = (FTM_RESP_CCTV_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_GET_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

FTM_RET	FTM_CLIENT_CCTV_setPolicy
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_AC_POLICY	xPolicy
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_SET_POLICY_PARAMS	xReq;
	FTM_RESP_CCTV_SET_POLICY_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_SET_POLICY;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);
	xReq.xPolicy = xPolicy;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_CCTV_reset
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
)
{
	FTM_RET	xRet;
	FTM_REQ_CCTV_RESET_PARAMS		xReq;
	FTM_RESP_CCTV_RESET_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_CCTV_RESET;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);
	strncpy(xReq.pHash, pHash, sizeof(xReq.pHash) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET FTM_CLIENT_SWITCH_add
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_SWITCH_CONFIG_PTR	pConfig
)
{
	FTM_RET	xRet;
	FTM_REQ_SWITCH_ADD_PARAMS		xReq;
	FTM_RESP_SWITCH_ADD_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SWITCH_ADD;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
	memcpy(&xReq.xConfig, pConfig, sizeof(FTM_SWITCH_CONFIG));

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_SWITCH_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID
)
{
	FTM_RET	xRet;
	FTM_REQ_SWITCH_DEL_PARAMS		xReq;
	FTM_RESP_SWITCH_DEL_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pID == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SWITCH_DEL;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_SWITCH_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET	xRet;
	FTM_REQ_SWITCH_GET_COUNT_PARAMS		xReq;
	FTM_RESP_SWITCH_GET_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SWITCH_GET_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	*pCount = xResp.ulCount;

	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_SWITCH_setProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
)
{
	FTM_RET	xRet;
	FTM_REQ_SWITCH_SET_PROPERTIES_PARAMS	xReq;
	FTM_RESP_SWITCH_SET_PROPERTIES_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pID == NULL) || (pConfig == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SWITCH_SET_PROPERTIES;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

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

FTM_RET	FTM_CLIENT_SWITCH_getProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig
)
{
	FTM_RET	xRet;
	FTM_REQ_SWITCH_GET_PROPERTIES_PARAMS	xReq;
	FTM_RESP_SWITCH_GET_PROPERTIES_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pID == NULL) || (pConfig == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SWITCH_GET_PROPERTIES;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	memcpy(pConfig, &xResp.xConfig, sizeof(FTM_SWITCH_CONFIG));

	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_SWITCH_getIDList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_SWITCH_GET_ID_LIST_PARAMS	xReq;
	FTM_RESP_SWITCH_GET_ID_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pIDList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	ulRespLen = sizeof(FTM_RESP_SWITCH_GET_ID_LIST_PARAMS) + sizeof(FTM_ID) * ulMaxCount;
	pResp = (FTM_RESP_SWITCH_GET_ID_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SWITCH_GET_ID_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

FTM_RET	FTM_CLIENT_SWITCH_getList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_SWITCH_CONFIG_PTR	pSwitchList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_SWITCH_GET_LIST_PARAMS	xReq;
	FTM_RESP_SWITCH_GET_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pSwitchList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if ( ulMaxCount > (MAX_FRAME_SIZE - sizeof(FTM_RESP_SWITCH_GET_LIST_PARAMS)) / sizeof(FTM_SWITCH_CONFIG))
	{
		ulMaxCount = (MAX_FRAME_SIZE - sizeof(FTM_RESP_SWITCH_GET_LIST_PARAMS)) / sizeof(FTM_SWITCH_CONFIG);
	}

	ulRespLen = sizeof(FTM_RESP_SWITCH_GET_LIST_PARAMS) + sizeof(FTM_SWITCH_CONFIG) * ulMaxCount;
	pResp = (FTM_RESP_SWITCH_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SWITCH_GET_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

FTM_RET	FTM_CLIENT_SWITCH_getModelList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_SWITCH_MODEL_INFO_PTR	pModelList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_SWITCH_GET_MODEL_LIST_PARAMS	xReq;
	FTM_RESP_SWITCH_GET_MODEL_LIST_PARAMS_PTR	pResp;
	FTM_UINT32	ulRespLen = 0;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if ((pModelList == NULL) || (pulCount == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if ( ulMaxCount > (MAX_FRAME_SIZE - sizeof(FTM_RESP_SWITCH_GET_MODEL_LIST_PARAMS)) / sizeof(FTM_SWITCH_MODEL_INFO))
	{
		ulMaxCount = (MAX_FRAME_SIZE - sizeof(FTM_RESP_SWITCH_GET_MODEL_LIST_PARAMS)) / sizeof(FTM_SWITCH_MODEL_INFO);
	}

	ulRespLen = sizeof(FTM_RESP_SWITCH_GET_MODEL_LIST_PARAMS) + sizeof(FTM_SWITCH_MODEL_INFO) * ulMaxCount;
	pResp = (FTM_RESP_SWITCH_GET_MODEL_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SWITCH_GET_MODEL_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
	xReq.ulIndex		=	0;
	xReq.ulCount 		=   ulMaxCount;

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)pResp, ulRespLen);
	if (xRet == FTM_RET_OK)
	{
		if (pResp->ulCount < ulMaxCount)
		{
			*pulCount = pResp->ulCount;
			memcpy(pModelList, pResp->pModelList, sizeof(FTM_SWITCH_MODEL_INFO) * pResp->ulCount);
		}
		else
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	if(pResp != NULL)
	{
		FTM_MEM_free(pResp);	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_LOG_getInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pulCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_LOG_GET_INFO_PARAMS	xReq;
	FTM_RESP_LOG_GET_INFO_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_LOG_GET_INFO;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;	
		*pulFirstTime = xResp.ulFirstTime;	
		*pulLastTime = xResp.ulLastTime;	
	}

	return	xRet;
}
FTM_RET	FTM_CLIENT_LOG_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_LOG_GET_COUNT_PARAMS	xReq;
	FTM_RESP_LOG_GET_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_LOG_GET_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_LOG_getCount2
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_LOG_TYPE	xType,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulBeginTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_LOG_GET_COUNT2_PARAMS	xReq;
	FTM_RESP_LOG_GET_COUNT2_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pulCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	INFO("Begin : %u, End : %u", ulBeginTime, ulEndTime);
	
	memset(&xReq, 0, sizeof(xReq));
	memset(&xResp, 0, sizeof(xResp));

	xReq.xCommon.xCmd	=	FTM_CMD_LOG_GET_COUNT2;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;
	}
	else
	{
		ERROR(xRet, "Failed to get log count!");	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_LOG_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_LOG_DEL_PARAMS	xReq;
	FTM_RESP_LOG_DEL_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_LOG_DEL;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

FTM_RET	FTM_CLIENT_LOG_getList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
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
	FTM_REQ_LOG_GET_LIST_PARAMS	xReq;
	FTM_RESP_LOG_GET_LIST_PARAMS_PTR	pResp;
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

	INFO("Begin : %u, End : %u", ulBeginTime, ulEndTime);
	ulAllowedNumber = (FTM_PARAM_MAX_LEN - sizeof(FTM_RESP_LOG_GET_LIST_PARAMS)) / sizeof(FTM_LOG);
	
	if(ulAllowedNumber >= ulMaxCount)
	{
		ulRespLen = sizeof(FTM_RESP_LOG_GET_LIST_PARAMS) + sizeof(FTM_LOG) * ulMaxCount;
		pResp = (FTM_RESP_LOG_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
		if (pResp == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(&xReq, 0, sizeof(xReq));

		xReq.xCommon.xCmd	=	FTM_CMD_LOG_GET_LIST;
		xReq.xCommon.ulLen	=	sizeof(xReq);
		strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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
		else
		{
			ERROR(xRet, "Failed to get log!");	
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

			xRet = FTM_CLIENT_LOG_getList(pClient, pSSID, xType, pID, pIP, xStat, ulBeginTime, ulEndTime, ulIndex, ulReqCount, &pLogList[ulTotalCount], &ulRespCount);
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
FTM_RET FTM_CLIENT_ALARM_add
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pEmail,
	FTM_CHAR_PTR	pMessage
)
{
	FTM_RET	xRet;
	FTM_REQ_ALARM_ADD_PARAMS		xReq;
	FTM_RESP_ALARM_ADD_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ALARM_ADD;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

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

FTM_RET	FTM_CLIENT_ALARM_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pName
)
{
	FTM_RET	xRet;
	FTM_REQ_ALARM_DEL_PARAMS	xReq;
	FTM_RESP_ALARM_DEL_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pName == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ALARM_DEL;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	strncpy(xReq.pName, pName, sizeof(xReq.pName) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	return	xResp.xCommon.xRet;
}

FTM_RET	FTM_CLIENT_ALARM_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET	xRet;
	FTM_REQ_ALARM_GET_COUNT_PARAMS	xReq;
	FTM_RESP_ALARM_GET_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	if (pCount == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ALARM_GET_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	*pCount = xResp.ulCount;

	return	xResp.xCommon.xRet;
}

FTM_RET FTM_CLIENT_ALARM_get
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm
)
{
	FTM_RET	xRet;
	FTM_REQ_ALARM_GET_PARAMS	xReq;
	FTM_RESP_ALARM_GET_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ALARM_GET;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
	strncpy(xReq.pName, pName, sizeof(xReq.pName) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	memcpy(pAlarm, &xResp.xAlarm, sizeof(FTM_ALARM));

	return	xResp.xCommon.xRet;
}

FTM_RET FTM_CLIENT_ALARM_set
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm,
	FTM_UINT32		ulFieldFlags
)
{
	FTM_RET	xRet;
	FTM_REQ_ALARM_SET_PARAMS		xReq;
	FTM_RESP_ALARM_SET_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ALARM_SET;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

FTM_RET FTM_CLIENT_ALARM_getNameList
(
 	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_NAME_PTR	pNameList,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET	xRet;
	FTM_REQ_ALARM_GET_NAME_LIST_PARAMS	xReq;
	FTM_RESP_ALARM_GET_NAME_LIST_PARAMS_PTR	pResp;
	FTM_UINT32						ulRespLen;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	ulRespLen = sizeof(FTM_RESP_ALARM_GET_NAME_LIST_PARAMS)	+ sizeof(FTM_NAME)*ulMaxCount;
	pResp = (FTM_RESP_ALARM_GET_NAME_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
	if (pResp == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_ALARM_GET_NAME_LIST;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

FTM_RET	FTM_CLIENT_SMTP_get
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig
)
{
	FTM_RET	xRet;
	FTM_REQ_SMTP_GET_PARAMS		xReq;
	FTM_RESP_SMTP_GET_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SMTP_GET;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		memcpy(pConfig, &xResp.xSMTP, sizeof(FTM_NOTIFIER_SMTP_CONFIG));
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_SMTP_set
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig,
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pResultConfig
)
{
	FTM_RET	xRet;
	FTM_REQ_SMTP_SET_PARAMS		xReq;
	FTM_RESP_SMTP_SET_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SMTP_SET;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
	memcpy(&xReq.xSMTP, pConfig, sizeof(FTM_NOTIFIER_SMTP_CONFIG));

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if ((xRet == FTM_RET_OK) && (pResultConfig != NULL))
	{
		memcpy(pResultConfig, &xResp.xSMTP, sizeof(FTM_NOTIFIER_SMTP_CONFIG));
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_STAT_setInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_SYSTEM_INFO_PTR	pRequestInfo,
	FTM_SYSTEM_INFO_PTR	pResultInfo
)
{
	FTM_RET	xRet;
	FTM_REQ_STAT_SET_INFO_PARAMS	xReq;
	FTM_RESP_STAT_SET_INFO_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_STAT_SET_INFO;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
	memcpy(&xReq.xInfo, pRequestInfo, sizeof(FTM_SYSTEM_INFO));

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if ((xRet == FTM_RET_OK) && (pResultInfo != NULL))
	{
		memcpy(pResultInfo, &xResp.xInfo, sizeof(FTM_SYSTEM_INFO));
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_STAT_getInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_SYSTEM_INFO_PTR	pInfo
)
{
	FTM_RET	xRet;
	FTM_REQ_STAT_GET_INFO_PARAMS	xReq;
	FTM_RESP_STAT_GET_INFO_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_STAT_GET_INFO;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &xResp.xInfo, sizeof(FTM_SYSTEM_INFO));
	}

	return	xRet;
}
FTM_RET	FTM_CLIENT_STAT_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_STAT_GET_COUNT_PARAMS	xReq;
	FTM_RESP_STAT_GET_COUNT_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_STAT_GET_COUNT;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		*pulCount = xResp.ulCount;	
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_STAT_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_STAT_DEL_PARAMS	xReq;
	FTM_RESP_STAT_DEL_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_STAT_DEL;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

FTM_RET	FTM_CLIENT_STAT_del2
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulBeginTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
)
{
	FTM_RET	xRet;
	FTM_REQ_STAT_DEL2_PARAMS	xReq;
	FTM_RESP_STAT_DEL2_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_STAT_DEL2;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

FTM_RET	FTM_CLIENT_STAT_getList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pStatList,
	FTM_UINT32_PTR	pulCount
)
{
	FTM_RET	xRet;
	FTM_REQ_STAT_GET_LIST_PARAMS	xReq;
	FTM_RESP_STAT_GET_LIST_PARAMS_PTR	pResp;
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
	
	ulAllowedNumber = (FTM_PARAM_MAX_LEN - sizeof(FTM_RESP_STAT_GET_LIST_PARAMS)) / sizeof(FTM_STATISTICS);
	
	if(ulAllowedNumber >= ulMaxCount)
	{
		ulRespLen = sizeof(FTM_RESP_STAT_GET_LIST_PARAMS) + sizeof(FTM_STATISTICS) * ulMaxCount;
		pResp = (FTM_RESP_STAT_GET_LIST_PARAMS_PTR)FTM_MEM_malloc(ulRespLen);
		if (pResp == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(&xReq, 0, sizeof(xReq));

		xReq.xCommon.xCmd	=	FTM_CMD_STAT_GET_LIST;
		xReq.xCommon.ulLen	=	sizeof(xReq);
		strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);
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

			xRet = FTM_CLIENT_STAT_getList(pClient, pSSID, ulIndex, ulReqCount, &pStatList[ulTotalCount], &ulRespCount);
			if (xRet != FTM_RET_OK)
			{
				break;	
			}

			ulIndex += ulRespCount;
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

FTM_RET	FTM_CLIENT_SSID_create
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_UINT32		ulIDLen,
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulSSIDLen
)
{
	FTM_RET	xRet;
	FTM_REQ_SSID_CREATE_PARAMS	xReq;
	FTM_RESP_SSID_CREATE_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SSID_CREATE;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.pID, pID, sizeof(xReq.pID) - 1);
	strncpy(xReq.pPasswd, pPasswd, sizeof(xReq.pPasswd) - 1);

	xRet = FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
	if (xRet == FTM_RET_OK)
	{
		if (ulSSIDLen < strlen(xResp.pSSID))
		{
			return	FTM_RET_BUFFER_TOO_SMALL;	
		}

		strncpy(pSSID, xResp.pSSID, ulSSIDLen);
	}

	return	xRet;
}

FTM_RET	FTM_CLIENT_SSID_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulSSIDLen
)
{
	FTM_REQ_SSID_DEL_PARAMS	xReq;
	FTM_RESP_SSID_DEL_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_STAT_DEL2;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SESSION_ID_LEN);

	return	FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
}

FTM_RET	FTM_CLIENT_SSID_verify
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulSSIDLen
)
{
	FTM_REQ_SSID_VERIFY_PARAMS	xReq;
	FTM_RESP_SSID_VERIFY_PARAMS	xResp;

	if ((pClient == NULL) || (pClient->hSock == 0))
	{
		return	FTM_RET_CLIENT_HANDLE_INVALID;	
	}

	memset(&xReq, 0, sizeof(xReq));

	xReq.xCommon.xCmd	=	FTM_CMD_SSID_VERIFY;
	xReq.xCommon.ulLen	=	sizeof(xReq);
	strncpy(xReq.xCommon.pSSID, pSSID, FTM_SSID_LEN);

	return	FTM_CLIENT_request( pClient, (FTM_VOID_PTR)&xReq, sizeof(xReq), (FTM_VOID_PTR)&xResp, sizeof(xResp));
}

