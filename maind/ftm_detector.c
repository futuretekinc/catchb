#include "ftm_mem.h"
#include "ftm_detector.h"
#include "ftm_message.h"
#include "ftm_trace.h"

#include <common_libssh.h>

    /*
       0. (tb_ck_switch_detection_info) select

       1. (tb_ck_deny_ip) sql switch ip, id, password read
       2. ssh process(ssh connection, ssh command write)
       3. (tb_ck_switch_detection_info) sql detection insert
    */

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

	xRet = FTM_MSGQ_create(&pDetector->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create message queue!");
		goto error;
   	}
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

	if ((*ppDetector)->pMsgQ != NULL)
	{
		xRet = FTM_MSGQ_destroy(&(*ppDetector)->pMsgQ);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to destroy message queue!\n");
		}
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
		TRACE("The notifier is already running!\n");	
		return	xRet;
	}

	if (pthread_create(&pDetector->xThread, NULL, FTM_DETECTOR_process, (FTM_VOID_PTR)pDetector) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		TRACE("Failed to start notifier!");
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

	pDetector->bStop = FTM_FALSE;
	while(!pDetector->bStop)
	{
		FTM_MSG_PTR	pRcvdMsg;

		xRet = 	FTM_MSGQ_timedPop(pDetector->pMsgQ, 1000, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if(xRet == FTM_RET_OK)
		{
			if (pRcvdMsg->xType == FTM_MSG_TYPE_IP_DETECTED)
			{
#if 0
				FTM_CCTV	xCCTVInfo;
				FTM_MSG_IP_DETECTED_PTR pMsg = (FTM_MSG_IP_DETECTED_PTR)pRcvdMsg;

				xRet = FTM_DB_CCTV_getUsingIP(pDetector->pDB, pMsg->pIP, &xCCTVInfo);
				if (xRet == FTM_RET_OK)
				{
					if (pMsg->bAbnormal)
					{
						FTM_DETECTOR_setDeny(pDetector, pMsg->pIP, "");
					}
					else
					{
						FTM_DETECTOR_resetDeny(pDetector, pMsg->pIP);
					}
				}
#endif
			}

			FTM_MEM_free(pRcvdMsg);
		}
	}

	return	0;
}

FTM_RET	FTM_DETECTOR_setDeny
(
	FTM_DETECTOR_PTR	pDetector,
	FTM_CHAR_PTR		pIP,
	FTM_CHAR_PTR		pSwitchID
)
{
	ASSERT(pDetector != NULL);
	ASSERT(pIP != NULL);
	FTM_RET	xRet = FTM_RET_OK;

#if 0
	FTM_DENY	xDeny;

	xRet = FTM_DB_DENY_get(pDetector->pDB, pIP, &xDeny);
	if (xRet == FTM_RET_OBJECT_NOT_FOUND)
	{
		FTM_DB_DENY_insert(pDetector->pDB, pIP, 0, pSwitchID);
	}
#endif
	return	xRet;
}

FTM_RET	FTM_DETECTOR_resetDeny
(
	FTM_DETECTOR_PTR	pDetector,
	FTM_CHAR_PTR		pIP
)
{
	ASSERT(pDetector != NULL);
	ASSERT(pIP != NULL);
	FTM_RET	xRet = FTM_RET_OK;

#if 0
	FTM_DENY	xDeny;

	xRet = FTM_DB_DENY_get(pDetector->pDB, pIP, &xDeny);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTM_DB_DENY_delete(pDetector->pDB, pIP);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to delete info from DB!\n");
	}
#endif
	return	xRet;
}


