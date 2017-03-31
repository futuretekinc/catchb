#ifndef	BASE64_H_
#define	BASE64_H_

#include "ftm_types.h"

FTM_RET	FTM_BASE64_encode
(
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulDataLen,
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_UINT32_PTR	pulOutputLen
);

FTM_RET	FTM_BASE64_decode
(
	FTM_CHAR_PTR	pData,
	FTM_UINT32		ulDataLen,
	FTM_UINT8_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_UINT32_PTR	pulOutputLen
);
#endif
