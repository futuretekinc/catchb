#ifndef	FTM_CCTV_H_
#define	FTM_CCTV_H_
#include "ftm_types.h"
#include "ftm_timer.h"
#include "ftm_lock.h"

typedef	enum	FTM_CCTV_STAT_ENUM
{
	FTM_CCTV_STAT_UNREGISTERED=0,
	FTM_CCTV_STAT_UNUSED,
	FTM_CCTV_STAT_NORMAL,
	FTM_CCTV_STAT_ABNORMAL
}	FTM_CCTV_STAT, _PTR_ FTM_CCTV_STAT_PTR;

#define	FTM_CCTV_FIELD_ID			(1 << 0)
#define	FTM_CCTV_FIELD_IP			(1 << 1)
#define	FTM_CCTV_FIELD_SWITCH_ID	(1 << 2)
#define	FTM_CCTV_FIELD_COMMENT		(1 << 3)
#define	FTM_CCTV_FIELD_TIME			(1 << 4)
#define	FTM_CCTV_FIELD_HASH			(1 << 5)
#define	FTM_CCTV_FIELD_STAT			(1 << 6)
#define	FTM_CCTV_FIELD_ALL			(0xFF)

typedef	struct	FTM_CCTV_CONFIG_STRUCT
{
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_CHAR		pIP[FTM_IP_LEN+1];
	FTM_CHAR		pSwitchID[FTM_ID_LEN+1];
	FTM_CHAR		pComment[FTM_COMMENT_LEN+1];
	FTM_UINT32		ulTime;
	FTM_CHAR		pHash[FTM_HASH_LEN+1];
	FTM_CCTV_STAT	xStat;
}	FTM_CCTV_CONFIG, _PTR_	FTM_CCTV_CONFIG_PTR;

typedef	struct	FTM_CCTV_STRUCT
{
	FTM_CCTV_CONFIG	xConfig;

	FTM_UINT32		ulReferenceCount;
	FTM_LOCK		xLock;
	FTM_TIMER		xExpiredTimer;

	FTM_UINT16		pPortList[16];
	FTM_BOOL		pPortStat[16];
	FTM_UINT32		ulPortCount;
}	FTM_CCTV, _PTR_ FTM_CCTV_PTR;

FTM_RET	FTM_CCTV_create
(
	FTM_CCTV_CONFIG_PTR	pConfig,
	FTM_CCTV_PTR _PTR_ ppCCTV
);

FTM_RET	FTM_CCTV_destroy
(
	FTM_CCTV_PTR _PTR_ ppCCTV
);

FTM_RET	FTM_CCTV_lock
(
	FTM_CCTV_PTR	pCCTV
);

FTM_RET	FTM_CCTV_unlock
(
	FTM_CCTV_PTR	pCCTV
);

FTM_CHAR_PTR	FTM_CCTV_STAT_print
(
	FTM_CCTV_STAT	xStat
);
#endif
