#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_utils.h"

FTM_RET	FTM_CGI_getSysInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	cJSON _PTR_	pRoot;
	FTM_TIME	xBootTime;
	FTM_FLOAT	fCPUUtilization = 0;
	FTM_UINT32	ulTotalMemory = 0;
	FTM_UINT32	ulFreeMemory = 0;
	FTM_NET_INFO	xNetInfo;

	pRoot = cJSON_CreateObject();

	FTM_getCPUUtilization(&fCPUUtilization);
	FTM_getMemoryUtilization(&ulTotalMemory, &ulFreeMemory);
	FTM_getNetInfo(&xNetInfo);

	ERROR(xRet, "         CPU : %5.2f %%", fCPUUtilization);
	ERROR(xRet, "Total Memory : %d KB", ulTotalMemory);
	ERROR(xRet, " Free Memory : %d KB", ulFreeMemory);


	cJSON_AddStringToObject(pRoot, "boot time", FTM_TIME_printf(&xBootTime, "%Y/%m/%d %H:%M:%S"));
	cJSON_AddStringToObject(pRoot, "current time", FTM_TIME_printfCurrent(NULL));

	cJSON _PTR_ pUtilization = cJSON_CreateObject();
	cJSON_AddNumberToObject(pUtilization, "cpu", fCPUUtilization);
	cJSON_AddNumberToObject(pUtilization, "total_memory", ulTotalMemory);
	cJSON_AddNumberToObject(pUtilization, "free_memory", ulFreeMemory);
	cJSON_AddItemToObject(pRoot, "utilization", pUtilization);

	cJSON _PTR_ pNet= cJSON_CreateObject();
	cJSON _PTR_ pInterfaces = cJSON_CreateArray();
	for(FTM_UINT32 i = 0 ; i < xNetInfo.ulIFCount ; i++)
	{
		cJSON _PTR_ pInterface = cJSON_CreateObject();

		cJSON_AddStringToObject(pInterface, "name", xNetInfo.pIF[i].pName);
		cJSON_AddStringToObject(pInterface, "ip", 	xNetInfo.pIF[i].pIP);
		cJSON_AddStringToObject(pInterface, "netmask", xNetInfo.pIF[i].pNetmask);

		cJSON_AddItemToArray(pInterfaces, pInterfaces);
	}
	cJSON_AddItemToObject(pNet, "interfaces", pInterfaces);
	cJSON_AddStringToObject(pNet, "gateway", xNetInfo.pGateway);

	cJSON_AddItemToObject(pRoot, "net", pNet);

	return	FTM_CGI_finish(pReq, pRoot, FTM_RET_OK);
}

