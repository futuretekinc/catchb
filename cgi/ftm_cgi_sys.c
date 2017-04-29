#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_utils.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

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

