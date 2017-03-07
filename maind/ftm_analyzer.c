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
#define	__MODULE__	"analyzer"


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

/*****************************************************************
 *
 *****************************************************************/
FTM_RET	FTM_ANALYZER_CONFIG_setDefault
(
	FTM_ANALYZER_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);


	pConfig->ulPortCount = 0;
	pConfig->pPortList[pConfig->ulPortCount++] = 80;
	pConfig->pPortList[pConfig->ulPortCount++] = 135;
	pConfig->pPortList[pConfig->ulPortCount++] = 139;
	pConfig->pPortList[pConfig->ulPortCount++] = 443; 
	pConfig->pPortList[pConfig->ulPortCount++] = 445; 
	pConfig->pPortList[pConfig->ulPortCount++] = 554; 
	pConfig->pPortList[pConfig->ulPortCount++] = 4520; 
	pConfig->pPortList[pConfig->ulPortCount++] = 49152;

	pConfig->ulIPCheckInterval 	= FTM_CATCHB_ANALYZER_DEFAULT_IP_CHECK_INTERVAL;

	pConfig->xTest.bEnable 		= FTM_CATCHB_ANALYZER_DEFAULT_TEST_ENABLE;
	pConfig->xTest.ulErrorRate 	= FTM_CATCHB_ANALYZER_DEFAULT_TEST_ERROR_RATE;

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

	pSection = cJSON_GetObjectItem(pRoot, "port");
	if (pSection != NULL)
	{
		if(pSection->type == cJSON_Array)
		{
			FTM_INT32	ulCount, i;

			pConfig->ulPortCount = 0;

			ulCount = cJSON_GetArraySize(pSection);
			for(i = 0 ; i < ulCount && i < sizeof(pConfig->pPortList) / sizeof(pConfig->pPortList[0]) ; i++)
			{
				pItem = cJSON_GetArrayItem(pSection, i);
				if (pItem != NULL)
				{
					pConfig->pPortList[pConfig->ulPortCount++] = pItem->valueint;		
				}
			}	
		}
	}

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
	FTM_ANALYZER_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL	xLevel
)
{
	ASSERT(pConfig != NULL);
	FTM_CHAR	pBuffer[2048];
	FTM_INT32	i, ulLen = 0;

	memset(pBuffer, 0, sizeof(pBuffer));
	for(i = 0 ; i < pConfig->ulPortCount ; i++)
	{
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "%5d ", pConfig->pPortList[i]);
	}

	OUTPUT(xLevel, "");
	OUTPUT(xLevel, "[ Analyzer Configuration ]");
	OUTPUT(xLevel, "%16s : %d ms", 	"Interval", pConfig->ulIPCheckInterval);
	OUTPUT(xLevel, "%16s : %s", "Port", pBuffer);
	OUTPUT(xLevel, "%16s : %s", 	"Test Enabled", (pConfig->xTest.bEnable)?"yes":"no");
	OUTPUT(xLevel, "%16s : %d %", 	"Test Error Rate", pConfig->xTest.ulErrorRate);

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

	pAnalyzer->bStop = FTM_TRUE;
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
		ERROR(xRet, "Failed to set analyzer configuration!");
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
		INFO("The %s is already running!", pAnalyzer->pName);	
		return	xRet;
	}

	if (pthread_create(&pAnalyzer->xThread, NULL, FTM_ANALYZER_threadMain, (FTM_VOID_PTR)pAnalyzer) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		INFO("Failed to start %s!", pAnalyzer->pName);
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

	INFO("%s started.", pAnalyzer->pName);

	pAnalyzer->bStop = FTM_FALSE;

	while(!pAnalyzer->bStop)
	{
		FTM_MSG_PTR	pRcvdMsg;

		xRet = FTM_MSGQ_timedPop(pAnalyzer->pMsgQ, 1000, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pRcvdMsg->xType)
			{
			default:
				INFO("Unknown message[%s]", FTM_MESSAGE_getString(pRcvdMsg->xType));
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

	INFO("%s stopped.", pAnalyzer->pName);

	return	0;
}


FTM_RET	FTM_ANALYZER_process
(
	FTM_ANALYZER_PTR	pAnalyzer
)
{
	ASSERT(pAnalyzer != NULL);

	FTM_RET			xRet;
	FTM_UINT32		ulCount = 0;
	FTM_CHAR_PTR	pID;

	FTM_LOCK_set(pAnalyzer->pLock);

	xRet = FTM_LIST_count(pAnalyzer->pList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get cctv count from DB!\n");
		goto finished;
	}
	else if (ulCount == 0)
	{
		xRet = FTM_RET_OK;
		goto finished;
	}

	xRet = FTM_LIST_getAt(pAnalyzer->pList, 0, (FTM_VOID_PTR _PTR_)&pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV ID from scheduler!");
		goto finished;
	}

	FTM_CCTV_PTR	pCCTV = NULL;
	xRet = FTM_CATCHB_getCCTV(pAnalyzer->pCatchB, pID, &pCCTV);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV[%s] from CatchB!", pID);
		goto finished;
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

		INFO("CCTV[%s] : Start status analysis!", pID);

		FTM_PING_check(pCCTV->xConfig.pIP, &ulReplyCount);

		if(ulReplyCount == 0)
		{ 
			INFO("CCTV[%s] : No response!", pCCTV->xConfig.pID);
			if (pCCTV->xConfig.xStat != FTM_CCTV_STAT_UNREGISTERED)
			{
				FTM_CATCHB_setCCTVStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_UNUSED);

			}
		}
		else
		{
			FTM_UINT32	j;
			FTM_CHAR	pMAC[24]; 

			FTM_ARP_parsing(pCCTV->xConfig.pIP, pMAC); 

			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen , "[ip : %s, mac : %s]", pCCTV->xConfig.pIP, pMAC);
			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, " [port :");

			pCCTV->ulPortCount = 0;
			for(j =0 ;j < FTM_CATCHB_ANALYZER_MAX_PORT_COUNT ; j++)
			{
				FTM_BOOL	bOpened = FTM_FALSE;

				FTM_portScan(pCCTV->xConfig.pIP, pAnalyzer->xConfig.pPortList[j], &bOpened);
				if (j != 0)
				{
					ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, ", ");
				}

				pCCTV->pPortList[pCCTV->ulPortCount] = pAnalyzer->xConfig.pPortList[j];
				pCCTV->pPortStat[pCCTV->ulPortCount] = bOpened;
				pCCTV->ulPortCount++;

				ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, "%d - %s", pAnalyzer->xConfig.pPortList[j], (bOpened)?"open":"close"); 
			}
			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, "]");

			FTM_HASH_SHA1((FTM_UINT8_PTR)pHashData, ulHashDataLen, pHashValue, sizeof(pHashValue));

			if (strlen(pCCTV->xConfig.pHash) == 0)
			{
				FTM_CATCHB_registerCCTV(pAnalyzer->pCatchB, pCCTV->xConfig.pID, pHashValue);
			}
			else
			{
				FTM_BOOL	bTestFailed = FTM_FALSE;

				if (pAnalyzer->xConfig.xTest.bEnable)
				{
					FTM_UINT32	ulErrorProbability = rand() % 100;
					INFO("Error probability : %u", ulErrorProbability);
					if (pAnalyzer->xConfig.xTest.ulErrorRate > ulErrorProbability)
					{
						bTestFailed = FTM_TRUE;	
					}
				}

				if(!bTestFailed && !strncmp(pCCTV->xConfig.pHash, pHashValue, sizeof(pHashValue)))
				{
					FTM_CATCHB_setCCTVStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_NORMAL);
				}
				else
				{
					INFO("CCTV[%s] hash is abnormal.", pCCTV->xConfig.pID);
					INFO("Original Hash : %s", pCCTV->xConfig.pHash);
					INFO(" Current Hash : %s", pHashValue);
					
					FTM_CATCHB_setCCTVStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_ABNORMAL);
				}

			}
		}

		INFO("CCTV[%s] : Status analysis completed!", pID);

		FTM_TIMER_addMS(&pCCTV->xExpiredTimer, pAnalyzer->xConfig.ulIPCheckInterval);

		FTM_LIST_remove(pAnalyzer->pList, pID);
		FTM_LIST_append(pAnalyzer->pList, pID);
	}

	FTM_CCTV_unlock(pCCTV);


finished:

	FTM_LOCK_reset(pAnalyzer->pLock);

	return	xRet;
}

FTM_RET	FTM_ANALYZER_getPortCount
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_UINT32_PTR		pCount
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pCount != NULL);

	*pCount = pAnalyzer->xConfig.ulPortCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_ANALYZER_getPortList
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_UINT16_PTR		pPortList,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pPortList != NULL);
	ASSERT(pCount != NULL);

	FTM_INT32	i;

	for(i = 0 ; i < ulMaxCount && i < pAnalyzer->xConfig.ulPortCount ; i++)
	{
		pPortList[i] = pAnalyzer->xConfig.pPortList[i];	
	}

	*pCount = i;

	return	FTM_RET_OK;
}


FTM_RET	FTM_ANALYZER_CCTV_attach
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pID != NULL);

	FTM_RET			xRet;
	FTM_CHAR_PTR	pSavedID = NULL;

	FTM_LOCK_set(pAnalyzer->pLock);

	xRet = FTM_LIST_get(pAnalyzer->pList, pID, (FTM_VOID_PTR _PTR_)&pSavedID);
	if (xRet == FTM_RET_OK)
	{
		ERROR(xRet, "Already exist!");	
	}
	else
	{
		pSavedID = (FTM_CHAR_PTR)FTM_MEM_malloc(sizeof(FTM_ID));
		if (pSavedID == NULL)
		{
			ERROR(xRet, "Failed to create ID!");
		}
		else
		{
			strncpy(pSavedID, pID, sizeof(FTM_ID) - 1);
			xRet = FTM_LIST_append(pAnalyzer->pList, pSavedID);
			if (xRet != FTM_RET_OK)
			{
				FTM_MEM_free(pSavedID);
				ERROR(xRet, "Failed to attach CCTV!");
			}
			else
			{
				INFO("CCTV[%s] has been attached to the analyzer for analysis.", pSavedID);
			}
		}
	}

	FTM_LOCK_reset(pAnalyzer->pLock);

	return	xRet;
}

FTM_RET	FTM_ANALYZER_CCTV_detach
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pID != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_CHAR_PTR	pSavedID = NULL;

	FTM_LOCK_set(pAnalyzer->pLock);

	xRet = FTM_LIST_get(pAnalyzer->pList, pID, (FTM_VOID_PTR _PTR_)&pSavedID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV[%s]!", pID);
	}
	else
	{
		xRet = FTM_LIST_remove(pAnalyzer->pList, pSavedID);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to remove CCTV[%s]!", pID);
		}
		else
		{
			INFO("CCTV[%s] has been removed from the analyzer.", pSavedID);
			FTM_MEM_free(pSavedID);
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
