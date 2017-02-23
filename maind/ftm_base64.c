#include <stdint.h>
#include <stdlib.h>
#include "ftm_base64.h"
#include "ftm_trace.h"

static 
FTM_CHAR	pEncodingTable[] = 
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

static 
FTM_CHAR	pDecodingTable[256] = { 0, };
static 
FTM_INT			pModTable[] = {0, 2, 1};


FTM_RET	FTM_BASE64_encode
(
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulDataLen,
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_UINT32_PTR	pulOutputLen
) 
{
	ASSERT(pData != NULL);
	ASSERT(pBuff != NULL);
	ASSERT(pulOutputLen != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	ulOutputLen;
	FTM_UINT32	i, j;

	ulOutputLen = 4 * ((ulDataLen + 2) / 3);
	if (ulBuffLen < ulOutputLen)
	{
		xRet = FTM_RET_BUFFER_TOO_SMALL;	
		ERROR(xRet, "The buffer size is too small to encoded.");
		return	xRet;
	}

	for (i = 0, j = 0; i < ulDataLen; ) 
	{
		FTM_UINT32	ulValue;

		ulValue = (FTM_UINT32)pData[i++] << 16;
		if (i < ulDataLen)
		{
			ulValue |= (FTM_UINT32)pData[i++] << 8;

			if (i < ulDataLen)
			{
				ulValue |= (FTM_UINT32)pData[i++];
			}
		}

		pBuff[j++] = pEncodingTable[(ulValue >> 3 * 6) & 0x3F];
		pBuff[j++] = pEncodingTable[(ulValue >> 2 * 6) & 0x3F];
		pBuff[j++] = pEncodingTable[(ulValue >> 1 * 6) & 0x3F];
		pBuff[j++] = pEncodingTable[(ulValue >> 0 * 6) & 0x3F];
	}

	for (i = 0; i < pModTable[ulDataLen % 3]; i++)
	{
		pBuff[ulOutputLen - 1 - i] = '=';
	}

	*pulOutputLen = ulOutputLen;

	return FTM_RET_OK;
}


FTM_RET	FTM_BASE64_decode
(
	FTM_CHAR_PTR	pData,
	FTM_UINT32		ulDataLen,
	FTM_UINT8_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_UINT32_PTR	pulOutputLen
)
{
	ASSERT(pData != NULL);
	ASSERT(pBuff != NULL);
	ASSERT(pulOutputLen != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	ulOutputLen;
	FTM_UINT32	i, j;

	if (pDecodingTable[0] == 0)
	{	
		for(i = 0 ; i < 64 ; i++)
		{
			pDecodingTable[(FTM_UINT8)pEncodingTable[i]] = i;
		}

		pDecodingTable[0] = 1;
	}

	if (ulDataLen % 4 != 0)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		ERROR(xRet, "Input data length is invalid.");
		return xRet;
	}

	ulOutputLen = ulDataLen / 4 * 3;
	if (pData[ulDataLen - 1] == '=') 
	{
		ulOutputLen--;
	}

	if (pData[ulDataLen - 2] == '=') 
	{
		ulOutputLen--;
	}

	if (ulBuffLen < ulOutputLen)
	{
		xRet = FTM_RET_BUFFER_TOO_SMALL;
		ERROR(xRet, "The buffer size is too small to decoded.");
		return	xRet;
	}

	for (i = 0, j = 0; i < ulDataLen; i += 4, j += 3) 
	{
		FTM_UINT32	k;
		FTM_UINT32	ulValue = 0;

		for(k = 0 ; k < 4 ; k++)
		{
			if (pData[i+k] != '=')
			{
				ulValue |= (pDecodingTable[(FTM_UINT8)pData[i+k]] << ((3 - k) * 6));
			}
		}

		for(k = 0 ; (k < 3) && (k < (ulOutputLen - j)) ; k++)
		{
			pBuff[j+k] = (ulValue >> ((2 - k) * 8)) & 0xFF;
		}
	}

	*pulOutputLen = ulOutputLen;
	return FTM_RET_OK;
}

