#include <string.h>
#include <stdio.h>
#include "ftm_ssid.h"
#include "ftm_mem.h"
#include "ftm_trace.h"
#include "ftm_list.h"
#include "ftm_lock.h"
#include "ftm_utils.h"

#undef	__MODULE__
#define	__MODULE__	"ssid"

FTM_RET	FTM_HASH_SHA1
(
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulDataLen,
	FTM_CHAR_PTR	pValue,
	FTM_UINT32		ulBufferSize
);
static	FTM_BOOL	FTM_SSID_Seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);

static	FTM_BOOL	_initialized = FTM_FALSE;
static	FTM_LIST	_id_list;
static	FTM_LOCK	_lock;

FTM_RET		FTM_SSID_create(FTM_CHAR_PTR	pID, FTM_CHAR_PTR pPasswd, FTM_CHAR_PTR	pKey)
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_UINT32	ulTime;

	INFO("ID : %s, Passwd : %s", pID, pPasswd);
	if (!_initialized)
	{
		xRet = FTM_LIST_init(&_id_list) ;
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		xRet = FTM_LOCK_init(&_lock);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		FTM_LIST_setSeeker(&_id_list, FTM_SSID_Seeker);

		_initialized = FTM_TRUE;
	}

#if 0
	xRet = FTM_LIST_seek(&_id_list, pKey);
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_ALREADY_EXIST;
	}
#endif

	FTM_TIME_getCurrentSecs(&ulTime);

	FTM_SSID_PTR	pNewSSID = (FTM_SSID_PTR)FTM_MEM_malloc(sizeof(FTM_SSID));
	if (pNewSSID == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}


	FTM_UINT32		ulSeedLen = 0;
	FTM_UINT32		ulSeedMaxLen = sizeof(pID) + sizeof(pPasswd) + 32;
	FTM_CHAR_PTR	pSeed = (FTM_CHAR_PTR)FTM_MEM_malloc(ulSeedMaxLen);
	if (pSeed == NULL)
	{
		FTM_MEM_free(pNewSSID);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	ulSeedLen += snprintf(&pSeed[ulSeedLen], ulSeedMaxLen - ulSeedLen, "%s", pID);
	ulSeedLen += snprintf(&pSeed[ulSeedLen], ulSeedMaxLen - ulSeedLen, "%s", pPasswd);
	ulSeedLen += snprintf(&pSeed[ulSeedLen], ulSeedMaxLen - ulSeedLen, "%08x", ulTime);
	ulSeedLen += snprintf(&pSeed[ulSeedLen], ulSeedMaxLen - ulSeedLen, "%08x", (FTM_UINT32)pNewSSID);

	FTM_HASH_SHA1((FTM_UINT8_PTR)pSeed, ulSeedLen, pNewSSID->pKey, FTM_SESSION_ID_LEN);
	pNewSSID->ulTime = ulTime;
	strncpy(pKey, pNewSSID->pKey, FTM_SESSION_ID_LEN);

	FTM_LIST_append(&_id_list, pNewSSID);

	FTM_MEM_free(pSeed);

	return	FTM_RET_OK;

}

FTM_RET	FTM_SSID_destroy(FTM_CHAR_PTR	pKey)
{
	FTM_RET	xRet;
	FTM_SSID_PTR	pSSID = NULL;
	xRet = FTM_LIST_get(&_id_list, pKey, (FTM_VOID_PTR _PTR_)&pSSID);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(&_id_list, pSSID);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pSSID);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_SSID_get(FTM_CHAR_PTR pKey, FTM_UINT32 ulTimeout, FTM_SSID_PTR _PTR_ ppSSID)
{
	FTM_RET	xRet;
	FTM_SSID_PTR	pSSID = NULL;

	if (!_initialized)
	{
		xRet = FTM_LIST_init(&_id_list) ;
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		xRet = FTM_LOCK_init(&_lock);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		FTM_LIST_setSeeker(&_id_list, FTM_SSID_Seeker);

		_initialized = FTM_TRUE;
	}

	xRet = FTM_LIST_get(&_id_list, pKey, (FTM_VOID_PTR _PTR_)&pSSID);
	if (xRet == FTM_RET_OK)
	{
		FTM_UINT32	ulTime;

		FTM_TIME_getCurrentSecs(&ulTime);

		INFO("ulTime = %d, ulTimeout = %d, pSSID->ulTimeout = %d", ulTime, ulTimeout, pSSID->ulTime);
		if ((ulTimeout != 0) && (ulTime - pSSID->ulTime > ulTimeout))
		{
			xRet = FTM_LIST_remove(&_id_list, pSSID);
			if (xRet == FTM_RET_OK)
			{
				FTM_MEM_free(pSSID);	
			}
	
			xRet = FTM_RET_OBJECT_NOT_FOUND;
		}
		else
		{
			*ppSSID = pSSID;	
		}
	}

	return	xRet;
}

FTM_RET		FTM_SSID_isValid(FTM_CHAR_PTR pKey, FTM_UINT32 ulTimeout)
{
	FTM_SSID_PTR	pSSID;

	if (FTM_SSID_get(pKey, ulTimeout, &pSSID) != FTM_RET_OK)
	{
		return	FTM_RET_INVALID_SSID;	
	}

	FTM_TIME_getCurrentSecs(&pSSID->ulTime);

	return	FTM_RET_OK;
}

FTM_BOOL	FTM_SSID_Seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTM_SSID_PTR	pSSID = (FTM_SSID_PTR)pElement;
	FTM_CHAR_PTR	pKey = (FTM_CHAR_PTR)pIndicator;

	return	(strcmp(pSSID->pKey, pKey) == 0);
}
