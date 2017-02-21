#include <string.h>
#include "ftm_mem.h"
#include "ftm_detector.h"
#include "ftm_message.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_mem.h"
#include "ftm_timer.h"
#include "ftm_cctv.h"
#include "ftm_cctv.h"

#undef	__MODULE__
#define	__MODULE__	"CATCHB"

FTM_RET	FTM_CATCHB_EVENT_checkNewCCTV
(
	FTM_TIMER_PTR pTimer, 
	FTM_VOID_PTR pData
);

FTM_VOID_PTR	FTM_CATCHB_process
(
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_CATCHB_onCheckNewCCTV
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CHECK_NEW_CCTV_PTR	pMsg
);

FTM_RET		FTM_CATCHB_foundNewCCTVInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CCTV_CONFIG_PTR	pConfig
);

FTM_RET		FTM_CATCHB_onFoundCCTVInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_FOUND_NEW_CCTV_IN_DB_PTR	pMsg
);

FTM_RET		FTM_CATCHB_removedCCTVInDB
(	
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID
);

FTM_RET		FTM_CATCHB_onRemovedCCTVInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_REMOVED_CCTV_IN_DB_PTR	pMsg
);

FTM_RET		FTM_CATCHB_onCCTVHashUpdated
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CCTV_HASH_UPDATED_PTR	pMsg
);

FTM_RET	FTM_CATCHB_CCTV_onSetStat
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CCTV_SET_STAT_PTR	pMsg
);

FTM_RET		FTM_CATCHB_onCCTVRegister
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CCTV_REGISTER_PTR	pMsg
);

FTM_BOOL	FTM_CATCHB_CCTV_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

FTM_RET	FTM_CATCHB_EVENT_checkNewSwitch
(
	FTM_TIMER_PTR pTimer, 
	FTM_VOID_PTR pData
);

FTM_RET	FTM_CATCHB_onCheckNewSwitch
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CHECK_NEW_SWITCH_PTR	pMsg
);

FTM_RET		FTM_CATCHB_foundNewSwitchInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_SWITCH_CONFIG_PTR	pConfig
);

FTM_RET		FTM_CATCHB_onFoundSwitchInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_FOUND_NEW_SWITCH_IN_DB_PTR	pMsg
);

FTM_RET		FTM_CATCHB_removedSwitchInDB
(	
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID
);

FTM_RET		FTM_CATCHB_onRemovedSwitchInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_REMOVED_SWITCH_IN_DB_PTR	pMsg
);

FTM_BOOL	FTM_CATCHB_SWITCH_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

FTM_RET	FTM_CATCHB_CONFIG_init
(
	FTM_CATCHB_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	sprintf(pConfig->xDB.pFileName, "./catchb.db");
	pConfig->xCCTV.ulUpdateInterval = 10000;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CATCHB_create
(
	FTM_CATCHB_PTR _PTR_ ppCatchB
)
{
	ASSERT(ppCatchB != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CATCHB_PTR	pCatchB;

	pCatchB = (FTM_CATCHB_PTR)FTM_MEM_malloc(sizeof(FTM_CATCHB));
	if (pCatchB == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create notifier!");	
		goto error;
	}

	xRet = FTM_CATCHB_CONFIG_init(&pCatchB->xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to init config!");
		goto error;
	}

	xRet = FTM_LIST_create(&pCatchB->pCCTVList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create cctv list!");
		goto error;
	}

	FTM_LIST_setSeeker(pCatchB->pCCTVList, FTM_CATCHB_CCTV_seeker);

	xRet = FTM_LIST_create(&pCatchB->pSwitchList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create switch list!");
		goto error;
	}

	FTM_LIST_setSeeker(pCatchB->pSwitchList, FTM_CATCHB_SWITCH_seeker);

	xRet = FTM_DB_create(&pCatchB->pDB);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create DB interface!");
		goto error;
	}

	TRACE("pCatchB = %08x, pDB = %08x\n", pCatchB, pCatchB->pDB);
	xRet = FTM_MSGQ_create(&pCatchB->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create message queue!");
		goto error;
   	}

	xRet = FTM_ANALYZER_create(pCatchB, &pCatchB->pAnalyzer);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create analyzer!");
		goto error;
	}

	xRet = FTM_DETECTOR_create(pCatchB, &pCatchB->pDetector);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create detector!");
		goto error;
	}

	xRet = FTM_EVENT_TIMER_MANAGER_create(&pCatchB->pEventManager);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create event timer manager!");
		goto error;
	}

	*ppCatchB = pCatchB;	

	return	xRet;

error:
	if (pCatchB != NULL)
	{
		if (pCatchB->pEventManager != NULL)
		{
			xRet = FTM_EVENT_TIMER_MANAGER_destroy(&pCatchB->pEventManager);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to destroy event timer manager!");
			}
		}

		if (pCatchB->pDetector != NULL)
		{
			FTM_DETECTOR_destroy(&pCatchB->pDetector);	
		}

		if (pCatchB->pAnalyzer != NULL)
		{
			FTM_ANALYZER_destroy(&pCatchB->pAnalyzer);	
		}

		if (pCatchB->pMsgQ != NULL)
		{
			FTM_MSGQ_destroy(&pCatchB->pMsgQ);	
		}

		if (pCatchB->pDB != NULL)
		{
			FTM_DB_destroy(&pCatchB->pDB);
		}

		if (pCatchB->pSwitchList != NULL)
		{
			FTM_LIST_destroy(&pCatchB->pSwitchList);	
		}

		if (pCatchB->pCCTVList != NULL)
		{
			FTM_LIST_destroy(&pCatchB->pCCTVList);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_CATCHB_destroy
(
	FTM_CATCHB_PTR _PTR_ ppCatchB
)
{
	ASSERT(ppCatchB != NULL);
	ASSERT(*ppCatchB != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	if ((*ppCatchB)->pCCTVList != NULL)
	{
		FTM_CCTV_PTR	pCCTV = NULL;

		FTM_LIST_iteratorStart((*ppCatchB)->pCCTVList);
		while(FTM_LIST_iteratorNext((*ppCatchB)->pCCTVList, (FTM_VOID_PTR _PTR_)&pCCTV) == FTM_RET_OK)
		{
			FTM_MEM_free(pCCTV);
		}

		xRet = FTM_LIST_destroy(&(*ppCatchB)->pCCTVList);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to destroy cctv list!");
		}
	}

	if ((*ppCatchB)->pSwitchList != NULL)
	{
		FTM_SWITCH_PTR	pSwitch = NULL;

		FTM_LIST_iteratorStart((*ppCatchB)->pSwitchList);
		while(FTM_LIST_iteratorNext((*ppCatchB)->pSwitchList, (FTM_VOID_PTR _PTR_)&pSwitch) == FTM_RET_OK)
		{
			FTM_MEM_free(pSwitch);
		}

		xRet = FTM_LIST_destroy(&(*ppCatchB)->pSwitchList);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to destroy cctv list!");
		}
	}

	if ((*ppCatchB)->pMsgQ != NULL)
	{
		xRet = FTM_MSGQ_destroy(&(*ppCatchB)->pMsgQ);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to destroy message queue!");
		}
	}

	if ((*ppCatchB)->pDB != NULL)
	{
		FTM_DB_destroy(&(*ppCatchB)->pDB);
	}

	if ((*ppCatchB)->pDetector != NULL)
	{
		FTM_DETECTOR_destroy(&(*ppCatchB)->pDetector);	
	}

	if ((*ppCatchB)->pAnalyzer != NULL)
	{
		FTM_ANALYZER_destroy(&(*ppCatchB)->pAnalyzer);	
	}

	if ((*ppCatchB)->pEventManager != NULL)
	{
		xRet = FTM_EVENT_TIMER_MANAGER_destroy(&(*ppCatchB)->pEventManager);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to destroy event timer manager!");
		}
	}

	FTM_MEM_free(*ppCatchB);

	*ppCatchB = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_CATCHB_start
(
	FTM_CATCHB_PTR	pCatchB
)
{
	ASSERT(pCatchB != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pCatchB->xThread != 0)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
		TRACE("The notifier is already running!");	
		return	xRet;
	}

	TRACE("pCatchB = %08x, pDB = %08x\n", pCatchB, pCatchB->pDB);
	if (pthread_create(&pCatchB->xThread, NULL, FTM_CATCHB_process, (FTM_VOID_PTR)pCatchB) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		TRACE("Failed to start notifier!");
	}

	return	xRet;
}

FTM_RET	FTM_CATCHB_stop
(
	FTM_CATCHB_PTR	pCatchB
)
{
	ASSERT(pCatchB != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pCatchB->xThread != 0)
	{
		pCatchB->bStop = FTM_TRUE;	
		pthread_join(pCatchB->xThread, NULL);

		pCatchB->xThread = 0;
	}

	return	xRet;
}

FTM_RET	FTM_CATCHB_waitingForFinished
(
	FTM_CATCHB_PTR	pCatchB
)
{
	ASSERT(pCatchB != NULL);

	pthread_join(pCatchB->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CATCHB_initProcess
(	
	FTM_CATCHB_PTR	pCatchB
)
{
	ASSERT(pCatchB != NULL);

	FTM_RET	xRet;
	FTM_BOOL	bExist;

	TRACE("pCatchB = %08x, pDB = %08x\n", pCatchB, pCatchB->pDB);
	xRet = FTM_DB_open(pCatchB->pDB, pCatchB->xConfig.xDB.pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to open DB!");	
		return	xRet;
	}

	xRet = FTM_DB_SWITCH_isTableExist(pCatchB->pDB, &bExist);
	if (bExist)
	{
		FTM_UINT32	ulCount;

		xRet = FTM_DB_SWITCH_count(pCatchB->pDB, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get SWITCH count!");	
		}
		else if (ulCount != 0)
		{
			FTM_SWITCH_CONFIG_PTR	pConfigArray = (FTM_SWITCH_CONFIG_PTR)FTM_MEM_calloc(sizeof(FTM_SWITCH_CONFIG), ulCount);
			if (pConfigArray == NULL)
			{
				xRet = FTM_RET_NOT_ENOUGH_MEMORY;
				ERROR(xRet, "Failed to alloc SWITCH config array!");
			}
			else
			{
				FTM_UINT32	i;

				xRet = FTM_DB_SWITCH_getList(pCatchB->pDB, pConfigArray, ulCount, &ulCount);
				if (xRet != FTM_RET_OK)
				{
					ERROR(xRet, "Failed to get SWITCH list!");
				}
				else
				{
					for(i = 0 ; i < ulCount ; i++)
					{
						xRet = FTM_CATCHB_foundNewSwitchInDB(pCatchB, &pConfigArray[i]);
						if (xRet != FTM_RET_OK)
						{
							ERROR(xRet, "Failed to send message to CatchB!");	
						}

					}
				}	
				FTM_MEM_free(pConfigArray);	
			}
		}
	}
	else
	{
		xRet = FTM_DB_SWITCH_createTable(pCatchB->pDB);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to create SWITCH table!");	
		}	
	}

	xRet = FTM_DB_CCTV_isTableExist(pCatchB->pDB, &bExist);
	if (bExist)
	{
		FTM_UINT32	ulCount;

		xRet = FTM_DB_CCTV_count(pCatchB->pDB, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get CCTV count!");	
		}
		else if (ulCount != 0)
		{
			FTM_CCTV_CONFIG_PTR	pConfigArray = (FTM_CCTV_CONFIG_PTR)FTM_MEM_calloc(sizeof(FTM_CCTV_CONFIG), ulCount);
			if (pConfigArray == NULL)
			{
				xRet = FTM_RET_NOT_ENOUGH_MEMORY;
				ERROR(xRet, "Failed to alloc CCTV config array!");
			}
			else
			{
				FTM_UINT32	i;

				xRet = FTM_DB_CCTV_getList(pCatchB->pDB, pConfigArray, ulCount, &ulCount);
				if (xRet != FTM_RET_OK)
				{
					ERROR(xRet, "Failed to get CCTV list!");
				}
				else
				{
					for(i = 0 ; i < ulCount ; i++)
					{
						xRet = FTM_CATCHB_foundNewCCTVInDB(pCatchB, &pConfigArray[i]);
						if (xRet != FTM_RET_OK)
						{
							ERROR(xRet, "Failed to send message to CatchB!");	
						}

					}
				}	
				FTM_MEM_free(pConfigArray);	
			}
		}
	}
	else
	{
		xRet = FTM_DB_CCTV_createTable(pCatchB->pDB);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to create CCTV table!");	
		}	
	}

	xRet = FTM_DB_ALARM_isTableExist(pCatchB->pDB, &bExist);
	if (bExist)
	{
	}
	else
	{
		FTM_DB_ALARM_createTable(pCatchB->pDB);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to create alarm table!");	
		}	
	}

	xRet = FTM_DB_LOG_isTableExist(pCatchB->pDB, &bExist);
	if (bExist)
	{
	}
	else
	{
		xRet = FTM_DB_LOG_createTable(pCatchB->pDB);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to create log table!");	
		}	
	}

	xRet = FTM_DB_SWITCH_isTableExist(pCatchB->pDB, &bExist);
	if (bExist)
	{
	}
	else
	{
		xRet = FTM_DB_SWITCH_createTable(pCatchB->pDB);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to create switch table!");	
		}	
	}

	xRet = FTM_DB_DENY_isTableExist(pCatchB->pDB, &bExist);
	if (bExist)
	{
	}
	else
	{
		xRet = FTM_DB_DENY_createTable(pCatchB->pDB);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to create deny table!");	
		}
	}

	return	xRet;
}

FTM_VOID_PTR	FTM_CATCHB_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTM_RET	xRet;
	FTM_CATCHB_PTR	pCatchB = (FTM_CATCHB_PTR)pData;

	TRACE("pCatchB = %08x, pDB = %08x\n", pCatchB, pCatchB->pDB);
	xRet = FTM_CATCHB_initProcess(pCatchB);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to init catchb!");
	}

	xRet = FTM_ANALYZER_start(pCatchB->pAnalyzer);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to start analyzer!");
	}

	xRet = FTM_DETECTOR_start(pCatchB->pDetector);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to start detector!");
	}

	xRet = FTM_EVENT_TIMER_MANAGER_start(pCatchB->pEventManager);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to start event manager!");
	}

	xRet = FTM_EVENT_TIMER_MANAGER_add(pCatchB->pEventManager, FTM_EVENT_TIMER_TYPE_REPEAT, pCatchB->xConfig.xCCTV.ulUpdateInterval, FTM_CATCHB_EVENT_checkNewCCTV, pCatchB, NULL);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add event!");
	}

	TRACE_ENTRY();
	pCatchB->bStop = FTM_FALSE;
	while(!pCatchB->bStop)
	{
		FTM_MSG_PTR	pRcvdMsg;

		
		xRet = 	FTM_MSGQ_timedPop(pCatchB->pMsgQ, 1000, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if(xRet == FTM_RET_OK)
		{
			switch(pRcvdMsg->xType)
			{
			case	FTM_MSG_TYPE_CHECK_NEW_SWITCH:
				{
					xRet = FTM_CATCHB_onCheckNewSwitch(pCatchB, (FTM_MSG_CHECK_NEW_SWITCH_PTR)pRcvdMsg);
				}
				break;

			case	FTM_MSG_TYPE_FOUND_NEW_SWITCH_IN_DB:
				{
					xRet = FTM_CATCHB_onFoundSwitchInDB(pCatchB, (FTM_MSG_FOUND_NEW_SWITCH_IN_DB_PTR)pRcvdMsg);
				}
				break;

			case	FTM_MSG_TYPE_REMOVED_SWITCH_IN_DB:
				{
					xRet = FTM_CATCHB_onRemovedSwitchInDB(pCatchB, (FTM_MSG_REMOVED_SWITCH_IN_DB_PTR)pRcvdMsg);
				}
				break;

			case	FTM_MSG_TYPE_CHECK_NEW_CCTV:
				{
					xRet = FTM_CATCHB_onCheckNewCCTV(pCatchB, (FTM_MSG_CHECK_NEW_CCTV_PTR)pRcvdMsg);
				}
				break;

			case	FTM_MSG_TYPE_FOUND_NEW_CCTV_IN_DB:
				{
					xRet = FTM_CATCHB_onFoundCCTVInDB(pCatchB, (FTM_MSG_FOUND_NEW_CCTV_IN_DB_PTR)pRcvdMsg);
				}
				break;

			case	FTM_MSG_TYPE_REMOVED_CCTV_IN_DB:
				{
					xRet = FTM_CATCHB_onRemovedCCTVInDB(pCatchB, (FTM_MSG_REMOVED_CCTV_IN_DB_PTR)pRcvdMsg);
				}
				break;

			case	FTM_MSG_TYPE_CCTV_HASH_UPDATED:
				{
					xRet = FTM_CATCHB_onCCTVHashUpdated(pCatchB, (FTM_MSG_CCTV_HASH_UPDATED_PTR)pRcvdMsg);
				}
				break;

			case	FTM_MSG_TYPE_CCTV_SET_STAT:
				{
					xRet = FTM_CATCHB_CCTV_onSetStat(pCatchB, (FTM_MSG_CCTV_SET_STAT_PTR)pRcvdMsg);
				}
				break;

			case	FTM_MSG_TYPE_CCTV_REGISTER:
				{
					xRet = FTM_CATCHB_onCCTVRegister(pCatchB, (FTM_MSG_CCTV_REGISTER_PTR)pRcvdMsg);
				}
				break;

			default:
				{
					ERROR(FTM_RET_NOT_SUPPORTED_MESSAGE, "Unknown message[%x]", pRcvdMsg->xType);
				}
			}

			FTM_MEM_free(pRcvdMsg);
		}
	}

	FTM_EVENT_TIMER_MANAGER_stop(pCatchB->pEventManager);
	FTM_DETECTOR_stop(pCatchB->pDetector);
	FTM_ANALYZER_stop(pCatchB->pAnalyzer);
	
	return	0;
}


FTM_RET	FTM_CATCHB_EVENT_checkNewCCTV
(
	FTM_TIMER_PTR pTimer, 
	FTM_VOID_PTR pData
)
{
	ASSERT(pTimer != NULL);
	ASSERT(pData != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CATCHB_PTR	pCatchB = (FTM_CATCHB_PTR)pData;
	FTM_MSG_CHECK_NEW_CCTV_PTR	pMsg  = (FTM_MSG_CHECK_NEW_CCTV_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_CHECK_NEW_CCTV));

	pMsg->xHead.xType = FTM_MSG_TYPE_CHECK_NEW_CCTV;
	pMsg->xHead.ulLen = sizeof(FTM_MSG_CHECK_NEW_CCTV);

	xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to push message!");
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET	FTM_CATCHB_onCheckNewCCTV
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CHECK_NEW_CCTV_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	FTM_RET			xRet;
	FTM_UINT32		ulCount;

	TRACE("Check New CCTV!");
	xRet = FTM_DB_CCTV_count(pCatchB->pDB, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		if (ulCount != 0)
		{
			FTM_UINT32		i;
			FTM_CCTV_PTR	pCCTV = NULL;
			FTM_CCTV_CONFIG_PTR	pConfigArray = NULL;

			pConfigArray = (FTM_CCTV_CONFIG_PTR)FTM_MEM_calloc(sizeof(FTM_CCTV_CONFIG), ulCount);
			if (pConfigArray != NULL)
			{
				xRet = FTM_DB_CCTV_getList(pCatchB->pDB, pConfigArray, ulCount, &ulCount);
				if (xRet == FTM_RET_OK)
				{
					FTM_LIST_iteratorStart(pCatchB->pCCTVList);
					while(FTM_LIST_iteratorNext(pCatchB->pCCTVList, (FTM_VOID_PTR _PTR_)&pCCTV) == FTM_RET_OK)
					{
						for(i = 0 ; i < ulCount ; i++)
						{
							if (strcmp(pCCTV->xConfig.pID, pConfigArray[i].pID) == 0)
							{
								break;	
							}
						}

						if (i == ulCount)
						{
							FTM_CATCHB_removedCCTVInDB(	pCatchB, pCCTV->xConfig.pID);
						}
					}


					for(i = 0 ; i < ulCount ; i++)
					{
						xRet = FTM_LIST_get(pCatchB->pCCTVList, pConfigArray[i].pID, (FTM_VOID_PTR _PTR_)&pCCTV);
						if (xRet != FTM_RET_OK)
						{
							FTM_CATCHB_foundNewCCTVInDB(pCatchB, &pConfigArray[i]);
						}
					}
				}
				else
				{
					ERROR(xRet, "Failed to get CCTV list from DB!");
				}

				FTM_MEM_free(pConfigArray);	
				pConfigArray = NULL;
			}
			else
			{
				ERROR(xRet, "Failed to alloc CCTV array!");
			}
		}
	}
	else
	{
		ERROR(xRet, "Failed to get CCTV count from DB!");
	}

	return	xRet;
}


FTM_RET		FTM_CATCHB_foundNewCCTVInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CCTV_CONFIG_PTR	pConfig
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;	
	FTM_MSG_FOUND_NEW_CCTV_IN_DB_PTR	pMsg  = (FTM_MSG_FOUND_NEW_CCTV_IN_DB_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_FOUND_NEW_CCTV_IN_DB));

	pMsg->xHead.xType = FTM_MSG_TYPE_FOUND_NEW_CCTV_IN_DB;
	pMsg->xHead.ulLen = sizeof(FTM_MSG_FOUND_NEW_CCTV_IN_DB);
	memcpy(&pMsg->xConfig, pConfig, sizeof(FTM_CCTV_CONFIG));

	xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to push message!");
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET		FTM_CATCHB_onFoundCCTVInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_FOUND_NEW_CCTV_IN_DB_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET			xRet;
	FTM_CCTV_PTR	pCCTV;

	xRet = FTM_CCTV_create(&pMsg->xConfig, &pCCTV);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create CCTV!");
	}
	else
	{
		xRet = FTM_LIST_append(pCatchB->pCCTVList, pCCTV);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to append list!");
			FTM_CCTV_destroy(&pCCTV);
		}
		else
		{
			xRet = FTM_ANALYZER_CCTV_add(pCatchB->pAnalyzer, pCCTV->xConfig.pID);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet,	"Failed to append CCTV to scheduler!");
			}
		}
	}

	return	xRet;
}

FTM_RET		FTM_CATCHB_removedCCTVInDB
(	
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet;	
	FTM_MSG_REMOVED_CCTV_IN_DB_PTR pMsg  = (FTM_MSG_REMOVED_CCTV_IN_DB_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_FOUND_NEW_CCTV_IN_DB));

	pMsg->xHead.xType = FTM_MSG_TYPE_REMOVED_CCTV_IN_DB;
	pMsg->xHead.ulLen = sizeof(FTM_MSG_REMOVED_CCTV_IN_DB);
	strncpy(pMsg->pID, pID, sizeof(pMsg->pID) - 1);

	xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to push message!");
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET		FTM_CATCHB_onRemovedCCTVInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_REMOVED_CCTV_IN_DB_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pMsg != NULL);
	FTM_RET			xRet;
	FTM_CCTV_PTR	pCCTV;

	TRACE("On Removed CCTV In DB : %s", pMsg->pID);

	xRet = FTM_LIST_get(pCatchB->pCCTVList, pMsg->pID, (FTM_VOID_PTR _PTR_)&pCCTV);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV!");
		return	xRet;	
	}

	xRet = FTM_LIST_remove(pCatchB->pCCTVList, pCCTV);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to remove CCTV!");
		return	xRet;
	}

	xRet = FTM_ANALYZER_CCTV_delete(pCatchB->pAnalyzer, pCCTV->xConfig.pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet,	"Failed to delete CCTV from analyzer!");
	}

	FTM_CCTV_destroy(&pCCTV);

	return	xRet;
}

FTM_BOOL	FTM_CATCHB_CCTV_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	FTM_CCTV_PTR	pCCTV = (FTM_CCTV_PTR)pElement;
	FTM_CHAR_PTR	pID	  = (FTM_CHAR_PTR)pIndicator;

	return	(strcmp(pCCTV->xConfig.pID, pID) == 0);
}

FTM_RET	FTM_CATCHB_CCTV_get
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_PTR _PTR_ ppCCTV
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pID != NULL);
	ASSERT(ppCCTV != NULL);

	return	FTM_LIST_get(pCatchB->pCCTVList, pID, (FTM_VOID_PTR _PTR_)ppCCTV);
}

FTM_RET	FTM_CATCHB_CCTV_count
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pCount
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pCount != NULL);

	return	FTM_LIST_count(pCatchB->pCCTVList, pCount);
}

FTM_RET	FTM_CATCHB_CCTV_hashUpdated
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pID != NULL);
	ASSERT(pHash != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_MSG_CCTV_HASH_UPDATED_PTR	pMsg;

	pMsg = (FTM_MSG_CCTV_HASH_UPDATED_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_CCTV_HASH_UPDATED));
	if (pMsg == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create message!");
	}
	else
	{
		pMsg->xHead.xType = FTM_MSG_TYPE_CCTV_HASH_UPDATED;
		pMsg->xHead.ulLen = sizeof(FTM_MSG_CCTV_HASH_UPDATED);

		strncpy(pMsg->pID, pID, FTM_ID_LEN);
		strncpy(pMsg->pHash, pHash, FTM_HASH_LEN);

		xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to send message to CatchB!");
			FTM_MEM_free(pMsg);	
		}
	}

	return	xRet;
}

FTM_RET		FTM_CATCHB_onCCTVHashUpdated
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CCTV_HASH_UPDATED_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CCTV_PTR	pCCTV;

	TRACE("On CCTV Hash Updated : %s[%s]", pMsg->pID, pMsg->pHash);

	xRet = FTM_LIST_get(pCatchB->pCCTVList, pMsg->pID, (FTM_VOID_PTR _PTR_)&pCCTV);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV[%s]", pMsg->pID);	
	}
	else
	{
		strncpy(pCCTV->xConfig.pHash, pMsg->pHash, FTM_HASH_LEN);

		xRet = FTM_DB_CCTV_hashUpdated(pCatchB->pDB, pMsg->pID, pMsg->pHash);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to update CCTV[%s] hash", pMsg->pID);	
		}
		else
		{
		
			xRet = FTM_ANALYZER_CCTV_add(pCatchB->pAnalyzer, pCCTV->xConfig.pID);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to add CCTV[%s] to analyzer!", pCCTV->xConfig.pID);	
			}
		}
	}
	
	return	xRet;
}

FTM_RET		FTM_CATCHB_CCTV_setStat
(	
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_STAT	xStat
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet;	
	FTM_MSG_CCTV_SET_STAT_PTR	pMsg  = (FTM_MSG_CCTV_SET_STAT_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_CCTV_SET_STAT));

	pMsg->xHead.xType = FTM_MSG_TYPE_CCTV_SET_STAT;
	pMsg->xHead.ulLen = sizeof(FTM_MSG_CCTV_SET_STAT);
	strncpy(pMsg->pID, pID, sizeof(pMsg->pID) - 1);
	pMsg->xStat = xStat;

	xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to push message!");
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET	FTM_CATCHB_CCTV_onSetStat
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CCTV_SET_STAT_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CCTV_PTR	pCCTV;

	xRet = FTM_LIST_get(pCatchB->pCCTVList, pMsg->pID, (FTM_VOID_PTR _PTR_)&pCCTV);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV[%s]", pMsg->pID);	
	}
	else
	{
		if (pMsg->xStat == FTM_CCTV_STAT_NORMAL)
		{
			if (pCCTV->xConfig.xStat == FTM_CCTV_STAT_ABNORMAL)
			{
				FTM_DETECTOR_setControl(pCatchB->pDetector, "1.1.1.1", pCCTV->xConfig.pIP, FTM_FALSE);
			}
		}
		else if (pMsg->xStat == FTM_CCTV_STAT_ABNORMAL)
		{
			if (pCCTV->xConfig.xStat == FTM_CCTV_STAT_NORMAL)
			{
				FTM_DETECTOR_setControl(pCatchB->pDetector, "1.1.1.1", pCCTV->xConfig.pIP, FTM_TRUE);
			}
		}

		pCCTV->xConfig.xStat = pMsg->xStat;

		xRet = FTM_DB_LOG_insert(pCatchB->pDB, pCCTV->xConfig.pID, pCCTV->xConfig.pIP, pCCTV->xConfig.pHash, "", "", pCCTV->xConfig.xStat);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to set log");	
		}
	}

	return	xRet;
}

FTM_RET	FTM_CATCHB_CCTV_register
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pID != NULL);
	ASSERT(pHash != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_MSG_CCTV_REGISTER_PTR	pMsg;

	pMsg = (FTM_MSG_CCTV_REGISTER_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_CCTV_REGISTER));
	if (pMsg == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create message!");
	}
	else
	{
		pMsg->xHead.xType = FTM_MSG_TYPE_CCTV_REGISTER;
		pMsg->xHead.ulLen = sizeof(FTM_MSG_CCTV_REGISTER);

		strncpy(pMsg->pID, pID, FTM_ID_LEN);
		strncpy(pMsg->pHash, pHash, FTM_HASH_LEN);

		xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to send message to CatchB!");
			FTM_MEM_free(pMsg);	
		}
	}

	return	xRet;
}

FTM_RET		FTM_CATCHB_onCCTVRegister
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CCTV_REGISTER_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CCTV_PTR	pCCTV;

	TRACE("On CCTV register : %s[%s]", pMsg->pID, pMsg->pHash);

	xRet = FTM_LIST_get(pCatchB->pCCTVList, pMsg->pID, (FTM_VOID_PTR _PTR_)&pCCTV);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV[%s]", pMsg->pID);	
	}
	else
	{
		strncpy(pCCTV->xConfig.pHash, pMsg->pHash, FTM_HASH_LEN);

		xRet = FTM_DB_CCTV_hashUpdated(pCatchB->pDB, pMsg->pID, pMsg->pHash);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to update CCTV[%s] hash", pMsg->pID);	
		}
	}
	
	return	xRet;
}

FTM_RET		FTM_CATCHB_foundNewSwitchInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_SWITCH_CONFIG_PTR	pConfig
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;	
	FTM_MSG_FOUND_NEW_SWITCH_IN_DB_PTR	pMsg  = (FTM_MSG_FOUND_NEW_SWITCH_IN_DB_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_FOUND_NEW_SWITCH_IN_DB));

	pMsg->xHead.xType = FTM_MSG_TYPE_FOUND_NEW_SWITCH_IN_DB;
	pMsg->xHead.ulLen = sizeof(FTM_MSG_FOUND_NEW_SWITCH_IN_DB);
	memcpy(&pMsg->xConfig, pConfig, sizeof(FTM_SWITCH_CONFIG));

	xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to push message!");
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET		FTM_CATCHB_onFoundSwitchInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_FOUND_NEW_SWITCH_IN_DB_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET			xRet;
	FTM_SWITCH_PTR	pSwitch;

	xRet = FTM_SWITCH_create(&pMsg->xConfig, &pSwitch);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create SWITCH!");
	}
	else
	{
		xRet = FTM_LIST_append(pCatchB->pSwitchList, pSwitch);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to append list!");
			FTM_SWITCH_destroy(&pSwitch);
		}
	}

	return	xRet;
}

FTM_RET		FTM_CATCHB_removedSwitchInDB
(	
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet;	
	FTM_MSG_REMOVED_SWITCH_IN_DB_PTR pMsg  = (FTM_MSG_REMOVED_SWITCH_IN_DB_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_FOUND_NEW_SWITCH_IN_DB));

	pMsg->xHead.xType = FTM_MSG_TYPE_REMOVED_SWITCH_IN_DB;
	pMsg->xHead.ulLen = sizeof(FTM_MSG_REMOVED_SWITCH_IN_DB);
	strncpy(pMsg->pID, pID, sizeof(pMsg->pID) - 1);

	xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to push message!");
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET		FTM_CATCHB_onRemovedSwitchInDB
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_REMOVED_SWITCH_IN_DB_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pMsg != NULL);
	FTM_RET			xRet;
	FTM_SWITCH_PTR	pSwitch;

	TRACE("On Removed SWITCH In DB : %s", pMsg->pID);

	xRet = FTM_LIST_get(pCatchB->pSwitchList, pMsg->pID, (FTM_VOID_PTR _PTR_)&pSwitch);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get SWITCH!");
		return	xRet;	
	}

	xRet = FTM_LIST_remove(pCatchB->pSwitchList, pSwitch);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to remove SWITCH!");
		return	xRet;
	}

	FTM_SWITCH_destroy(&pSwitch);

	return	xRet;
}
FTM_BOOL	FTM_CATCHB_SWITCH_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	FTM_SWITCH_PTR	pSwitch = (FTM_SWITCH_PTR)pElement;
	FTM_CHAR_PTR	pID	  = (FTM_CHAR_PTR)pIndicator;

	return	(strcmp(pSwitch->xConfig.pID, pID) == 0);
}

FTM_RET	FTM_CATCHB_SWITCH_get
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_PTR _PTR_ ppSwitch
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pID != NULL);
	ASSERT(ppSwitch != NULL);

	return	FTM_LIST_get(pCatchB->pSwitchList, pID, (FTM_VOID_PTR _PTR_)ppSwitch);
}

FTM_RET	FTM_CATCHB_SWITCH_count
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pCount
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(pCount != NULL);

	return	FTM_LIST_count(pCatchB->pSwitchList, pCount);
}

FTM_RET	FTM_CATCHB_EVENT_checkNewSwitch
(
	FTM_TIMER_PTR pTimer, 
	FTM_VOID_PTR pData
)
{
	ASSERT(pTimer != NULL);
	ASSERT(pData != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CATCHB_PTR	pCatchB = (FTM_CATCHB_PTR)pData;
	FTM_MSG_CHECK_NEW_SWITCH_PTR	pMsg  = (FTM_MSG_CHECK_NEW_SWITCH_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_CHECK_NEW_SWITCH));

	pMsg->xHead.xType = FTM_MSG_TYPE_CHECK_NEW_SWITCH;
	pMsg->xHead.ulLen = sizeof(FTM_MSG_CHECK_NEW_SWITCH);

	xRet = FTM_MSGQ_push(pCatchB->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to push message!");
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}

FTM_RET	FTM_CATCHB_onCheckNewSwitch
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_MSG_CHECK_NEW_SWITCH_PTR	pMsg
)
{
	ASSERT(pCatchB != NULL);
	FTM_RET			xRet;
	FTM_UINT32		ulCount;

	TRACE("Check New Switch!");
	xRet = FTM_DB_SWITCH_count(pCatchB->pDB, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		if (ulCount != 0)
		{
			FTM_UINT32		i;
			FTM_SWITCH_PTR	pSwitch = NULL;
			FTM_SWITCH_CONFIG_PTR	pConfigArray = NULL;

			pConfigArray = (FTM_SWITCH_CONFIG_PTR)FTM_MEM_calloc(sizeof(FTM_SWITCH_CONFIG), ulCount);
			if (pConfigArray != NULL)
			{
				xRet = FTM_DB_SWITCH_getList(pCatchB->pDB, pConfigArray, ulCount, &ulCount);
				if (xRet == FTM_RET_OK)
				{
					FTM_LIST_iteratorStart(pCatchB->pSwitchList);
					while(FTM_LIST_iteratorNext(pCatchB->pSwitchList, (FTM_VOID_PTR _PTR_)&pSwitch) == FTM_RET_OK)
					{
						for(i = 0 ; i < ulCount ; i++)
						{
							if (strcmp(pSwitch->xConfig.pID, pConfigArray[i].pID) == 0)
							{
								break;	
							}
						}

						if (i == ulCount)
						{
							FTM_CATCHB_removedSwitchInDB(	pCatchB, pSwitch->xConfig.pID);
						}
					}


					for(i = 0 ; i < ulCount ; i++)
					{
						xRet = FTM_LIST_get(pCatchB->pSwitchList, pConfigArray[i].pID, (FTM_VOID_PTR _PTR_)&pSwitch);
						if (xRet != FTM_RET_OK)
						{
							FTM_CATCHB_foundNewSwitchInDB(pCatchB, &pConfigArray[i]);
						}
					}
				}
				else
				{
					ERROR(xRet, "Failed to get Switch list from DB!");
				}

				FTM_MEM_free(pConfigArray);	
				pConfigArray = NULL;
			}
			else
			{
				ERROR(xRet, "Failed to alloc Switch array!");
			}
		}
	}
	else
	{
		ERROR(xRet, "Failed to get Switch count from DB!");
	}

	return	xRet;
}

