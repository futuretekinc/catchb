#include "ftm_mem.h"
#include "ftm_catchb.h"
#include "ftm_detector.h"
#include "ftm_message.h"
#include "ftm_trace.h"

#include <common_libssh.h>

#undef	__MODULE__
#define	__MODULE__	"detector"

FTM_BOOL	FTM_DETECTOR_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

FTM_RET	FTM_DETECTOR_setControl
(
	FTM_DETECTOR_PTR	pDetector,
	FTM_CHAR_PTR		pSwitchID,
	FTM_CHAR_PTR		pTargetIP,
	FTM_BOOL			bAllow
);

FTM_RET	FTM_DETECTOR_onSetControl
(
	FTM_DETECTOR_PTR	pDetector,
	FTM_MSG_SWITCH_CONTROL_PTR	pMsg
);

FTM_RET	FTM_DETECTOR_setDeny
(
	FTM_DETECTOR_PTR	pDetector,
	FTM_CHAR_PTR		pIP,
	FTM_CHAR_PTR		pSwitchID
);

FTM_RET	FTM_DETECTOR_resetDeny
(
	FTM_DETECTOR_PTR	pDetector,
	FTM_CHAR_PTR		pIP
);

static
FTM_VOID_PTR	FTM_DETECTOR_process
(
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_DETECTOR_create
(
	struct FTM_CATCHB_STRUCT _PTR_ pCatchB,
	FTM_DETECTOR_PTR _PTR_ ppDetector
)
{
	ASSERT(ppDetector != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_DETECTOR_PTR	pDetector;

	pDetector = (FTM_DETECTOR_PTR)FTM_MEM_malloc(sizeof(FTM_DETECTOR));
	if (pDetector == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create notifier!\n");	
		goto error;
	}

	strcpy(pDetector->pName, __MODULE__);

	xRet = FTM_LOCK_create(&pDetector->pLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create lock!");
		goto error;
	}

	xRet = FTM_LIST_create(&pDetector->pList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create CCTV list!");
		goto error;
	}
	FTM_LIST_setSeeker(pDetector->pList, FTM_DETECTOR_seeker);

	xRet = FTM_MSGQ_create(&pDetector->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create message queue!");
		goto error;
   	}
	pDetector->bStop = FTM_FALSE;
	pDetector->pCatchB = pCatchB;

	*ppDetector = pDetector;	

	return	xRet;

error:
	if (pDetector != NULL)
	{
		if (pDetector->pMsgQ != NULL)
		{
			FTM_MSGQ_destroy(&pDetector->pMsgQ);	
		}

		if (pDetector->pList != NULL)
		{
			FTM_LIST_destroy(&pDetector->pList);	
		}

		if (pDetector->pLock != NULL)
		{
			FTM_LOCK_destroy(&pDetector->pLock);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_DETECTOR_destroy
(
	FTM_DETECTOR_PTR _PTR_ ppDetector
)
{
	ASSERT(ppDetector != NULL);
	ASSERT(*ppDetector != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	FTM_DETECTOR_stop((*ppDetector));

	if ((*ppDetector)->pMsgQ != NULL)
	{
		xRet = FTM_MSGQ_destroy(&(*ppDetector)->pMsgQ);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to destroy message queue!\n");
		}
	}

	if ((*ppDetector)->pList != NULL)
	{
		FTM_CHAR_PTR	pID;

		FTM_LIST_iteratorStart((*ppDetector)->pList);
		while(FTM_LIST_iteratorNext((*ppDetector)->pList, (FTM_VOID_PTR _PTR_)&pID) == FTM_RET_OK)
		{
			FTM_MEM_free(pID);
		}
		FTM_LIST_destroy(&(*ppDetector)->pList);
	}

	if ((*ppDetector)->pLock != NULL)
	{
		FTM_LOCK_destroy(&(*ppDetector)->pLock);
	}

	FTM_MEM_free(*ppDetector);

	*ppDetector = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_DETECTOR_start
(
	FTM_DETECTOR_PTR	pDetector
)
{
	ASSERT(pDetector != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pDetector->xThread != 0)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
		INFO("The notifier is already running!\n");	
		return	xRet;
	}

	if (pthread_create(&pDetector->xThread, NULL, FTM_DETECTOR_process, (FTM_VOID_PTR)pDetector) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		INFO("Failed to start notifier!");
	}

	return	xRet;
}

FTM_RET	FTM_DETECTOR_stop
(
	FTM_DETECTOR_PTR	pDetector
)
{
	ASSERT(pDetector != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pDetector->xThread != 0)
	{
		FTM_MSGQ_pushQuit(pDetector->pMsgQ);

		pDetector->bStop = FTM_TRUE;	
		pthread_join(pDetector->xThread, NULL);

		pDetector->xThread = 0;
	}

	return	xRet;
}

FTM_VOID_PTR	FTM_DETECTOR_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTM_RET	xRet;
	FTM_DETECTOR_PTR	pDetector = (FTM_DETECTOR_PTR)pData;

	INFO("%s started.", pDetector->pName);

	pDetector->bStop = FTM_FALSE;
	while(!pDetector->bStop)
	{
		FTM_MSG_PTR	pRcvdMsg;

		xRet = 	FTM_MSGQ_timedPop(pDetector->pMsgQ, 10, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if(xRet == FTM_RET_OK)
		{
			switch(pRcvdMsg->xType)
			{
			case	FTM_MSG_TYPE_QUIT:
				{
					pDetector->bStop = FTM_TRUE;
				}
				break;

			case	FTM_MSG_TYPE_SWITCH_CONTROL:
				{
					FTM_DETECTOR_onSetControl(pDetector, (FTM_MSG_SWITCH_CONTROL_PTR)pRcvdMsg);
				}	
				break;

			default:
				{
					INFO("Unknown command[%x]", pRcvdMsg->xType);
				}
			}

			FTM_MEM_free(pRcvdMsg);
		}
	}

	INFO("%s stopped.", pDetector->pName);

	return	0;
}


FTM_RET	FTM_DETECTOR_setControl
(
	FTM_DETECTOR_PTR	pDetector,
	FTM_CHAR_PTR		pSwitchID,
	FTM_CHAR_PTR		pTargetIP,
	FTM_BOOL			bAllow
)
{
	ASSERT(pDetector != NULL);
	ASSERT(pSwitchID != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_MSG_SWITCH_CONTROL_PTR	pMsg;
	
	pMsg = (FTM_MSG_SWITCH_CONTROL_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_SWITCH_CONTROL));
	if (pMsg == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create message!");
	}
	else
	{
		pMsg->xHead.xType = FTM_MSG_TYPE_SWITCH_CONTROL;
		pMsg->xHead.ulLen = sizeof(FTM_MSG_SWITCH_CONTROL);
		strncpy(pMsg->pSwitchID, pSwitchID, sizeof(pMsg->pSwitchID) - 1);
		strncpy(pMsg->pTargetIP, pTargetIP, sizeof(pMsg->pTargetIP) - 1);
		pMsg->xPolicy	= (bAllow)?FTM_SWITCH_AC_POLICY_ALLOW:FTM_SWITCH_AC_POLICY_DENY;

		xRet = FTM_MSGQ_push(pDetector->pMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to send message!");
			FTM_MEM_free(pMsg);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_DETECTOR_onSetControl
(
	FTM_DETECTOR_PTR	pDetector,
	FTM_MSG_SWITCH_CONTROL_PTR	pMsg
)
{
	ASSERT(pDetector != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_SWITCH_PTR		pSwitch = NULL;
	FTM_SWITCH_AC_PTR	pAC;

	xRet = FTM_CATCHB_getSwitch(pDetector->pCatchB, pMsg->pSwitchID, &pSwitch);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get switch[%s]!", pMsg->pSwitchID);
		return	xRet;	
	}

	xRet = FTM_SWITCH_getAC(pSwitch, pMsg->pTargetIP, &pAC);
	if (xRet == FTM_RET_OBJECT_NOT_FOUND)
	{
		if (pMsg->xPolicy == FTM_SWITCH_AC_POLICY_DENY)
		{
			xRet = FTM_SWITCH_addAC(pSwitch, pMsg->pTargetIP, pMsg->xPolicy, &pAC);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to add AC to switch[%s]", pMsg->pSwitchID);
				return	xRet;	
			}
		}
	}
	else
	{
		if (pMsg->xPolicy == FTM_SWITCH_AC_POLICY_ALLOW)
		{
			FTM_SWITCH_deleteAC(pSwitch, pMsg->pTargetIP);
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTM_DETECTOR_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	(strcasecmp((FTM_CHAR_PTR)pElement, (FTM_CHAR_PTR)pIndicator) == 0);
}
