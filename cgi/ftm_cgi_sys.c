#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON/cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_utils.h"
#include "ftm_profile.h"
#include "ftm_json_utils.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

FTM_RET	FTM_CGI_GET_SYS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
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
		goto finished;
	}

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set passwd with invalid SSID.");		
		goto finished;
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
finished:
	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SYS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_TIME	xBootTime;
	FTM_FLOAT	fCPUUtilization = 0;
	FTM_UINT32	ulTotalMemory = 0;
	FTM_UINT32	ulFreeMemory = 0;
	FTM_DISK_STATISTICS	xDisk;
	FTM_NET_INFO	xNetInfo;

	memset(pSSID, 0, sizeof(pSSID));

	INFO("System information called!");
	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set passwd with invalid SSID.");		
		goto finished;
	}

	FTM_getCPUUtilization(&fCPUUtilization);
	FTM_getMemoryUtilization(&ulTotalMemory, &ulFreeMemory);
	FTM_getDiskUtilization(&xDisk);
	FTM_getNetInfo(&xNetInfo);

	INFO("         CPU : %5.2f %%", fCPUUtilization);
	INFO("Total Memory : %d KB", ulTotalMemory);
	INFO(" Free Memory : %d KB", ulFreeMemory);

	FTM_getBootTime(&xBootTime);
	cJSON_AddStringToObject(pRespRoot, "os", "linux 2.6.36");
	cJSON_AddStringToObject(pRespRoot, "boot time", FTM_TIME_printf(&xBootTime, "%Y/%m/%d %H:%M:%S"));
	INFO("%16s : %s", "Boot Time", FTM_TIME_printf(&xBootTime, "%Y/%m/%d %H:%M:%S"));
	cJSON_AddStringToObject(pRespRoot, "current time", FTM_TIME_printfCurrent(NULL));
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

	cJSON_AddItemToObject(pRespRoot, "utilization", pUtilization);

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

	cJSON_AddItemToObject(pRespRoot, "net", pNet);

finished:
	return	xRet;
}

FTM_RET	FTM_CGI_GET_SYS_getInfoConfig
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	FTM_RET		xRet = FTM_RET_OK;
	cJSON _PTR_	pRoot;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	cJSON _PTR_	pSection;
	FTM_SYSTEM_INFO	xInfo;
	
	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log with invalid SSID.");
		goto finished;
	}

	xRet = FTM_CLIENT_STAT_getInfo(pClient, pSSID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get system statistics information.");
		goto finished;	
	}

	pSection = cJSON_CreateObject();

	cJSON_AddNumberToObject(pSection, "interval", 	xInfo.xStatistics.ulInterval);
	cJSON_AddNumberToObject(pSection, "max count", 	xInfo.xStatistics.ulMaxCount);
	cJSON_AddNumberToObject(pSection, "count", 		xInfo.xStatistics.ulCount);
	cJSON_AddStringToObject(pSection, "first time",	FTM_TIME_printf2(xInfo.xStatistics.ulFirstTime, NULL));
	cJSON_AddStringToObject(pSection, "last time", 	FTM_TIME_printf2(xInfo.xStatistics.ulLastTime, NULL));

	cJSON_AddItemToObject(pRoot, "statistics", pSection);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SYS_getInfoConfig
(
	FTM_CLIENT_PTR	pClient,
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	cJSON _PTR_	pSection;
	FTM_SYSTEM_INFO	xInfo;
	
	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log with invalid SSID.");
		goto finished;
	}

	xRet = FTM_CLIENT_STAT_getInfo(pClient, pSSID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get system statistics information.");
		goto finished;	
	}

	pSection = cJSON_CreateObject();

	cJSON_AddNumberToObject(pSection, "interval", 	xInfo.xStatistics.ulInterval);
	cJSON_AddNumberToObject(pSection, "max count", 	xInfo.xStatistics.ulMaxCount);
	cJSON_AddNumberToObject(pSection, "count", 		xInfo.xStatistics.ulCount);
	cJSON_AddStringToObject(pSection, "first time",	FTM_TIME_printf2(xInfo.xStatistics.ulFirstTime, NULL));
	cJSON_AddStringToObject(pSection, "last time", 	FTM_TIME_printf2(xInfo.xStatistics.ulLastTime, NULL));

	cJSON_AddItemToObject(pRespRoot, "statistics", pSection);
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_SYS_setInfoConfig
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	cJSON _PTR_	pRoot;
	cJSON _PTR_	pSection;
	FTM_UINT32	ulCount = 0;
	FTM_UINT32	ulInterval = 0;
	FTM_SYSTEM_INFO	xInfo;
	
	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log with invalid SSID.");
		goto finished;
	}

	xRet = FTM_CLIENT_STAT_getInfo(pClient, pSSID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get system statistics information.");
		goto finished;	
	}

	xInfo.ulFields = 0;

	xRet = FTM_CGI_getCount(pReq, &ulCount, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		if (xInfo.xStatistics.ulMaxCount != ulCount)
		{
			xInfo.xStatistics.ulMaxCount = ulCount;
			xInfo.ulFields |= FTM_SYSTEM_FIELD_STATISTICS_MAX_COUNT;
		}
	}

	xRet = FTM_CGI_getInterval(pReq, &ulInterval, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		if (xInfo.xStatistics.ulInterval != ulInterval)
		{
			xInfo.xStatistics.ulInterval = ulInterval;
			xInfo.ulFields |= FTM_SYSTEM_FIELD_STATISTICS_INTERVAL;
		}
	}

	if (xInfo.ulFields != 0)
	{
		xRet = FTM_CLIENT_STAT_setInfo(pClient, pSSID, &xInfo, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get system information");
			goto finished;
		}
	}

	pSection = cJSON_CreateObject();

	cJSON_AddNumberToObject(pSection, "interval", 	xInfo.xStatistics.ulInterval);
	cJSON_AddNumberToObject(pSection, "max count", 	xInfo.xStatistics.ulMaxCount);
	cJSON_AddNumberToObject(pSection, "count", 		xInfo.xStatistics.ulCount);
	cJSON_AddStringToObject(pSection, "first time",	FTM_TIME_printf2(xInfo.xStatistics.ulFirstTime, NULL));
	cJSON_AddStringToObject(pSection, "last time", 	FTM_TIME_printf2(xInfo.xStatistics.ulLastTime, NULL));

	cJSON_AddItemToObject(pRoot, "statistics", pSection);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SYS_setInfoConfig
(
	FTM_CLIENT_PTR	pClient,
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	cJSON _PTR_	pSection;
	FTM_UINT32	ulCount = 0;
	FTM_UINT32	ulInterval = 0;
	FTM_SYSTEM_INFO	xInfo;
	
	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log with invalid SSID.");
		goto finished;
	}

	xRet = FTM_CLIENT_STAT_getInfo(pClient, pSSID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get system statistics information.");
		goto finished;	
	}

	xInfo.ulFields = 0;

	xRet = FTM_JSON_getCount(pReqRoot, FTM_FALSE, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		if (xInfo.xStatistics.ulMaxCount != ulCount)
		{
			xInfo.xStatistics.ulMaxCount = ulCount;
			xInfo.ulFields |= FTM_SYSTEM_FIELD_STATISTICS_MAX_COUNT;
		}
	}

	xRet = FTM_JSON_getInterval(pReqRoot, FTM_FALSE, &ulInterval);
	if (xRet == FTM_RET_OK)
	{
		if (xInfo.xStatistics.ulInterval != ulInterval)
		{
			xInfo.xStatistics.ulInterval = ulInterval;
			xInfo.ulFields |= FTM_SYSTEM_FIELD_STATISTICS_INTERVAL;
		}
	}

	if (xInfo.ulFields != 0)
	{
		xRet = FTM_CLIENT_STAT_setInfo(pClient, pSSID, &xInfo, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get system information");
			goto finished;
		}
	}

	pSection = cJSON_CreateObject();

	cJSON_AddNumberToObject(pSection, "interval", 	xInfo.xStatistics.ulInterval);
	cJSON_AddNumberToObject(pSection, "max count", 	xInfo.xStatistics.ulMaxCount);
	cJSON_AddNumberToObject(pSection, "count", 		xInfo.xStatistics.ulCount);
	cJSON_AddStringToObject(pSection, "first time",	FTM_TIME_printf2(xInfo.xStatistics.ulFirstTime, NULL));
	cJSON_AddStringToObject(pSection, "last time", 	FTM_TIME_printf2(xInfo.xStatistics.ulLastTime, NULL));

	cJSON_AddItemToObject(pRespRoot, "statistics", pSection);
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_SYS_checkPasswd
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	cJSON _PTR_	pRoot;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_PROFILE	xProfile;

	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set passwd with invalid SSID.");		
		goto finished;
	}

	xRet = FTM_PROFILE_get(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		goto finished;
	}

	memset(pPasswd, 0, sizeof(pPasswd));

	xRet = FTM_CGI_getPasswd(pReq, pPasswd, sizeof(pPasswd), FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to change passwd with invalid passwd.!");
		goto finished;
	}

	INFO("Passwd  : %s", xProfile.pPasswd);
	INFO("Input Passwd  : %s", pPasswd);
	if (strcmp(xProfile.pPasswd, pPasswd) != 0)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRoot, "message", "Passwd do not match!");
		goto finished;
	}

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SYS_checkPasswd
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_PROFILE	xProfile;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set passwd with invalid SSID.");		
		goto finished;
	}

	xRet = FTM_PROFILE_get(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		goto finished;
	}

	memset(pPasswd, 0, sizeof(pPasswd));

	xRet = FTM_JSON_getPasswd(pReqRoot, FTM_FALSE, pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to change passwd with invalid passwd.!");
		goto finished;
	}

	INFO("Passwd  : %s", xProfile.pPasswd);
	INFO("Input Passwd  : %s", pPasswd);
	if (strcmp(xProfile.pPasswd, pPasswd) != 0)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRespRoot, "message", "Passwd do not match!");
		goto finished;
	}

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_SYS_setPasswd
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	cJSON _PTR_	pRoot;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_CHAR	pNewPasswd[FTM_PASSWD_LEN+1];
	FTM_PROFILE	xProfile;

	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	memset(pPasswd, 0, sizeof(pPasswd));
	xRet = FTM_CGI_getPasswd(pReq, pPasswd, sizeof(pPasswd), FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to change passwd with invalid passwd.!");
		goto finished;
	}

	memset(pNewPasswd, 0, sizeof(pNewPasswd));
	xRet = FTM_CGI_getNewPasswd(pReq, pNewPasswd, sizeof(pNewPasswd), FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to change passwd with invalid new passwd.!");
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set passwd with invalid SSID.");		
		goto finished;
	}

	xRet = FTM_PROFILE_get(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		goto finished;
	}

	if (strcmp(xProfile.pPasswd, pPasswd) != 0)
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

	xRet = FTM_PROFILE_set(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "There was a problem saving the password.");
	}

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SYS_setPasswd
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_CHAR	pNewPasswd[FTM_PASSWD_LEN+1];
	FTM_PROFILE	xProfile;

	memset(pSSID, 0, sizeof(pSSID));
	memset(pPasswd, 0, sizeof(pPasswd));
	memset(pNewPasswd, 0, sizeof(pNewPasswd));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_JSON_getPasswd(pReqRoot, FTM_FALSE, pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to change passwd with invalid passwd.!");
		goto finished;
	}

	xRet = FTM_JSON_getNewPasswd(pReqRoot, FTM_FALSE, pNewPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to change passwd with invalid new passwd.!");
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set passwd with invalid SSID.");		
		goto finished;
	}

	xRet = FTM_PROFILE_get(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		goto finished;
	}

	if (strcmp(xProfile.pPasswd, pPasswd) != 0)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRespRoot, "message", "Passwd do not match!");
		goto finished;
	}

	if (strlen(pNewPasswd) < 8)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRespRoot, "message", "The new passwd is too short!");
		goto finished;
	}

	strncpy(xProfile.pPasswd, pNewPasswd, sizeof(xProfile.pPasswd) - 1);

	xRet = FTM_PROFILE_set(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "There was a problem saving the password.");
	}

finished:

	return	xRet;
}


FTM_RET	FTM_CGI_GET_SYS_setTime
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	cJSON _PTR_	pRoot;
	FTM_UINT32	ulTime = 0;

	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create json object!");
		return	xRet;
	}

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_CGI_getDate(pReq, &ulTime, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid time.!");
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_setTime((time_t)ulTime);
	}

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}


FTM_RET	FTM_CGI_POST_SYS_setTime
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulTime = 0;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_JSON_getDate(pReqRoot, FTM_FALSE, &ulTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid time.!");
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_setTime((time_t)ulTime);
		if (xRet == FTM_RET_OK)
		{
			time_t	xTime = 0;
			FTM_CHAR	pBuff[16];

			FTM_getTime(&xTime);

			cJSON_AddNumberToObject(pRespRoot, "time", (FTM_UINT32)xTime);

			FTM_CLIENT_disconnect(pClient);

			FILE* fp = popen("/etc/init.d/catchb restart", "r");
			if (fp == NULL)
			{
				ERROR(xRet, "Failed to restart catchb!");
			}
			else
			{
				fread(pBuff, 1, 1, fp);
			}
			pclose(fp);
		}
	}

finished:

	return	xRet;
}


FTM_RET	FTM_CGI_POST_SYS_upload
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulTime = 0;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_JSON_getDate(pReqRoot, FTM_FALSE, &ulTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid time.!");
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_setTime((time_t)ulTime);
		if (xRet == FTM_RET_OK)
		{
			time_t	xTime = 0;

			FTM_getTime(&xTime);

			cJSON_AddNumberToObject(pRespRoot, "time", (FTM_UINT32)xTime);
		}
	}

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_POST_SYS_reboot
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));
	memset(pPasswd, 0, sizeof(pPasswd));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reboot with invalid SSID.!");
		goto finished;
	}
#if 0	
	xRet = FTM_JSON_getUserID(pReqRoot, FTM_FALSE, pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reboot with invalid arguments.!");
		goto finished;
	}

	xRet = FTM_JSON_getPasswd(pReqRoot, FTM_FALSE, pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reboot with invalid arguments.!");
		goto finished;
	}

	FTM_PROFILE	xProfile;
	xRet = FTM_PROFILE_get(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to reboot with internal error!");
		goto finished;
	}

	if ((strcmp(xProfile.pUserID, pID) != 0) || (strcmp(xProfile.pPasswd, pPasswd) != 0))
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRespRoot, "message", "Passwd do not match!");
		goto finished;
	}
#endif

	FILE*	fp;

	fp = popen("reboot", "r");
	if (fp == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to reboot with invalid command!");
	}
	else
	{
		FTM_CHAR	pBuff[16];

		fread(pBuff, 1, 16, fp);
		pclose(fp);
	}
	
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_POST_SYS_shutdown
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));
	memset(pPasswd, 0, sizeof(pPasswd));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set time with invalid SSID.!");
		goto finished;
	}

#if 0
	xRet = FTM_JSON_getUserID(pReqRoot, FTM_FALSE, pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reboot with invalid arguments.!");
		goto finished;
	}

	xRet = FTM_JSON_getPasswd(pReqRoot, FTM_FALSE, pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reboot with invalid arguments.!");
		goto finished;
	}

	FTM_PROFILE	xProfile;
	xRet = FTM_PROFILE_get(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to reboot with internal error!");
		goto finished;
	}

	if ((strcmp(xProfile.pUserID, pID) != 0) || (strcmp(xProfile.pPasswd, pPasswd) != 0))
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRespRoot, "message", "Passwd do not match!");
		goto finished;
	}
#endif
	FILE*	fp;

	fp = popen("shutdown", "r");
	if (fp == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to shutdown with invalid command!");
	}
	else
	{
		FTM_CHAR	pBuff[16];

		fread(pBuff, 1, 16, fp);
		pclose(fp);
	}
	
finished:

	return	xRet;
}

