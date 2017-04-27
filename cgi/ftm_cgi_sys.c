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
	FTM_FLOAT	fCPUUtilization = 0;
	FTM_UINT32	ulTotalMemory = 0;
	FTM_UINT32	ulFreeMemory = 0;

	pRoot = cJSON_CreateObject();

	FTM_getCPUUtilization(&fCPUUtilization);
	ERROR(xRet, "         CPU : %5.2f %%", fCPUUtilization);
	FTM_getMemoryUtilization(&ulTotalMemory, &ulFreeMemory);
	ERROR(xRet, "Total Memory : %d KB", ulTotalMemory);
	ERROR(xRet, " Free Memory : %d KB", ulFreeMemory);

	cJSON _PTR_ pSys = cJSON_CreateObject();

	cJSON_AddStringToObject(pSys, "date", FTM_TIME_printfCurrent(NULL));
	cJSON_AddNumberToObject(pSys, "cpu", fCPUUtilization);
	cJSON_AddNumberToObject(pSys, "total_memory", ulTotalMemory);
	cJSON_AddNumberToObject(pSys, "free_memory", ulFreeMemory);

	cJSON_AddItemToObject(pRoot, "utilization", pSys);

	return	FTM_CGI_finish(pReq, pRoot, FTM_RET_OK);
}

