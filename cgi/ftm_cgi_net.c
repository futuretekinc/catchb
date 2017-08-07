#include <ctype.h>
#include <string.h>
#include "cJSON/cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_utils.h"
#include "ftm_profile.h"
#include "ftm_json_utils.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

FTM_RET	FTM_CGI_POST_NET_get
(
	FTM_CLIENT_PTR 	pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];

	memset(pSSID, 0, sizeof(pSSID));

	INFO("System information called!");
	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get net with invalid SSID.!");
		goto finished;
	}
	
	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get net with invalid SSID.");		
		goto finished;
	}

	FTM_NET_IF_INFO	xNetIFInfo;

	xRet = FTM_getNetIFInfo("wan", &xNetIFInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get network information!"); 
		goto finished;	
	} 

	cJSON_AddStringToObject(pRespRoot, "ip", 	xNetIFInfo.pIP); 
	INFO("%16s : %s", "IP", xNetIFInfo.pIP); 
	cJSON_AddStringToObject(pRespRoot, "netmask", xNetIFInfo.pNetmask); 
	INFO("%16s : %s", "Netmask", xNetIFInfo.pNetmask); 
	FTM_CHAR	pGateway[64]; 
	FTM_getGateway(pGateway, sizeof(pGateway)); 
	cJSON_AddStringToObject(pRespRoot, "gateway", pGateway); 

finished: 
	return	xRet; 
} 

FTM_RET	FTM_CGI_POST_NET_set
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	
	memset(pSSID, 0, sizeof(pSSID));

	INFO("System information called!");
	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set net with invalid SSID.!");
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set net with invalid SSID.");		
		goto finished;
	}

	FTM_CHAR	pIP[FTM_IP_LEN+1];
	memset(pIP, 0, sizeof(pIP));
	xRet = FTM_JSON_getString(pReqRoot, "ip", FTM_IP_LEN, FTM_FALSE, pIP);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set net with invalid IP.");		
		goto finished;
	}

	FTM_CHAR	pNetmask[FTM_IP_LEN+1];
	memset(pNetmask, 0, sizeof(pNetmask));
	xRet = FTM_JSON_getString(pReqRoot, "netmask", FTM_IP_LEN, FTM_FALSE, pNetmask);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set net with invalid netmask.");		
		goto finished;
	}


	FTM_CHAR	pGateway[FTM_IP_LEN+1];
	memset(pGateway, 0, sizeof(pGateway));
	xRet = FTM_JSON_getString(pReqRoot, "gateway", FTM_IP_LEN, FTM_FALSE, pGateway);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set net with invalid gateway.");		
		goto finished;
	}

	FTM_BOOL	bApply = FTM_FALSE;
	xRet = FTM_JSON_getBool(pReqRoot, "apply", FTM_TRUE, &bApply);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set net with invalid arguments.");		
		goto finished;
	}

	
	FILE*	fp;

	fp = fopen("/tmp/network.usr", "wt");
	if (fp == NULL)
	{
		ERROR(xRet,"Failed to set net with internal operation error!");
		xRet = FTM_RET_ERROR;
		goto finished;	
	}

	fprintf(fp, "config interface\n");
	fprintf(fp, "    option %8s %s\n", "type", 		"wan");
	fprintf(fp, "    option %8s %s\n", "name", 		"wan");
	fprintf(fp, "    option %8s %s\n", "proto", 	"static");
	fprintf(fp, "    option %8s %s\n", "ipaddr",	pIP);
	fprintf(fp, "    option %8s %s\n", "netmask", 	pNetmask);
	fprintf(fp, "    option %8s %s\n", "gateway", 	pGateway);
	fprintf(fp, "    option %8s %s\n", "port0", 	"eth0");
	fprintf(fp, "    option %8s %s\n", "port1", 	"eth1");

	fclose(fp);

	fp = popen("cp /tmp/network.usr /etc/config/", "r");
	if (fp == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet,"Failed to set net with internal operation error!");
		goto finished;	
	}

	FTM_CHAR	pBuff[16];
	fread(pBuff, sizeof(pBuff), 1, fp);
	pclose(fp);


	if (bApply)
	{
		fp = popen("/etc/init.d/wan restart", "r");
		if (fp == NULL)
		{
			xRet = FTM_RET_ERROR;
			ERROR(xRet,"Failed to apply with internal operation error!");
			goto finished;	
		}

		FTM_CHAR	pBuff[16];
		fread(pBuff, sizeof(pBuff), 1, fp);
		pclose(fp);

	}

finished:
	return	xRet;
}


