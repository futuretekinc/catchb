#include <string.h>
#include <stdlib.h>
#include "ftm_catchb.h"
#include "ftm_analyzer.h"
#include "ftm_trace.h"
#include "ftm_message.h"
#include "ftm_utils.h"
#include "ftm_mem.h"
#include "ftm_ping.h"

#undef	__MODULE__
#define	__MODULE__	"ANALYZER"

FTM_BOOL	FTM_ANALYZER_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

FTM_RET	FTM_ANALYZER_process
(
	FTM_ANALYZER_PTR	pAnalyzer
);

FTM_VOID_PTR	FTM_ANALYZER_threadMain
(
	FTM_VOID_PTR	pParam
);

FTM_RET	FTM_ANALYZER_onAddCCTV
(
	FTM_ANALYZER_PTR		pAnalyzer,
	FTM_MSG_ADD_CCTV_PTR	pMsg
);

FTM_RET	FTM_ANALYZER_onDeleteCCTV
(
	FTM_ANALYZER_PTR		pAnalyzer,
	FTM_MSG_DELETE_CCTV_PTR	pMsg
);

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTM_ANALYZER_CONFIG_setDefault
(
	FTM_ANALYZER_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	
	pConfig->xTest.bEnable 		= FTM_CATCHB_ANALYZER_DEFAULT_TEST_ENABLE;
	pConfig->xTest.ulErrorRate 	= FTM_CATCHB_ANALYZER_DEFAULT_TEST_ERROR_RATE;

	pConfig->ulIPCheckInterval 	= FTM_CATCHB_ANALYZER_DEFAULT_IP_CHECK_INTERVAL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_ANALYZER_CONFIG_load
(
	FTM_ANALYZER_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pSection;
	cJSON _PTR_ pItem;

	pItem = cJSON_GetObjectItem(pRoot, "interval");
	if (pItem != NULL)
	{
		if (FTM_CATCHB_ANALYZER_MINIMUM_IP_CHECK_INTERVAL <= pItem->valueint)
		{
			pConfig->ulIPCheckInterval = pItem->valueint;	
		}
	}

	pSection = cJSON_GetObjectItem(pRoot, "test");
	if (pSection != NULL)
	{
		pItem = cJSON_GetObjectItem(pSection, "enable");
		if (pItem != NULL)
		{
			if ((strcmp(pItem->valuestring, "on") == 0) || (strcmp(pItem->valuestring, "yes") == 0))
			{
				pConfig->xTest.bEnable = FTM_TRUE;	
			}
			else if ((strcmp(pItem->valuestring, "off") == 0) || (strcmp(pItem->valuestring, "no") == 0))
			{
				pConfig->xTest.bEnable = FTM_FALSE;	
			}
			else
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}
		}

		pItem = cJSON_GetObjectItem(pSection, "error_rate");
		if (pItem != NULL)
		{
			if (0 <= pItem->valueint && pItem->valueint <= 100)
			{
				pConfig->xTest.ulErrorRate = pItem->valueint;	
			}
		}
	}

	return	FTM_RET_OK;
}


FTM_RET	FTM_ANALYZER_CONFIG_show
(
	FTM_ANALYZER_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	LOG("[ Analyzer Configuration ]");
	LOG("%16s : %d ms", "IP Check Interval", pConfig->ulIPCheckInterval);
	LOG("%16s : %s", "Test Enabled", (pConfig->xTest.bEnable)?"yes":"no");
	LOG("%16s : %d %", "Test Error Rate", pConfig->xTest.ulErrorRate);

	return	FTM_RET_OK;
}

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTM_ANALYZER_create
(	
	struct	FTM_CATCHB_STRUCT _PTR_	pCatchB,
	FTM_ANALYZER_PTR _PTR_ ppAnalyzer
)
{
	ASSERT(ppAnalyzer != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_ANALYZER_PTR	pAnalyzer;

	pAnalyzer = (FTM_ANALYZER_PTR)FTM_MEM_malloc(sizeof(FTM_ANALYZER));
	if (pAnalyzer == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create scheduler!\n");
		goto error;
	}

	strcpy(pAnalyzer->pName, __MODULE__);

	FTM_ANALYZER_CONFIG_setDefault(&pAnalyzer->xConfig);

	xRet = FTM_LOCK_create(&pAnalyzer->pLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create lock!");
		goto error;
	}

	xRet = FTM_LIST_create(&pAnalyzer->pList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create CCTV list!");
		goto error;
	}
	FTM_LIST_setSeeker(pAnalyzer->pList, FTM_ANALYZER_seeker);

	xRet = FTM_MSGQ_create(&pAnalyzer->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create message queue!\n");
		goto error;
	}

	pAnalyzer->pCatchB = pCatchB;

	*ppAnalyzer = pAnalyzer;	

	return	xRet;

error:
	if (pAnalyzer != NULL)
	{
		if (pAnalyzer->pMsgQ != NULL)
		{
			FTM_MSGQ_destroy(&pAnalyzer->pMsgQ);	
		}

		if (pAnalyzer->pList != NULL)
		{
			FTM_LIST_destroy(&pAnalyzer->pList);	
		}

		if (pAnalyzer->pLock != NULL)
		{
			FTM_LOCK_destroy(&pAnalyzer->pLock);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_ANALYZER_destroy
(
	FTM_ANALYZER_PTR _PTR_ ppAnalyzer
)
{
	ASSERT(ppAnalyzer != NULL);
	ASSERT(*ppAnalyzer != NULL);

	if ((*ppAnalyzer)->pMsgQ != NULL)
	{
		FTM_MSG_PTR	pMsg;

		while(FTM_MSGQ_pop((*ppAnalyzer)->pMsgQ, (FTM_VOID_PTR _PTR_)&pMsg) == FTM_RET_OK)
		{
			FTM_MEM_free(pMsg);
		}
		FTM_MSGQ_destroy(&(*ppAnalyzer)->pMsgQ);	
	}

	if ((*ppAnalyzer)->pList != NULL)
	{
		FTM_CHAR_PTR	pID;

		FTM_LIST_iteratorStart((*ppAnalyzer)->pList);
		while(FTM_LIST_iteratorNext((*ppAnalyzer)->pList, (FTM_VOID_PTR _PTR_)&pID) == FTM_RET_OK)
		{
			FTM_MEM_free(pID);
		}
		FTM_LIST_destroy(&(*ppAnalyzer)->pList);
	}

	if ((*ppAnalyzer)->pLock != NULL)
	{
		FTM_LOCK_destroy(&(*ppAnalyzer)->pLock);
	}

	FTM_MEM_free(*ppAnalyzer);
	
	*ppAnalyzer= NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_ANALYZER_setConfig
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_ANALYZER_CONFIG_PTR	pConfig
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	if (!pAnalyzer->bStop)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
		ERROR(xRet, "Failed to set config!");
	}
	else
	{
		memcpy(&pAnalyzer->xConfig, pConfig, sizeof(FTM_ANALYZER_CONFIG));
	}

	return	xRet;
}

FTM_RET	FTM_ANALYZER_start
(
	FTM_ANALYZER_PTR	pAnalyzer
)
{
	ASSERT(pAnalyzer != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pAnalyzer->xThread != 0)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
		TRACE("The %s is already running!", pAnalyzer->pName);	
		return	xRet;
	}

	if (pthread_create(&pAnalyzer->xThread, NULL, FTM_ANALYZER_threadMain, (FTM_VOID_PTR)pAnalyzer) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		TRACE("Failed to start %s!", pAnalyzer->pName);
	}

    return xRet;
}

FTM_RET	FTM_ANALYZER_stop
(
	FTM_ANALYZER_PTR	pAnalyzer
)
{
	ASSERT(pAnalyzer != NULL);

	if (pAnalyzer->xThread != 0)
	{
		pAnalyzer->bStop = FTM_TRUE;

		pthread_join(pAnalyzer->xThread, NULL);

		pAnalyzer->xThread = 0;
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_ANALYZER_threadMain
(
	FTM_VOID_PTR pData
)
{
	FTM_RET				xRet;
	FTM_ANALYZER_PTR	pAnalyzer = (FTM_ANALYZER_PTR)pData;

	pAnalyzer->bStop = FTM_FALSE;

	while(!pAnalyzer->bStop)
	{
		FTM_MSG_PTR	pRcvdMsg;

		xRet = FTM_MSGQ_timedPop(pAnalyzer->pMsgQ, 1000, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pRcvdMsg->xType)
			{
			case	FTM_MSG_TYPE_ADD_CCTV:
					xRet = FTM_ANALYZER_onAddCCTV(pAnalyzer, (FTM_MSG_ADD_CCTV_PTR)pRcvdMsg);
				break;

			case	FTM_MSG_TYPE_DELETE_CCTV:
					xRet = FTM_ANALYZER_onDeleteCCTV(pAnalyzer, (FTM_MSG_DELETE_CCTV_PTR)pRcvdMsg);
				break;

			default:
				break;
			}
				
			FTM_MEM_free(pRcvdMsg);
		}

		xRet = FTM_ANALYZER_process(pAnalyzer);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to update analyzer!\n");	
		}
	}

	return	0;
}


FTM_RET	FTM_ANALYZER_process
(
	FTM_ANALYZER_PTR	pAnalyzer
)
{
	FTM_RET			xRet;
	FTM_UINT32		ulCount = 0;

	xRet = FTM_LIST_count(pAnalyzer->pList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get cctv count from DB!\n");
		return	xRet;	
	}
	else if (ulCount == 0)
	{
		return	FTM_RET_OK;
	}

	FTM_CHAR_PTR	pID;

	xRet = FTM_LIST_getAt(pAnalyzer->pList, 0, (FTM_VOID_PTR _PTR_)&pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV ID from scheduler!");
		return	xRet;
	}

	FTM_CCTV_PTR	pCCTV = NULL;
	xRet = FTM_CATCHB_CCTV_get(pAnalyzer->pCatchB, pID, &pCCTV);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV[%s] from CatchB!", pID);
		return	xRet;
	}

	FTM_CCTV_lock(pCCTV);

	if (FTM_TIMER_isExpired(&pCCTV->xExpiredTimer) == FTM_TRUE)
	{
		FTM_CHAR		pHashData[1024];
		FTM_UINT32		ulHashDataLen = 0;
		FTM_CHAR		pHashValue[64];
		FTM_UINT32		ulReplyCount = 0;

		memset(pHashData, 0, sizeof(pHashData));
		memset(pHashValue, 0, sizeof(pHashValue));

		TRACE("CCTV[%s] analyzing!", pCCTV->xConfig.pID);

		FTM_PING_check(pCCTV->xConfig.pIP, &ulReplyCount);

		if(ulReplyCount == 0)
		{ 
			TRACE("CCTV[%s] not responsed!", pCCTV->xConfig.pID);
			if (pCCTV->xConfig.xStat != FTM_CCTV_STAT_UNREGISTERED)
			{
				FTM_CATCHB_CCTV_setStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_UNUSED);

			}
		}
		else
		{
			FTM_UINT32	j;
			FTM_CHAR	pMAC[24]; 

			FTM_ARP_parsing(pCCTV->xConfig.pIP, pMAC); 

			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen , "[ip : %s, mac : %s]", pCCTV->xConfig.pIP, pMAC);
			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, " [port :");

			for(j =0 ;j < PORT_NUM ; j++)
			{
				FTM_BOOL	bOpened = FTM_FALSE;

				FTM_portScan(pCCTV->xConfig.pIP, pPortList[j], &bOpened);
				if (j != 0)
				{
					ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, ", ");
				}

				TRACE("Port %5d : %s", pPortList[j], (bOpened)?"open":"close"); 
				ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, "%d - %s", pPortList[j], (bOpened)?"open":"close"); 
			}
			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, "]");

			FTM_HASH_SHA1((FTM_UINT8_PTR)pHashData, ulHashDataLen, pHashValue, sizeof(pHashValue));

			if (strlen(pCCTV->xConfig.pHash) == 0)
			{
				FTM_CATCHB_CCTV_register(pAnalyzer->pCatchB, pCCTV->xConfig.pID, pHashValue);
			}
			else
			{
				FTM_BOOL	bTestFailed = FTM_FALSE;

				if (pAnalyzer->xConfig.xTest.bEnable)
				{
					if (pAnalyzer->xConfig.xTest.ulErrorRate > (rand() % 100))
					{
						bTestFailed = FTM_TRUE;	
					}
				}

				if(!bTestFailed && !strncmp(pCCTV->xConfig.pHash, pHashValue, sizeof(pHashValue)))
				{
					FTM_CATCHB_CCTV_setStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_NORMAL);
				}
				else
				{
					TRACE("CCTV[%s] hash is abnormal.", pCCTV->xConfig.pID);
					TRACE("Original Hash : %s", pCCTV->xConfig.pHash);
					TRACE(" Current Hash : %s", pHashValue);
					
					FTM_CATCHB_CCTV_setStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_ABNORMAL);
				}

			}
		}

		FTM_TIMER_addMS(&pCCTV->xExpiredTimer, pAnalyzer->xConfig.ulIPCheckInterval);

		FTM_LIST_remove(pAnalyzer->pList, pID);
		FTM_LIST_append(pAnalyzer->pList, pID);
	}

	FTM_CCTV_unlock(pCCTV);


	return	FTM_RET_OK;
}


FTM_RET	FTM_ANALYZER_CCTV_add
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pID != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_MSG_ADD_CCTV_PTR pMsg;

	TRACE("Request CCTV[%s] add.", pID);
	pMsg = (FTM_MSG_ADD_CCTV_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_ADD_CCTV));
	if(pMsg == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create CCTV!\n");
	}
	else
	{
		pMsg->xHead.xType = FTM_MSG_TYPE_ADD_CCTV;	
		pMsg->xHead.ulLen = sizeof(FTM_MSG_ADD_CCTV);
		strncpy(pMsg->pID, pID, sizeof(pMsg->pID) - 1);

		xRet = FTM_MSGQ_push(pAnalyzer->pMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to send message!\n");
			FTM_MEM_free(pMsg);	
		}
	}
	
	return	xRet;
}

FTM_RET	FTM_ANALYZER_onAddCCTV
(
	FTM_ANALYZER_PTR		pAnalyzer,
	FTM_MSG_ADD_CCTV_PTR	pMsg
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET			xRet;
	FTM_CHAR_PTR	pID = NULL;

	TRACE("ADD CCTV : %s", pMsg->pID);

	FTM_LOCK_set(pAnalyzer->pLock);

	xRet = FTM_LIST_get(pAnalyzer->pList, pMsg->pID, (FTM_VOID_PTR _PTR_)&pID);
	if (xRet == FTM_RET_OK)
	{
		ERROR(xRet, "Already exist!");	
	}
	else
	{
		pID = (FTM_CHAR_PTR)FTM_MEM_malloc(sizeof(FTM_ID));
		if (pID == NULL)
		{
			ERROR(xRet, "Failed to create ID!");
		}
		else
		{
			strncpy(pID, pMsg->pID, sizeof(FTM_ID) - 1);
			xRet = FTM_LIST_append(pAnalyzer->pList, pID);
			if (xRet != FTM_RET_OK)
			{
				FTM_MEM_free(pID);
				ERROR(xRet, "Failed to append CCTV!");
			}
		}
	}

	FTM_LOCK_reset(pAnalyzer->pLock);

	return	xRet;
}

FTM_RET	FTM_ANALYZER_CCTV_delete
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pID != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_MSG_DELETE_CCTV_PTR pMsg;
	
	pMsg = (FTM_MSG_DELETE_CCTV_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_DELETE_CCTV));
	if(pMsg == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create message!\n");
	}
	else
	{
		pMsg->xHead.xType = FTM_MSG_TYPE_DELETE_CCTV;	
		pMsg->xHead.ulLen = sizeof(FTM_MSG_DELETE_CCTV);
		strncpy(pMsg->pID, pID, FTM_ID_LEN);

		xRet = FTM_MSGQ_push(pAnalyzer->pMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to send message!\n");
			FTM_MEM_free(pMsg);	
		}
	}
	
	return	xRet;
}

FTM_RET	FTM_ANALYZER_onDeleteCCTV
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_MSG_DELETE_CCTV_PTR	pMsg
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_CHAR_PTR	pID = NULL;

	FTM_LOCK_set(pAnalyzer->pLock);

	xRet = FTM_LIST_get(pAnalyzer->pList, pMsg->pID, (FTM_VOID_PTR _PTR_)&pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV[%s]!", pMsg->pID);
	}
	else
	{
		xRet = FTM_LIST_remove(pAnalyzer->pList, pID);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to remove CCTV[%s]!", pMsg->pID);
		}
		else
		{
			FTM_MEM_free(pID);
		}
	}

	FTM_LOCK_reset(pAnalyzer->pLock);

	return	xRet;
}

FTM_RET	FTM_ANALYZER_CCTV_count
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_UINT32_PTR		pCount
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pCount != NULL);

	return	FTM_LIST_count(pAnalyzer->pList, pCount);
}

FTM_RET	FTM_ANALYZER_CCTV_getList
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_ID_PTR			pIDArray,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pIDArray != NULL);
	ASSERT(pCount != NULL);
	FTM_UINT32	i;

	FTM_LOCK_set(pAnalyzer->pLock);

	FTM_LIST_iteratorStart(pAnalyzer->pList);
	for(i = 0 ; i < ulMaxCount ; i++)
	{
		FTM_CHAR_PTR	pID;

		if (FTM_LIST_iteratorNext(pAnalyzer->pList, (FTM_VOID_PTR _PTR_)&pID) != FTM_RET_OK)
		{
			break;	
		}

		strncpy(pIDArray[i], pID, FTM_ID_LEN);
	}

	FTM_LOCK_reset(pAnalyzer->pLock);

	*pCount = i;

	return	FTM_RET_OK;
}


FTM_BOOL	FTM_ANALYZER_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	(strcasecmp((FTM_CHAR_PTR)pElement, (FTM_CHAR_PTR)pIndicator) == 0);
}
