#include <string.h>
#include "ftm_buffer.h"
#include "ftm_mem.h"
#include "ftm_trace.h"

FTM_RET	FTM_BUFFER_create
(
	FTM_UINT32	ulMaxSize,
	FTM_BUFFER_PTR _PTR_ ppBuffer
)
{
	ASSERT(ppBuffer != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_BUFFER_PTR	pBuffer;

	pBuffer = (FTM_BUFFER_PTR)FTM_MEM_malloc(sizeof(FTM_BUFFER) + ulMaxSize + 1024);
	if (pBuffer == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create buffer!");
		goto finished;
	}

	pBuffer->ulMaxSize = ulMaxSize;

	*ppBuffer = pBuffer;

finished:
	return	xRet;
}

FTM_RET	FTM_BUFFER_destroy
(
	FTM_BUFFER_PTR _PTR_ ppBuffer
)
{
	ASSERT(ppBuffer != NULL);

	FTM_MEM_free((*ppBuffer));

	*ppBuffer = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_BUFFER_getSize
(
	FTM_BUFFER_PTR	pBuffer,
	FTM_UINT32_PTR	pSize
)
{
	ASSERT(pBuffer != NULL);
	ASSERT(pSize != NULL);

	*pSize = pBuffer->ulDataLen;

	return	FTM_RET_OK;
}

FTM_RET	FTM_BUFFER_pushBack
(
	FTM_BUFFER_PTR 	pBuffer,
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulDataLen
)
{
	ASSERT(pBuffer != NULL);
	ASSERT(pData != NULL);

	FTM_UINT32	ulTemp;

	if ((pBuffer->ulMaxSize - 1) < (pBuffer->ulDataLen + ulDataLen))
	{
		FTM_UINT32	ulOverSize;

		ulOverSize = (pBuffer->ulDataLen + ulDataLen) - (pBuffer->ulMaxSize - 1);

		pBuffer->ulDataLen -= ulOverSize;
		pBuffer->ulFirst = (pBuffer->ulFirst + ulOverSize) % pBuffer->ulMaxSize;
	}

	ulTemp = pBuffer->ulMaxSize - pBuffer->ulLast;
	if(ulTemp >= ulDataLen)
	{
		memcpy(&pBuffer->pBuffer[pBuffer->ulLast], pData, ulDataLen);
	}
	else
	{
		memcpy(&pBuffer->pBuffer[pBuffer->ulLast], pData, ulTemp);
		memcpy(&pBuffer->pBuffer[0], &pData[ulTemp], ulDataLen - ulTemp);
	}

	pBuffer->ulDataLen += ulDataLen;
	pBuffer->ulLast = (pBuffer->ulLast + ulDataLen) % pBuffer->ulMaxSize;

	return	FTM_RET_OK;
}

FTM_RET	FTM_BUFFER_popFront
(
	FTM_BUFFER_PTR 	pBuffer,
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulMaxLen,
	FTM_UINT32_PTR	pDataLen
)
{
	ASSERT(pBuffer != NULL);
	ASSERT(pData != NULL);
	ASSERT(pDataLen != NULL);

	if (pBuffer->ulFirst == pBuffer->ulLast)
	{
		*pDataLen = 0;
		return	FTM_RET_OK;
	}

	if (pBuffer->ulDataLen < ulMaxLen)
	{
		ulMaxLen = pBuffer->ulDataLen;
	}

	if (pBuffer->ulFirst + ulMaxLen > pBuffer->ulMaxSize)
	{
		FTM_UINT32	ulTemp;

		ulTemp = pBuffer->ulMaxSize - pBuffer->ulFirst;
		memcpy(pData, &pBuffer->pBuffer[pBuffer->ulFirst], ulTemp);
		memcpy(&pData[ulTemp], pBuffer->pBuffer, ulMaxLen - ulTemp);
		pBuffer->ulFirst = ulMaxLen - ulTemp;
	}
	else
	{
		memcpy(pData, &pBuffer->pBuffer[pBuffer->ulFirst], ulMaxLen);
		pBuffer->ulFirst += ulMaxLen;
	}

	pBuffer->ulDataLen -= ulMaxLen;

	*pDataLen = ulMaxLen;

	return	FTM_RET_OK;
}

FTM_RET	FTM_BUFFER_getFirst
(
	FTM_BUFFER_PTR	pBuffer,
	FTM_UINT8_PTR	pData
)
{
	ASSERT(pBuffer != NULL);
	ASSERT(pData != NULL);

	if (pBuffer->ulDataLen == 0)
	{
		return	FTM_RET_LIST_EMPTY;
	}

	*pData = pBuffer->pBuffer[pBuffer->ulFirst];

	return	FTM_RET_OK;
}

FTM_RET	FTM_BUFFER_getAt
(
	FTM_BUFFER_PTR	pBuffer,
	FTM_UINT32		ulIndex,
	FTM_UINT8_PTR	pData
)
{
	ASSERT(pBuffer != NULL);
	ASSERT(pData != NULL);

	if (pBuffer->ulDataLen <= ulIndex)
	{
		return	FTM_RET_LIST_EMPTY;
	}

	*pData = pBuffer->pBuffer[(pBuffer->ulFirst + ulIndex) % pBuffer->ulMaxSize];

	return	FTM_RET_OK;

}
