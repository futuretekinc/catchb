#include <ctype.h>
#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_utils.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

typedef	struct
{
	FTM_CHAR	pLocation[64];
	FTM_CHAR	pUserID[64];
	FTM_CHAR	pPasswd[64];
	FTM_UINT32	ulTimeout;
}	FTM_PROFILE, _PTR_ FTM_PROFILE_PTR;

FTM_CHAR_PTR	FTM_trim(FTM_CHAR_PTR	pString);
FTM_RET	FTM_CGI_getProfile(FTM_PROFILE_PTR	pProfile);
FTM_RET	FTM_CGI_setProfile(FTM_PROFILE_PTR	pProfile);

FTM_RET	FTM_CGI_getSysInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	cJSON _PTR_	pRoot;
	FTM_TIME	xBootTime;
	FTM_FLOAT	fCPUUtilization = 0;
	FTM_UINT32	ulTotalMemory = 0;
	FTM_UINT32	ulFreeMemory = 0;
	FTM_DISK_STATISTICS	xDisk;
	FTM_NET_INFO	xNetInfo;

	INFO("System information called!");
	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	FTM_getCPUUtilization(&fCPUUtilization);
	FTM_getMemoryUtilization(&ulTotalMemory, &ulFreeMemory);
	FTM_getDiskUtilization(&xDisk);
	FTM_getNetInfo(&xNetInfo);

	INFO("         CPU : %5.2f %%", fCPUUtilization);
	INFO("Total Memory : %d KB", ulTotalMemory);
	INFO(" Free Memory : %d KB", ulFreeMemory);

	FTM_getBootTime(&xBootTime);
	cJSON_AddStringToObject(pRoot, "os", "linux 2.6.36");
	cJSON_AddStringToObject(pRoot, "boot time", FTM_TIME_printf(&xBootTime, "%Y/%m/%d %H:%M:%S"));
	INFO("%16s : %s", "Boot Time", FTM_TIME_printf(&xBootTime, "%Y/%m/%d %H:%M:%S"));
	cJSON_AddStringToObject(pRoot, "current time", FTM_TIME_printfCurrent(NULL));
	INFO("%16s : %s", "Current Time", FTM_TIME_printfCurrent(NULL));

	cJSON _PTR_ pUtilization = cJSON_CreateObject();
	if (pUtilization == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	cJSON_AddNumberToObject(pUtilization, "cpu", fCPUUtilization);
	cJSON_AddNumberToObject(pUtilization, "total memory", ulTotalMemory);
	cJSON_AddNumberToObject(pUtilization, "free memory", ulFreeMemory);

	cJSON	_PTR_ pDiskUtils = cJSON_CreateArray();
	for(FTM_INT32 i = 0 ; i < xDisk.ulCount ; i++)
	{
		cJSON	_PTR_ pPartition = cJSON_CreateObject();

		cJSON_AddStringToObject(pPartition, "name", xDisk.xPartitions[i].pName);
		cJSON_AddStringToObject(pPartition, "use", xDisk.xPartitions[i].pRate);

		cJSON_AddItemToArray(pDiskUtils, pPartition);
	}
	cJSON_AddItemToObject(pUtilization, "disk", pDiskUtils);

	cJSON_AddItemToObject(pRoot, "utilization", pUtilization);

	cJSON _PTR_ pNet= cJSON_CreateObject();
	if (pNet == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	cJSON _PTR_ pInterfaces = cJSON_CreateArray();
	if (pInterfaces == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	for(FTM_UINT32 i = 0 ; i < xNetInfo.ulIFCount ; i++)
	{
		cJSON _PTR_ pInterface = cJSON_CreateObject();

		cJSON_AddStringToObject(pInterface, "name", xNetInfo.pIF[i].pName);
		INFO("%16s : %s", "Name", xNetInfo.pIF[i].pName);
		cJSON_AddStringToObject(pInterface, "ip", 	xNetInfo.pIF[i].pIP);
		INFO("%16s : %s", "IP", xNetInfo.pIF[i].pIP);
		cJSON_AddStringToObject(pInterface, "netmask", xNetInfo.pIF[i].pNetmask);
		INFO("%16s : %s", "Netmask", xNetInfo.pIF[i].pNetmask);

		cJSON_AddItemToArray(pInterfaces, pInterface);
	}
	cJSON_AddItemToObject(pNet, "interfaces", pInterfaces);
	cJSON_AddStringToObject(pNet, "gateway", xNetInfo.pGateway);
	INFO("%16s : %s", "Gateway", xNetInfo.pGateway);

	cJSON_AddItemToObject(pRoot, "net", pNet);

	return	FTM_CGI_finish(pReq, pRoot, FTM_RET_OK);
}

FTM_RET	FTM_CGI_setPasswd
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	cJSON _PTR_	pRoot;
	FTM_CHAR	pPasswd[64];
	FTM_CHAR	pNewPasswd[64];
	FTM_PROFILE	xProfile;

	INFO("System information called!");
	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	xRet = FTM_CGI_getProfile(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		goto finished;
	}

	memset(pPasswd, 0, sizeof(pPasswd));
	memset(pNewPasswd, 0, sizeof(pNewPasswd));

	xRet = FTM_CGI_getPasswd(pReq, pPasswd, sizeof(pPasswd), FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to change passwd because invalid passwd.!");
		goto finished;
	}

	xRet = FTM_CGI_getPasswd(pReq, pNewPasswd, sizeof(pNewPasswd), FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to change passwd because invalid new passwd.!");
		goto finished;
	}

	if (strcpy(xProfile.pPasswd, pPasswd) != 0)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRoot, "message", "Passwd do not match!");
		goto finished;
	}

	if (strlen(pNewPasswd) < 8)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRoot, "message", "The new passwd is too short!");
		goto finished;
	}

	strncpy(xProfile.pPasswd, pNewPasswd, sizeof(xProfile.pPasswd) - 1);

	xRet = FTM_CGI_setProfile(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "There was a problem saving the password.");
		goto finished;
	}

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}


FTM_RET	FTM_CGI_getProfile
(
	FTM_PROFILE_PTR	pProfile
)
{
	FTM_PROFILE	xProfile;
	char		pBuffer[512];

	memset(&xProfile, 0, sizeof(xProfile));

	FILE *fp = popen("/etc/init.d/webadmin get", "r");
	if (fp == NULL)
	{
		return	-1;	
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

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_setProfile(FTM_PROFILE_PTR	pProfile)
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

FTM_CHAR_PTR	FTM_trim(FTM_CHAR_PTR	pString)
{
	while(0 != (*pString) && isspace(*pString))
	{
		pString++;
	}

	if (0 != (*pString))
	{
		int nLen = strlen(pString);

		for(int i = nLen - 1 ; i >= 0 ; i--)
		{
			if (!isspace(pString[i]))
			{
				break;
			}

			pString[i] = 0;
		}
	}
	return	pString;
}
