#ifndef	FTM_ANALYZER_H_
#define	FTM_ANALYZER_H_

#include <pthread.h>
#include "ftm_types.h"
#include "ftm_list.h"
#include "ftm_db.h"
#include "ftm_msgq.h"
#include "ftm_lock.h"
#include "cjson/cJSON.h"
#include "ftm_trace.h"
#include "ftm_pcap.h"

typedef	struct	FTM_ANALYZER_CONFIG_STRUCT
{
	FTM_CHAR		pIFName[32];
	FTM_UINT16		pPortList[32];
	FTM_UINT32		ulPortCount;
	FTM_UINT32		ulIPCheckInterval;
	struct
	{
		FTM_BOOL	bEnable;
		FTM_UINT32	ulErrorRate;
	}	xTest;
}	FTM_ANALYZER_CONFIG, _PTR_ FTM_ANALYZER_CONFIG_PTR;

FTM_RET	FTM_ANALYZER_CONFIG_setDefault
(
	FTM_ANALYZER_CONFIG_PTR	pConfig
);

FTM_RET	FTM_ANALYZER_CONFIG_load
(
	FTM_ANALYZER_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
);

FTM_RET	FTM_ANALYZER_CONFIG_save
(
	FTM_ANALYZER_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
);

FTM_RET	FTM_ANALYZER_CONFIG_show
(
	FTM_ANALYZER_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL	xLevel
);
/***********************************************************
 *
 ***********************************************************/
typedef	struct FTM_ANALYZER_STRUCT
{
	FTM_ANALYZER_CONFIG	xConfig;

	FTM_CHAR		pName[FTM_NAME_LEN+1];

	struct	FTM_CATCHB_STRUCT _PTR_	pCatchB;

	FTM_MSGQ_PTR	pMsgQ;

	FTM_LIST_PTR	pList;	
	FTM_LOCK_PTR	pLock;

	pthread_t		xThread;
	FTM_BOOL		bStop;

	pthread_t		xPCAPThread;
	FTM_LOCK_PTR	pPCAPLock;
	FTM_PCAP_PTR	pPCAP;
} FTM_ANALYZER, _PTR_ FTM_ANALYZER_PTR;

FTM_RET	FTM_ANALYZER_create
(
	struct	FTM_CATCHB_STRUCT _PTR_	pCatchB,
	FTM_ANALYZER_PTR _PTR_ ppAnalyzer
);

FTM_RET	FTM_ANALYZER_destroy
(
	FTM_ANALYZER_PTR _PTR_ ppAnalyzer
);

FTM_RET	FTM_ANALYZER_setConfig
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_ANALYZER_CONFIG_PTR	pConfig
);

FTM_RET	FTM_ANALYZER_getConfig
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_ANALYZER_CONFIG_PTR	pConfig
);

FTM_RET	FTM_ANALYZER_start
(
	FTM_ANALYZER_PTR pAnalyzer
);

FTM_RET	FTM_ANALYZER_stop
(
	FTM_ANALYZER_PTR pAnalyzer
);

FTM_RET	FTM_ANALYZER_getPortCount
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_ANALYZER_getPortList
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_UINT16_PTR		pPortList,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_ANALYZER_CCTV_attach
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTM_ANALYZER_CCTV_detach
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTM_ANALYZER_CCTV_count
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_ANALYZER_CCTV_getList
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_ID_PTR			pID,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_ANALYZER_PCAP_start
(
	FTM_ANALYZER_PTR	pAnalyzer
);

FTM_RET	FTM_ANALYZER_PCAP_stop
(
	FTM_ANALYZER_PTR	pAnalyzer
);

#endif
