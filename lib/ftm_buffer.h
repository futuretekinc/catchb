#ifndef	FTM_BUFFER_H_
#define	FTM_BUFFER_H_

#include "ftm_types.h"

typedef	struct FTM_BUFFER_STRUCT
{
	FTM_UINT32		ulFirst;
	FTM_UINT32		ulLast;
	FTM_UINT32		ulDataLen;
	FTM_UINT32		ulMaxSize;
	FTM_UINT8		pBuffer[];
}	FTM_BUFFER, _PTR_ FTM_BUFFER_PTR;

FTM_RET	FTM_BUFFER_create
(
	FTM_UINT32	ulBufferSize,
	FTM_BUFFER_PTR _PTR_ ppBuffer
);

FTM_RET	FTM_BUFFER_destroy
(
	FTM_BUFFER_PTR _PTR_ ppBuffer
);

FTM_RET	FTM_BUFFER_getSize
(
	FTM_BUFFER_PTR	pBuffer,
	FTM_UINT32_PTR	pSize
);

FTM_RET	FTM_BUFFER_pushBack
(
	FTM_BUFFER_PTR 	pBuffer,
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulDataLen
);

FTM_RET	FTM_BUFFER_popFront
(
	FTM_BUFFER_PTR 	pBuffer,
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulMaxLen,
	FTM_UINT32_PTR	pDataLen
);

FTM_RET	FTM_BUFFER_getFirst
(
	FTM_BUFFER_PTR	pBuffer,
	FTM_UINT8_PTR	pData
);

FTM_RET	FTM_BUFFER_getAt
(
	FTM_BUFFER_PTR	pBuffer,
	FTM_UINT32		ulIndex,
	FTM_UINT8_PTR	pData
);
#endif
