#ifndef	FTM_UTILS_H_
#define	FTM_UTILS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ftm_types.h"

FTM_RET	FTM_getLocalIP
(
	struct in_addr*	pLocalIP
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
	FTM_CHAR_PTR		pValue,
	FTM_UINT32		ulBufferSize
);

FTM_RET	FTM_ARP_parsing
(
	FTM_CHAR_PTR	pTargetIP,
	FTM_CHAR_PTR	pMAC
);

FTM_INT FTM_PING_check
(
	FTM_CHAR_PTR	pTargetIP, 
	FTM_UINT32_PTR	pReplyCount
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

void FTM_SCORE_init
(
	FTM_SCORE_INFO_PTR	pScore
);

#endif
