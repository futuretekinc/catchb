#ifndef	FTM_SCORE_H_
#define	FTM_SCORE_H_

#include "ftm_types.h"

typedef struct FTM_OS_MATRIX_STRUCT
{
	FTM_BOOL 	bTTL;
	FTM_BOOL 	bDF;
	FTM_BOOL 	bTCPMSS;
	FTM_BOOL 	bWindowScale; 
	FTM_BOOL 	bWindowSize;
	FTM_BOOL 	bSynPacketSize;
	FTM_BOOL 	bOptionsOrder;
	FTM_BOOL 	bIPID;
}	FTM_OS_MATRIX, _PTR_ FTM_OS_MATRIX_PTR; 

typedef	struct	FTM_SCORE_STRUCT
{
	pid_t 		tid;
	FTM_FLOAT	fValue;
	FTM_OS_MATRIX xOSMatrix;
} FTM_SCORE, _PTR_ FTM_SCORE_PTR; 

void FTM_SCORE_processIP
(
	FTM_SCORE_PTR	pScoreInfo,
	FTM_UINT8_PTR	pPacket,
	FTM_UINT32		ulLen
);

void FTM_SCORE_processTCP
(
	FTM_SCORE_PTR	pScoreInfo,
	FTM_UINT8_PTR	pPacket,
	FTM_UINT32		ulLen
);
#endif
