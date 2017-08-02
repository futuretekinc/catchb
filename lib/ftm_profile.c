#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cJSON/cJSON.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_utils.h"

typedef	struct
{
	FTM_CHAR	pLocation[64];
	FTM_CHAR	pUserID[64];
	FTM_CHAR	pPasswd[64];
	FTM_UINT32	ulTimeout;
}	FTM_PROFILE, _PTR_ FTM_PROFILE_PTR;

FTM_RET	FTM_PROFILE_get
(
	FTM_PROFILE_PTR	pProfile
)
{
	FTM_RET		xRet = FTM_RET_OK;
	FTM_PROFILE	xProfile;
	char		pBuffer[512];

	memset(&xProfile, 0, sizeof(xProfile));

	FILE *fp = popen("/etc/init.d/webadmin get", "r");
	if (fp == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		return	xRet;	
	}

	while(0 != fgets(pBuffer, sizeof(pBuffer), fp))
	{
		char	pTag[32];
		char	*pPtr = pBuffer;
	
		if (pBuffer[0] == '#')
		{	
			continue;
		}

		if (1 != sscanf(pPtr, "%s", pTag))
		{
			continue;
		}

		if (0 == strcmp(pTag, "location:"))
		{
			pPtr = FTM_trim(pPtr + strlen(pTag));

			strncpy(xProfile.pLocation, pPtr, sizeof(xProfile.pLocation) - 1);
		}
		else if (0 == strcmp(pTag, "userid:"))
		{
			pPtr = FTM_trim(pPtr + strlen(pTag));

			strncpy(xProfile.pUserID, pPtr, sizeof(xProfile.pUserID) - 1);
		}
		else if (0 == strcmp(pTag, "passwd:"))
		{
			INFO("PASSWD : %s", pPtr);
			pPtr = FTM_trim(pPtr + strlen(pTag));

			strncpy(xProfile.pPasswd, pPtr, sizeof(xProfile.pPasswd) - 1);
		}
		else if (0 == strcmp(pTag, "timeout:"))
		{
			int	nTimeout;

			sscanf(pPtr + strlen(pTag), "%d", &nTimeout);
			pPtr = FTM_trim(pPtr + strlen(pTag));

			if (nTimeout < 0)
			{
				xProfile.ulTimeout = nTimeout;	
			}
		}
	}

	pclose(fp);

	memcpy(pProfile, &xProfile, sizeof(xProfile));

	return	FTM_RET_OK;
}

FTM_RET	FTM_PROFILE_set(FTM_PROFILE_PTR	pProfile)
{
	FTM_CHAR	pCmdLine[1024];
	FTM_UINT32	nCmdLen = 0;

	memset(pCmdLine, 0, sizeof(pCmdLine));

	if (strlen(pProfile->pLocation) != 0)
	{
		nCmdLen += snprintf(&pCmdLine[nCmdLen], sizeof(pCmdLine) - nCmdLen - 1, "LOCATION=%s ", pProfile->pLocation);
	}

	if (strlen(pProfile->pUserID) != 0)
	{
		nCmdLen += snprintf(&pCmdLine[nCmdLen], sizeof(pCmdLine) - nCmdLen - 1, "USERID=%s ", pProfile->pUserID);
	}

	if (strlen(pProfile->pPasswd) != 0)
	{
		nCmdLen += snprintf(&pCmdLine[nCmdLen], sizeof(pCmdLine) - nCmdLen - 1, "PASSWD=%s ", pProfile->pPasswd);
	}

	nCmdLen += snprintf(&pCmdLine[nCmdLen], sizeof(pCmdLine) - nCmdLen - 1, "TIMEOUT=%u ", pProfile->ulTimeout);

	nCmdLen += snprintf(&pCmdLine[nCmdLen], sizeof(pCmdLine) - nCmdLen - 1, " /etc/init.d/webadmin update");
	
	FILE *fp = popen(pCmdLine, "r");
	if (fp == NULL)
	{
		return	-1;	
	}

	pclose(fp);

	return	0;
}

