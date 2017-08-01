#ifndef	FTM_UTILS_H_
#define	FTM_UTILS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ftm_types.h"
#include "ftm_time.h"

typedef	struct
{
	FTM_UINT32	ulTotal;
	FTM_UINT32	ulFree;
}	FTM_MEMORY_STATISTICS, _PTR_ FTM_MEMORY_STATISTICS_PTR;

typedef	struct
{
	FTM_UINT32	ulCount;
	struct {
		FTM_CHAR	pName[32];
		FTM_CHAR	pRate[16];
	}	xPartitions[16];
}	FTM_DISK_STATISTICS, _PTR_ FTM_DISK_STATISTICS_PTR;
typedef	struct
{
	FTM_UINT32	ulRxBytes;
	FTM_UINT32	ulTxBytes;
}	FTM_NET_STATISTICS, _PTR_ FTM_NET_STATISTICS_PTR;

typedef	struct
{
	FTM_TIME	xTime;
	FTM_FLOAT	fCPU;
	FTM_MEMORY_STATISTICS	xMemory;
	FTM_NET_STATISTICS		xNet;
}	FTM_STATISTICS, _PTR_ FTM_STATISTICS_PTR;

typedef	struct
{
	FTM_CHAR	pName[32];
	FTM_CHAR	pIP[32];	
	FTM_CHAR	pNetmask[32];	
}	FTM_NET_IF_INFO, _PTR_ FTM_NET_IF_INFO_PTR;

typedef	struct
{
	FTM_UINT32		ulIFCount;
	FTM_NET_IF_INFO	pIF[4];
	FTM_CHAR		pGateway[32];	
}	FTM_NET_INFO, _PTR_ FTM_NET_INFO_PTR;

FTM_RET	FTM_getLocalIP
(
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffSize
);

FTM_RET	FTM_getNetInfo
(
	FTM_NET_INFO_PTR	pNetInfo
);

FTM_RET	FTM_getNetIFInfo
(
	FTM_CHAR_PTR	pName,
	FTM_NET_IF_INFO_PTR	pInfo
);

FTM_RET	FTM_getGateway
(
	FTM_CHAR_PTR	pGateway,
	FTM_UINT32		ulGatewayLen
);

FTM_RET	FTM_portScan
(
	FTM_CHAR_PTR	pTargetIP,
	FTM_UINT16		nPort,
	FTM_BOOL_PTR	pOpened
);

FTM_RET	FTM_HASH_SHA1
(
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulDataLen,
	FTM_CHAR_PTR	pValue,
	FTM_UINT32		ulBufferSize
);

FTM_RET	FTM_ARP_parsing
(
	FTM_CHAR_PTR	pTargetIP,
	FTM_CHAR_PTR	pMAC
);

#define PORT_NUM 8
extern	const FTM_UINT16		pPortList[PORT_NUM];

FTM_RET	FTM_getProcessCount
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_areDuplicatesRunning
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32		ulPID,
	FTM_BOOL_PTR	pDuplicated
);

FTM_RET	FTM_checkPIDFile
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32		ulPID,
	FTM_BOOL_PTR	pGood
);

FTM_RET	FTM_createPIDFile
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32		ulPID
);

FTM_RET	FTM_destroyPIDFile
(
	FTM_CHAR_PTR	pProcessName
);

FTM_RET	FTM_ReadPID
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32_PTR	pulPID
);

FTM_RET	FTM_getCPUUtilization
(
	FTM_FLOAT_PTR	pUtilization
) ;

FTM_RET	FTM_getMemoryUtilization
(
	FTM_UINT32_PTR	pTotalMemory,
	FTM_UINT32_PTR	pFreeMemory
) ;

FTM_RET	FTM_getNetStatistics
(
	FTM_NET_STATISTICS_PTR	pStatistics
);

FTM_RET	FTM_getDiskUtilization
(
	FTM_DISK_STATISTICS_PTR	pInfo
) ;

FTM_RET	FTM_getStatistics
(
	FTM_STATISTICS_PTR	pStatistics
);

FTM_RET	FTM_getBootTime
(
	FTM_TIME_PTR	pTime
);

FTM_RET	FTM_setTime
(
	time_t			xTime
);

FTM_RET	FTM_getTime
(
	time_t	*pTime
);

FTM_RET	FTM_encryptUserID
(
	FTM_CHAR_PTR	pUserID,
	FTM_UINT32		ulUserIDLen,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
);

FTM_RET	FTM_decryptUserID
(
	FTM_CHAR_PTR	pEncryptedUserID,
	FTM_UINT32		ulEncryptedUserIDLen,
	FTM_CHAR_PTR	pUserID,
	FTM_UINT32		ulUserIDLen
);

FTM_RET	FTM_encryptPasswd
(
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
);

FTM_RET	FTM_decryptPasswd
(
	FTM_CHAR_PTR	pEncryptedPasswd,
	FTM_UINT32		ulEncryptedPasswdLen,
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen
);

FTM_CHAR_PTR	FTM_trim(FTM_CHAR_PTR	pString);

#endif
