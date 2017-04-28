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
	FTM_TIME	xBooTime;
	FTM_FLOAT	fCPUUtilization = 0;
	FTM_UINT32	ulTotalMemory = 0;
	FTM_UINT32	ulFreeMemory = 0;

	pRoot = cJSON_CreateObject();

	FTM_getBootTime(&xBootTime);
	FTM_getCPUUtilization(&fCPUUtilization);
	ERROR(xRet, "         CPU : %5.2f %%", fCPUUtilization);
	FTM_getMemoryUtilization(&ulTotalMemory, &ulFreeMemory);
	ERROR(xRet, "Total Memory : %d KB", ulTotalMemory);
	ERROR(xRet, " Free Memory : %d KB", ulFreeMemory);


	cJSON_AddStringToObject(pRoot, "boot time", FTM_TIME_printf(&xBootTime, "%Y/%m/%d %H:%M:%S"));
	cJSON_AddStringToObject(pRoot, "current time", FTM_TIME_printfCurrent(NULL));

	cJSON _PTR_ pUtilization = cJSON_CreateObject();
	cJSON_AddNumberToObject(pUtilization, "cpu", fCPUUtilization);
	cJSON_AddNumberToObject(pUtilization, "total_memory", ulTotalMemory);
	cJSON_AddNumberToObject(pUtilization, "free_memory", ulFreeMemory);

	FILE*	pFP = popen("route -n | sed 1d  | sed 1d | awk 'begin{gateway=0.0.0.0}{if($1 ~ /0.0.0.0/) { gateway=$2 } else if ($2 ~ /0.0.0.0/) {ip=$1;netmask=$3;ifname=$8}}END{print  ifname, ip, gateway, netmask}'", "r");
	
	while (fgets(pBuff, sizeof(pBuff), pFP) != NULL)
	{
		sscanf(pBuff, "%s %s %s %s", pIFName, pIP, pGateway, pNetmask);
	}
	cJSON_AddItemToObject(pRoot, "utilization", pSys);

	return	FTM_CGI_finish(pReq, pRoot, FTM_RET_OK);
}

