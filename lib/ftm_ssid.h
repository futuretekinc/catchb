#ifndef	FTM_SSID_H_
#define	FTM_SSID_H_

#include "ftm_types.h"
#include "ftm_default.h"
#include "ftm_time.h"

typedef	struct
{
	FTM_CHAR	pKey[FTM_SESSION_ID_LEN+1];
	FTM_UINT32	ulTime;
}	FTM_SSID, _PTR_ FTM_SSID_PTR;

FTM_RET		FTM_SSID_create(FTM_CHAR_PTR	pID, FTM_CHAR_PTR pPasswd, FTM_CHAR_PTR	pKey);
FTM_RET		FTM_SSID_destroy(FTM_CHAR_PTR	pKey);

FTM_RET		FTM_SSIG_get(FTM_CHAR_PTR pKey, FTM_UINT32 ulTimeout, FTM_SSID_PTR _PTR_ ppSSID);
FTM_RET		FTM_SSID_isValid(FTM_CHAR_PTR pKey, FTM_UINT32 ulTimeout);

#endif
