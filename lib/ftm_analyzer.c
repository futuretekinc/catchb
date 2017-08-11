#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/ip.h>
#include "ftm_catchb.h"
#include "ftm_analyzer.h"
#include "ftm_trace.h"
#include "ftm_message.h"
#include "ftm_utils.h"
#include "ftm_mem.h"
#include "ftm_ping.h"
#include "ftm_catchb.h"
#include "ftm_score.h"

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

FTM_VOID_PTR FTM_ANALYZER_threadPCAP
(
	FTM_VOID_PTR pData
);

FTM_RET	FTM_ANALIZER_PCAP_calcScore
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_SCORE_PTR		pScore
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

	memset(pConfig->pPortList, 0, sizeof(pConfig->pPortList));
	pConfig->ulPortCount = 0;
	pConfig->pPortList[pConfig->ulPortCount++] = 80;
	pConfig->pPortList[pConfig->ulPortCount++] = 135;
	pConfig->pPortList[pConfig->ulPortCount++] = 139;
	pConfig->pPortList[pConfig->ulPortCount++] = 443; 
	pConfig->pPortList[pConfig->ulPortCount++] = 445; 
	pConfig->pPortList[pConfig->ulPortCount++] = 554; 
	pConfig->pPortList[pConfig->ulPortCount++] = 4520; 
	pConfig->pPortList[pConfig->ulPortCount++] = 49152;

	strcpy(pConfig->pIFName, "wan");
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
	
	FTM_RET	xRet = FTM_RET_OK;
	cJSON _PTR_ 	pSection;
	cJSON _PTR_ 	pItem;
	FTM_UINT16_PTR	pPortList = NULL;
	FTM_UINT32		ulPortCount = 0;

	pSection = cJSON_GetObjectItem(pRoot, "ifname");
	if (pSection != NULL)
	{
		strncpy(pConfig->pIFName, pSection->valuestring, sizeof(pConfig->pIFName));
	}

	pSection = cJSON_GetObjectItem(pRoot, "port");
	if (pSection != NULL)
	{
		if(pSection->type == cJSON_Array)
		{
			ulPortCount = cJSON_GetArraySize(pSection);
			if (ulPortCount > 0)
			{
				FTM_INT32	i;

				pPortList = FTM_MEM_malloc(sizeof(FTM_UINT16) * ulPortCount);
				if (pPortList == NULL)
				{
					xRet = FTM_RET_NOT_ENOUGH_MEMORY;
					ERROR(xRet, "Failed to load configuration due to not enough memory!");
					goto finished;
				}

				for(i = 0 ; i < ulPortCount ; i++)
				{
					pItem = cJSON_GetArrayItem(pSection, i);
					if (pItem == NULL)
					{
						xRet = FTM_RET_INVALID_JSON_FORMAT;
						ERROR(xRet, "Failed to load configuration due to JSON format error!");
						goto finished;
					}
					pPortList[i] = pItem->valueint;		
				}	
			}
		}
		else
		{
			xRet = FTM_RET_INVALID_JSON_FORMAT;
			ERROR(xRet, "Failed to load configuration due to JSON format error!");
			goto finished;
		}

		if (ulPortCount > 32)
		{
			ulPortCount = 32;
		}
		memcpy(pConfig->pPortList, pPortList, sizeof(FTM_UINT16)*ulPortCount);
		pConfig->ulPortCount = ulPortCount;
	}

	pItem = cJSON_GetObjectItem(pRoot, "interval");
	if (pItem != NULL)
	{
		INFO("Analyzer interval : %d", pItem->valueint);
		if (FTM_CATCHB_ANALYZER_MINIMUM_IP_CHECK_INTERVAL <= pItem->valueint)
		{
			pConfig->ulIPCheckInterval = pItem->valueint;	
		}
		else
		{
			WARN(FTM_RET_INVALID_JSON_FORMAT, "Failed to load configuration due to invalid interval value[ < %d]!",FTM_CATCHB_ANALYZER_MINIMUM_IP_CHECK_INTERVAL);
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
				WARN(FTM_RET_INVALID_JSON_FORMAT, "Failed to load configuration due to invalid enable value[yes or no]!");
			}
		}

		pItem = cJSON_GetObjectItem(pSection, "error_rate");
		if (pItem != NULL)
		{
			if (0 <= pItem->valueint && pItem->valueint <= 100)
			{
				pConfig->xTest.ulErrorRate = pItem->valueint;	
			}
			else
			{
				WARN(FTM_RET_INVALID_VALUE, "Failed to load configuration due to invalid error_rate value [0 ~ 100]!");
			}
		}
	}

finished:

	if (pPortList != NULL)
	{
		FTM_MEM_free(pPortList);
	}

	return	xRet;
}


FTM_RET	FTM_ANALYZER_CONFIG_save
(
	FTM_ANALYZER_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);
	
	cJSON _PTR_ 	pSection;
	cJSON _PTR_ 	pItem;

	pSection = cJSON_CreateArray();
	if (pSection != NULL)
	{
		FTM_UINT32	i;

		for(i = 0 ; i < pConfig->ulPortCount ; i++)
		{
			pItem = cJSON_CreateNumber(pConfig->pPortList[i]);
			if (pItem != NULL)
			{
				cJSON_AddItemToArray(pSection, pItem);	
			}
		}

		cJSON_AddItemToObject(pRoot, "port", pSection);
	}

	cJSON_AddNumberToObject(pRoot, "interval", pConfig->ulIPCheckInterval);
	
	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		cJSON_AddStringToObject(pSection, "enable", (pConfig->xTest.bEnable?"yes":"no"));
		cJSON_AddNumberToObject(pSection, "error_rate", pConfig->xTest.ulErrorRate);

		cJSON_AddItemToObject(pRoot, "test", pSection);
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

	printf("\n[ Analyzer Configuration ]\n");
	printf("%16s : %d ms\n", 	"Interval", pConfig->ulIPCheckInterval);
	printf("%16s : %s\n", 	"Interface Name", pConfig->pIFName);
	printf("%16s : %s\n", 	"Port", pBuffer);
	printf("%16s : %s\n", 	"Test Enabled", (pConfig->xTest.bEnable)?"yes":"no");
	printf("%16s : %d %%\n", 	"Test Error Rate", pConfig->xTest.ulErrorRate);

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
		goto finished;
	}

	strcpy(pAnalyzer->pName, __MODULE__);

	FTM_ANALYZER_CONFIG_setDefault(&pAnalyzer->xConfig);

	xRet = FTM_LOCK_create(&pAnalyzer->pPCAPLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create pcap lock!");
		goto finished;
	}

	xRet = FTM_LOCK_create(&pAnalyzer->pLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create lock!");
		goto finished;
	}

	xRet = FTM_LIST_create(&pAnalyzer->pList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create CCTV list!");
		goto finished;
	}
	FTM_LIST_setSeeker(pAnalyzer->pList, FTM_ANALYZER_seeker);

	xRet = FTM_MSGQ_create(&pAnalyzer->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create message queue!\n");
		goto finished;
	}

	xRet = FTM_PCAP_create(&pAnalyzer->pPCAP);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create pcap!");
		goto finished;	
	}

	pAnalyzer->bStop = FTM_TRUE;
	pAnalyzer->pCatchB = pCatchB;

	*ppAnalyzer = pAnalyzer;	

finished:
	if (xRet != FTM_RET_OK)
	{
		if (pAnalyzer != NULL)
		{
			if (pAnalyzer->pPCAP != NULL)
			{
				FTM_PCAP_destroy(&pAnalyzer->pPCAP);	
			}

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

			if (pAnalyzer->pPCAPLock != NULL)
			{
				FTM_LOCK_destroy(&pAnalyzer->pPCAPLock);	
			}
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

	FTM_ANALYZER_stop((*ppAnalyzer));

	if ((*ppAnalyzer)->pPCAP != NULL)
	{
		FTM_PCAP_destroy(&(*ppAnalyzer)->pPCAP);	
	}

	if ((*ppAnalyzer)->pMsgQ != NULL)
	{
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


	if ((*ppAnalyzer)->pPCAPLock != NULL)
	{
		FTM_LOCK_destroy(&(*ppAnalyzer)->pPCAPLock);	
	}

	FTM_MEM_free(*ppAnalyzer);

	INFO("Analyzer destroyed!");
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

FTM_RET	FTM_ANALYZER_getConfig
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_ANALYZER_CONFIG_PTR	pConfig
)
{
	ASSERT(pAnalyzer != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	memcpy(pConfig, &pAnalyzer->xConfig, sizeof(FTM_ANALYZER_CONFIG));

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
		ERROR(xRet, "Failed to start %s!", pAnalyzer->pName);
	}

	if (pthread_create(&pAnalyzer->xPCAPThread, NULL, FTM_ANALYZER_threadPCAP, (FTM_VOID_PTR)pAnalyzer) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		ERROR(xRet, "Failed to start %s!", pAnalyzer->pName);
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
		FTM_MSGQ_pushQuit(pAnalyzer->pMsgQ);

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

#if 0
	xRet = FTM_PCAP_open(pAnalyzer->pPCAP, pAnalyzer->xConfig.pIFName);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to open pcap!");
		goto finished;
	}
#endif

	pAnalyzer->bStop = FTM_FALSE;

	while(!pAnalyzer->bStop)
	{
		FTM_MSG_PTR	pRcvdMsg;

		xRet = FTM_MSGQ_timedPop(pAnalyzer->pMsgQ, 10, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pRcvdMsg->xType)
			{
			case	FTM_MSG_TYPE_QUIT:
				{
					pAnalyzer->bStop = FTM_TRUE;
				}
				break;

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

	//FTM_PCAP_close(pAnalyzer->pPCAP);

	INFO("%s stopped.", pAnalyzer->pName);

//finished:

	return	0;
}

FTM_VOID_PTR FTM_ANALYZER_threadPCAP
(
	FTM_VOID_PTR pData
)
{
	FTM_RET				xRet;
	FTM_ANALYZER_PTR	pAnalyzer = (FTM_ANALYZER_PTR)pData;

	INFO("%s started.", pAnalyzer->pName);

	xRet = FTM_PCAP_open(pAnalyzer->pPCAP, pAnalyzer->xConfig.pIFName);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to open pcap![%s]", pAnalyzer->xConfig.pIFName);
		goto finished;
	}

	pAnalyzer->bStop = FTM_FALSE;

	FTM_LOCK_set(pAnalyzer->pPCAPLock);

	while(!pAnalyzer->bStop)
	{

		FTM_LOCK_set(pAnalyzer->pPCAPLock);
		if (pAnalyzer->bStop)
		{
			break;
		}

		FTM_PCAP_start(pAnalyzer->pPCAP, 1000);
	}

	FTM_LOCK_reset(pAnalyzer->pPCAPLock);

	FTM_PCAP_close(pAnalyzer->pPCAP);

	INFO("%s stopped.", pAnalyzer->pName);

finished:

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
		FTM_CHAR		pMAC[64]; 
		FTM_CHAR		pLocalIP[32];
		FTM_CHAR		pHashData[1024];
		FTM_UINT32		ulHashDataLen = 0;
		FTM_CHAR		pHashValue[64];
		FTM_CHAR		pOptions[512];
		FTM_UINT32		ulOptionLen = 0;
		FTM_UINT32		ulReplyCount = 0;
		FTM_UINT32		ulTime;

		INFO("CCTV[%s] : Start status analysis!", pID);

		memset(pHashData, 0, sizeof(pHashData));
		memset(pHashValue, 0, sizeof(pHashValue));

		FTM_getLocalIP(pLocalIP,sizeof(pLocalIP));

		ulOptionLen += sprintf(&pOptions[ulOptionLen], "src host %s", pCCTV->xConfig.pIP);
		ulOptionLen += sprintf(&pOptions[ulOptionLen], " && dst host %s", pLocalIP);
		ulOptionLen += sprintf(&pOptions[ulOptionLen], " && (ip proto 1");

		if (pAnalyzer->xConfig.ulPortCount != 0)
		{
			ulOptionLen += sprintf(&pOptions[ulOptionLen], " || (ip proto 6 && (");
			for(FTM_UINT32	i = 0 ; i < pAnalyzer->xConfig.ulPortCount ; i++)
			{
				if (i == 0)
				{
					ulOptionLen += sprintf(&pOptions[ulOptionLen], "src port %d", pAnalyzer->xConfig.pPortList[i]);
				}
				else
				{
					ulOptionLen += sprintf(&pOptions[ulOptionLen], " || src port %d", pAnalyzer->xConfig.pPortList[i]);
				}
			}
			ulOptionLen += sprintf(&pOptions[ulOptionLen], " ))");
		}
		ulOptionLen += sprintf(&pOptions[ulOptionLen], ")");
		FTM_PCAP_setFilter(pAnalyzer->pPCAP, pOptions);

		FTM_PING_check(pCCTV->xConfig.pIP, &ulReplyCount);
		
		FTM_ANALYZER_PCAP_start(pAnalyzer);

		FTM_TIME_getCurrentSecs(&ulTime);

		if(ulReplyCount == 0)
		{ 
			INFO("CCTV[%s] : No response!", pCCTV->xConfig.pID);
			if (pCCTV->xConfig.xStat != FTM_CCTV_STAT_UNREGISTERED)
			{
				FTM_CATCHB_setCCTVStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_UNUSED, ulTime, NULL);
			}

			FTM_ANALYZER_PCAP_stop(pAnalyzer);
		}
		else
		{
			FTM_UINT32	j;
			FTM_SCORE	xScore;

			memset(&xScore, 0, sizeof(xScore));

			FTM_ARP_parsing(pCCTV->xConfig.pIP, pMAC); 
			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen , "[ip : %s, mac : %s]", pCCTV->xConfig.pIP, pMAC);
			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, " [port :");


			//FTM_PING_check(pCCTV->xConfig.pIP, &ulReplyCount);

			pCCTV->ulPortCount = 0;
			for(j =0 ;j < pAnalyzer->xConfig.ulPortCount ; j++)
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
				usleep(100000);	
			}
			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, "]");

			usleep(1000000);	

			FTM_ANALYZER_PCAP_stop(pAnalyzer);

			FTM_ANALIZER_PCAP_calcScore(pAnalyzer, &xScore);
			ulHashDataLen += snprintf(&pHashData[ulHashDataLen], sizeof(pHashData) - ulHashDataLen, "[score : %.1f]", xScore.fValue);
			INFO("Hash : %s", pHashData);

			FTM_HASH_SHA1((FTM_UINT8_PTR)pHashData, ulHashDataLen, pHashValue, sizeof(pHashValue));

			if (strlen(pCCTV->xConfig.pHash) == 0)
			{
				INFO("CCTV[%s] : Hash has been created.", pCCTV->xConfig.pID);
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
						pAnalyzer->xConfig.xTest.ulErrorRate = 100 - pAnalyzer->xConfig.xTest.ulErrorRate;
					}
				}

				if(!bTestFailed && !strncmp(pCCTV->xConfig.pHash, pHashValue, sizeof(pHashValue)))
				{
					INFO("CCTV[%s] : Hash is normal.", pCCTV->xConfig.pID);
					FTM_CATCHB_setCCTVStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_NORMAL, ulTime, pHashValue);
				}
				else
				{
					INFO("CCTV[%s] : Hash is abnormal.", pCCTV->xConfig.pID);
					INFO("Original Hash : %s", pCCTV->xConfig.pHash);
					INFO(" Current Hash : %s", pHashValue);
					
					FTM_CATCHB_setCCTVStat(pAnalyzer->pCatchB, pCCTV->xConfig.pID, FTM_CCTV_STAT_ABNORMAL, ulTime, pHashValue);
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
				INFO("CCTV[%s] : Attached to the analyzer for analysis.", pSavedID);
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
			INFO("CCTV[%s] : Removed from the analyzer.", pSavedID);
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

FTM_RET	FTM_ANALYZER_PCAP_start
(
	FTM_ANALYZER_PTR	pAnalyzer
)
{
	ASSERT(pAnalyzer != NULL);

	return	FTM_LOCK_reset(pAnalyzer->pPCAPLock);
}

FTM_RET	FTM_ANALYZER_PCAP_stop
(
	FTM_ANALYZER_PTR	pAnalyzer
)
{
	ASSERT(pAnalyzer != NULL);

	return	FTM_PCAP_stop(pAnalyzer->pPCAP);
}

FTM_RET	FTM_ANALIZER_PCAP_calcScore
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_SCORE_PTR		pScore
)
{
	ASSERT(pAnalyzer != NULL);

	
	FTM_UINT32	i, j;

	INFO("Packet captured[%d]", pAnalyzer->pPCAP->ulCaptureCount); 

	for(i = 0 ; i < pAnalyzer->pPCAP->ulCaptureCount ; i++)
	{
		struct iphdr* 	pIPHdr1	= ((struct iphdr*)(pAnalyzer->pPCAP->ppCapturePackets[i] + 14));
		for(j = i+1 ; j < pAnalyzer->pPCAP->ulCaptureCount ; j++)
		{
			struct iphdr* 	pIPHdr2 	= ((struct iphdr*)(pAnalyzer->pPCAP->ppCapturePackets[j] + 14));
			
			if (pIPHdr1->protocol > pIPHdr2->protocol)
			{
				FTM_UINT8_PTR	pTemp;

				pTemp = pAnalyzer->pPCAP->ppCapturePackets[i];
				pAnalyzer->pPCAP->ppCapturePackets[i] = pAnalyzer->pPCAP->ppCapturePackets[j];
				pAnalyzer->pPCAP->ppCapturePackets[j] = pTemp;
			}
		}
	}


	for(i = 0 ; i < pAnalyzer->pPCAP->ulCaptureCount ; i++)
	{
		FTM_SCORE_processIP(pScore, pAnalyzer->pPCAP->ppCapturePackets[i], 64);
		FTM_SCORE_processTCP(pScore, pAnalyzer->pPCAP->ppCapturePackets[i], 64);
	}

	return	FTM_RET_OK;

}

