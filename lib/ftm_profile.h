#ifndef	FTM_PROFILE_H_
#define	FTM_PROFILE_H_

typedef	struct
{
	FTM_CHAR	pLocation[64];
	FTM_CHAR	pUserID[64];
	FTM_CHAR	pPasswd[64];
	FTM_UINT32	ulTimeout;
}	FTM_PROFILE, _PTR_ FTM_PROFILE_PTR;

FTM_RET	FTM_PROFILE_get(FTM_PROFILE_PTR	pProfile);
FTM_RET	FTM_PROFILE_set(FTM_PROFILE_PTR	pProfile);

#endif
