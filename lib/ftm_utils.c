#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>

#include "ftm_utils.h"
#include "ftm_trace.h"
#include "libsha1.h"
#include "libaes.h"

#undef	__MODULE__
#define	__MODULE__	"utils"

FTM_RET	FTM_encrypt
(
	FTM_UINT8		pIV[16],
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
);

FTM_RET	FTM_decrypt
(
	FTM_UINT8		pIV[16],
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
);

FTM_RET	FTM_getLocalIP
(
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffSize
) 
{
	ASSERT(pBuff != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_NET_INFO	xNetInfo;

	xRet = FTM_getNetInfo(&xNetInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get net info!");
		return	xRet;
	}

	if (xNetInfo.ulIFCount == 0)
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;
		ERROR(xRet, "Failed to get net interface!");
		return	xRet;
	}

	strncpy(pBuff, xNetInfo.pIF[0].pIP, ulBuffSize);

	return	xRet;
}

FTM_RET	FTM_getNetInfo
(
	FTM_NET_INFO_PTR	pNetInfo
)
{
	ASSERT(pNetInfo != NULL);
	
	FILE*	pFP;
	FTM_UINT32	ulIFCount = 0;
	FTM_CHAR	pBuffer[256];
	FTM_CHAR	pIFNames[4][64];

	memset(pNetInfo, 0, sizeof(FTM_NET_INFO));
	memset(pIFNames, 0, sizeof(pIFNames));

	pFP = popen("cat /proc/net/dev | awk '{ if ($1 ~ /:/) print $1}' | sed 's/://'", "r");
	while(fgets(pBuffer, sizeof(pBuffer) - 1, pFP) != NULL)
	{
		if (strncmp(pBuffer, "lo", 2) != 0)
		{
			sscanf(pBuffer, "%s", pIFNames[ulIFCount++]);
		}
	}
	pclose(pFP);

	for(FTM_UINT32 i = 0 ; i < ulIFCount && pNetInfo->ulIFCount < sizeof(pNetInfo->pIF) / sizeof(pNetInfo->pIF[0]) ; i++)
	{
		if (FTM_getNetIFInfo(pIFNames[i], &pNetInfo->pIF[pNetInfo->ulIFCount]) ==  FTM_RET_OK)
		{
			pNetInfo->ulIFCount++;	
		}
	}

	FTM_getGateway(pNetInfo->pGateway, sizeof(pNetInfo->pGateway));

	return	FTM_RET_OK;
}

FTM_RET	FTM_getNetIFInfo
(
	FTM_CHAR_PTR		pIFName,
	FTM_NET_IF_INFO_PTR	pIFInfo
) 
{
	ASSERT(pIFName != NULL);
	ASSERT(pIFInfo != NULL);

	FTM_RET	xRet = FTM_RET_OK;
    FILE *pFP;
	FTM_CHAR	pCommandLine[256];
	FTM_CHAR	pBuffer[256];

	for(FTM_INT i = 0 ; i < strlen(pIFName) ; i++)
	{
		if (isspace(pIFName[i]))
		{
			pIFName[i] = '\0';	
		}
	}

	sprintf(pCommandLine, "ifconfig %s | grep 'inet addr:'", pIFName);
	pFP = popen(pCommandLine, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;	
		ERROR(xRet, "Failed to run shell command!");
		goto finished;
	}

	if (fgets(pBuffer, sizeof(pBuffer), pFP) == NULL)
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;
		goto finished;
	}
	pclose(pFP);
	pFP = NULL;

	FTM_CHAR_PTR	pStart = strstr(pBuffer, "inet addr:");
	if (pStart == NULL)
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;
		ERROR(xRet, "Failed to get ip!");
		goto finished;
	}

	FTM_CHAR_PTR	pToken= strtok(pStart + 10, " ");
	if (pToken == NULL)
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;
		ERROR(xRet, "Failed to get ip!");
		goto finished;
	}

	strncpy(pIFInfo->pIP, pToken, sizeof(pIFInfo->pIP) - 1);
	pStart = pToken+strlen(pToken) + 1;

	pStart = strstr(pStart, "Mask:");
	if (pStart == NULL)
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;
		ERROR(xRet, "Failed to get netmask!");
		goto finished;
	}

	pToken = strtok(pStart+5, " \n");
	if (pToken== NULL)
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;
		goto finished;
	}

	strncpy(pIFInfo->pNetmask, pToken, sizeof(pIFInfo->pNetmask) - 1);

	strncpy(pIFInfo->pName, pIFName, sizeof(pIFInfo->pName) - 1);
finished:
	if (pFP != NULL)
	{
		pclose(pFP);	
	}
	return	xRet;
}

FTM_RET	FTM_getGateway
(
	FTM_CHAR_PTR	pGateway,
	FTM_UINT32		ulGatewayLen
)
{
	FTM_RET	xRet = FTM_RET_OK;
	FILE	*pFP;

	pFP = popen("route -n", "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;
		goto finished;
	}

	while(1)
	{
		FTM_CHAR	pBuffer[256];
		FTM_CHAR	pArg1[64];
		FTM_CHAR	pArg2[64];

		if (fgets(pBuffer, sizeof(pBuffer), pFP) == NULL)
		{
			break;	
		}
	
		
		if (sscanf(pBuffer, "%s %s", pArg1, pArg2) != 2)
		{
			break;
		}

		if (strcmp(pArg1, "0.0.0.0") == 0)
		{
			strncpy(pGateway, pArg2, ulGatewayLen);	
			break;
		}
	}
	pclose(pFP);	
	
finished:
	
	return	xRet;
}

FTM_RET	FTM_portScan
(
	FTM_CHAR_PTR	pTargetIP,
	FTM_UINT16	nPort,
	FTM_BOOL_PTR	pOpened
)
{
	ASSERT(pTargetIP != NULL);
	ASSERT(pOpened != NULL);
	FTM_RET		xRet = FTM_RET_OK;
	FTM_SOCKET	xSocket;

	*pOpened = FTM_FALSE;

	xSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (xSocket < 0)
	{
		xRet = FTM_RET_SOCKET_CREATION_FAILED;
		ERROR(xRet, "Failed to create socket!");
	}
	else
	{
		struct sockaddr_in xTargetAddr; 
    	FTM_INT	nNewStat; 
    	FTM_INT	nOriginStat; 
		FTM_INT	nRet;

		xTargetAddr.sin_family 	= AF_INET; 
		xTargetAddr.sin_port 	= htons(nPort);
		xTargetAddr.sin_addr.s_addr = inet_addr(pTargetIP);

		nOriginStat = fcntl(xSocket, F_GETFL, NULL);
		if ( nOriginStat < 0 )  
		{ 
			xRet = FTM_RET_SOCKET_STAT_GET_FAILED;
			ERROR(xRet, "Failed to get socket stat!");
		} 
		else
		{
			nNewStat = nOriginStat | O_NONBLOCK; 

			if(fcntl(xSocket, F_SETFL, nNewStat) < 0) 
			{ 
				xRet = FTM_RET_SOCKET_STAT_SET_FAILED;
				ERROR(xRet, "Failed to set socket stat!");
			} 
			else
			{

				nRet = connect(xSocket, (struct sockaddr *)&xTargetAddr, sizeof(xTargetAddr));
				if(nRet < 0) 
				{ 
					xRet = FTM_RET_SOCKET_CONNECTION_FAILED;
				//	INFO("Failed to connect socket!");
				} 
				else if (nRet == 0) 
				{ 
					INFO("The connection was successful!");
					fcntl(xSocket, F_SETFL, nOriginStat); 
					*pOpened = FTM_TRUE;
				} 
				else
				{
					fd_set	xRFDS;
					fd_set	xWFDS;
					struct timeval xTimeout; 

					FD_ZERO(&xRFDS); 
					FD_SET(xSocket, &xRFDS); 

					xWFDS = xRFDS; 

					xTimeout.tv_sec	= 2;     
					xTimeout.tv_usec= 0; 

					if (select(xSocket+1, &xRFDS, &xWFDS, NULL, &xTimeout) == 0) 
					{ 
						xRet = FTM_RET_SOCKET_CONNECTION_TIMEOUT;
						ERROR(xRet, "Failed to connect socket!");
					}
					else
					{
						if (FD_ISSET(xSocket, &xRFDS) || FD_ISSET(xSocket, &xWFDS) ) 
						{ 
							FTM_INT	nError;
							socklen_t	nSize = sizeof(FTM_INT);

							if (getsockopt(xSocket, SOL_SOCKET, SO_ERROR, &nError, (socklen_t *)&nSize) < 0) 
							{
								xRet = FTM_RET_SOCKET_CONNECTION_FAILED;
								ERROR(xRet, "Failed to connect socket!");
							}
						} 
						else 
						{ 
							xRet = FTM_RET_SOCKET_CONNECTION_TIMEOUT;
							ERROR(xRet, "Failed to connect socket!");
						} 
					}
				}

				fcntl(xSocket, F_SETFL, nOriginStat); 
			}
		}

		close(xSocket);
	} 

	return xRet; 
}


FTM_RET	FTM_HASH_SHA1
(
	FTM_UINT8_PTR	pData,
	FTM_UINT32		ulDataLen,
	FTM_CHAR_PTR		pValue,
	FTM_UINT32		ulBufferSize
)
{
    SHA1Context xSHA;
	FTM_UINT32	i, ulLen = 0;
	FTM_CHAR	pBuff[41];
    /*
     *  Perform test A
     */
    SHA1Reset(&xSHA);
    SHA1Input(&xSHA, pData, ulDataLen);
    if (!SHA1Result(&xSHA))
    {
		ERROR(0, "ERROR-- could not compute message digest");
		return	FTM_RET_ERROR;
    }
    else
    {
		memset(pBuff, 0, sizeof(pBuff));

        for(i = 0; i < 5 ; i++)
        {
            ulLen += snprintf(&pBuff[ulLen], sizeof(pBuff) - ulLen - 1, "%08x", ntohl(xSHA.Message_Digest[i]));
        }

		strncpy(pValue, pBuff, ulBufferSize);
    }

    return FTM_RET_OK;
}

#define ARP_PATH "/proc/net/arp"

FTM_RET	FTM_ARP_parsing
(
	FTM_CHAR_PTR	pTargetIP,
	FTM_CHAR_PTR	pMAC
)
{
	FTM_RET	xRet = FTM_RET_OK;
    FILE *pFile;

    FTM_CHAR		pBuffer[128];
    FTM_CHAR_PTR	pSeperator = " ";
    FTM_UINT32	i;

    memset(pBuffer, 0x00, sizeof(pBuffer));

    pFile = fopen(ARP_PATH, "r");
    if(pFile == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to open %s", ARP_PATH);
	}
	else
	{
		while(fgets(pBuffer, sizeof(pBuffer), pFile))
		{
			if(strstr(pBuffer, pTargetIP))
			{
				FTM_CHAR_PTR	pPos;

				pPos = strtok(pBuffer, pSeperator);

				i = 0;

				while(pPos != NULL)
				{

					if(i == 3)
					{
						memcpy(pMAC, pPos, 48);

						break;
					}

					pPos = strtok(NULL, pSeperator);
					i++;
				}

				break;
			}
		}
		fclose(pFile);
	}

	return xRet;
}



FTM_RET	FTM_getProcessCount
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32_PTR	pCount
)
{
	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pCmd[128];
	FTM_INT		nCount = 0;
	FILE*		pFP;

	memset(pCmd, 0, sizeof(pCmd));
	snprintf(pCmd, sizeof(pCmd) - 1, "ps -a | egrep %s | awk '{ if ($5 ~ \"%s\") print $5}' | wc | awk '{ print $1 }'", pProcessName, pProcessName);

	pFP = popen(pCmd, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to execute shell command[%s]", pCmd);
	}
	else
	{
		fscanf(pFP, "%d", &nCount);	
		pclose(pFP);

		*pCount = nCount;
	}

	INFO("Process : %d", nCount);
	return	xRet;
}


FTM_RET	FTM_areDuplicatesRunning
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32		ulPID,
	FTM_BOOL_PTR	pDuplicated
)
{
	ASSERT(pProcessName != NULL);
	ASSERT(pDuplicated != NULL);
	FTM_CHAR	pCmd[128];
	FTM_RET		xRet = FTM_RET_OK;
	FILE*		pFP;
#if 0
	FTM_UINT32	ulProcessCount = 0;
	
	xRet = FTM_getProcessCount(pProcessName, &ulProcessCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get process count!");	
		return	xRet;
	}

	*pDuplicated = (ulProcessCount != 1);
#endif
	memset(pCmd, 0, sizeof(pCmd));
	snprintf(pCmd, sizeof(pCmd) - 1, "ps -a | egrep %d | awk '{if($1 ~ %d) print \"found\"}'", ulPID, ulPID);
	pFP = popen(pCmd, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to execute shell command[%s]", pCmd);

		*pDuplicated = FTM_FALSE;
	}
	else
	{
		INFO("pCmd : %s", pCmd);
		memset(pCmd, 0, sizeof(pCmd));
		fgets(pCmd, sizeof(pCmd), pFP);	
		pclose(pFP);

		INFO("pCmd : %s", pCmd);
		*pDuplicated = (strncasecmp(pCmd, "found", 5) == 0);
	}

	
	return	xRet;
}

FTM_RET	FTM_checkPIDFile
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32		ulPID,
	FTM_BOOL_PTR	pGood
)
{
	ASSERT(pProcessName != NULL);
	ASSERT(pGood != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN];
	FILE*		pFP;
	FTM_UINT32	ulReadPID = 0;

	sprintf(pFileName, "/var/run/%s.pid", pProcessName);

	pFP = fopen(pFileName, "r");
	if (pFP == NULL)
	{
		*pGood = FTM_FALSE;	
	}
	else
	{
    	fscanf(pFP, "%d", &ulReadPID);
		fclose(pFP);

		*pGood = (ulPID == ulReadPID);
	}

	return	xRet;
}

FTM_RET	FTM_createPIDFile
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32		ulPID
)
{
	ASSERT(pProcessName != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN];
	FILE*		pFP;

	sprintf(pFileName, "/var/run/%s.pid", pProcessName);

	pFP = fopen(pFileName, "w");
	if (pFP == NULL)
	{
		xRet = FTM_RET_FILE_OPEN_FAILED;
		ERROR(xRet, "Failed to open file[%s]", pFileName);
	}
	else
	{
		fprintf(pFP, "%d", ulPID);
		fclose(pFP);
	}

	return	xRet;
}


FTM_RET	FTM_destroyPIDFile
(
	FTM_CHAR_PTR	pProcessName
)
{
	ASSERT(pProcessName != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pCmd[FTM_PATH_LEN + FTM_FILE_NAME_LEN];
	FILE*		pFP;

	snprintf(pCmd, sizeof(pCmd), "rm -f /var/run/%s.pid", pProcessName);

	pFP = popen(pCmd, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_EXECUTE_FAILED;
		ERROR(xRet, "Failed to execute [%s]", pCmd);
	}
	else
	{
		pclose(pFP);
	}

	return	xRet;
}


FTM_RET	FTM_ReadPID
(
	FTM_CHAR_PTR	pProcessName,
	FTM_UINT32_PTR	pulPID
)
{
	ASSERT(pProcessName != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN];
	FILE*		pFP;
	FTM_UINT32	ulReadPID = 0;

	sprintf(pFileName, "/var/run/%s.pid", pProcessName);

	pFP = fopen(pFileName, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_FILE_OPEN_FAILED;
	}
	else
	{
    	fscanf(pFP, "%d", &ulReadPID);
		fclose(pFP);

		*pulPID = ulReadPID;
		INFO("PID : %d", ulReadPID);
	}

	return	xRet;
}

FTM_RET	FTM_getCPUUtilization
(
	FTM_FLOAT_PTR	pUtilization
) 
{
	FTM_CHAR	pBuffer[128];
    FILE *pFP;

	memset(pBuffer, 0, sizeof(pBuffer));
	sprintf(pBuffer, "0");

#if MODEL == ftm-50
	pFP = popen("mpstat | tail -1 | awk '{print 100-$11}'", "r");
#elif MODEL == i686
	pFP = popen("mpstat | tail -1 | awk '{print 100-$13}'", "r");
#else
	#error "Model undefined"
#endif
    if(pFP != NULL) 
	{
        fgets(pBuffer, sizeof(pBuffer) - 1, pFP);
    	pclose(pFP);
    }

	*pUtilization = strtod(pBuffer, NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_getMemoryUtilization
(
	FTM_UINT32_PTR	pTotalMemory,
	FTM_UINT32_PTR	pFreeMemory
) 
{
	FTM_CHAR	pBuffer[128];
    FILE *pFP;
	FTM_UINT32	ulTotal = 0;
	FTM_UINT32	ulFree	= 0;

	memset(pBuffer, 0, sizeof(pBuffer));
	sprintf(pBuffer, "0");

	pFP = popen("cat /proc/meminfo | grep Mem | awk '{print $2}'", "r");
    if(pFP != NULL) 
	{
        if (fgets(pBuffer, sizeof(pBuffer) - 1, pFP) != NULL)
		{
			ulTotal = strtoul(pBuffer, 0, 10);
        	if (fgets(pBuffer, sizeof(pBuffer) - 1, pFP) != NULL)
			{
				ulFree = strtoul(pBuffer, 0, 10);
			}
		}
    	pclose(pFP);
    }

	*pTotalMemory = ulTotal;
	*pFreeMemory = ulFree;

	return	FTM_RET_OK;
}

FTM_RET	FTM_getNetStatistics
(
	FTM_NET_STATISTICS_PTR	pStatistics
)
{
	ASSERT(pStatistics != NULL);

    FILE *pFP;
	FTM_UINT32	ulRxBytes = 0;
	FTM_UINT32	ulTxBytes = 0;
	FTM_CHAR	pBuffer[256];

	pFP = popen("cat /proc/net/dev | awk '{ if ($1 ~ /:/) print $1, $2, $10}' | sed 's/://'", "r");
	while(fgets(pBuffer, sizeof(pBuffer) - 1, pFP) != NULL)
	{
		if (strncmp(pBuffer, "lo", 2) != 0)
		{
			FTM_CHAR	pName[64];
			FTM_UINT32	ulRx = 0;
			FTM_UINT32	ulTx = 0;
			sscanf(pBuffer, "%s %d %d", pName, &ulRx, &ulTx);

			ulRxBytes += ulRx;
			ulTxBytes += ulTx;
		}
	}
	pclose(pFP);

	INFO("RxBytes : %d, TxBytes : %d", ulRxBytes, ulTxBytes);
	pStatistics->ulRxBytes = ulRxBytes;
	pStatistics->ulTxBytes = ulTxBytes;

	return	FTM_RET_OK;
}

FTM_RET	FTM_getDiskUtilization
(
	FTM_DISK_STATISTICS_PTR	pInfo
) 
{
	FTM_CHAR	pBuffer[128];
    FILE *pFP;
	FTM_CHAR	pName[64];
	FTM_CHAR	pRate[64];

	memset(pInfo, 0, sizeof(FTM_DISK_STATISTICS));

	memset(pBuffer, 0, sizeof(pBuffer));
	sprintf(pBuffer, "0");

	pInfo->ulCount = 0;
	pFP = popen("df | awk '{ if ($1 != \"Filesystem\") print $1, $5}'", "r");
    if(pFP != NULL) 
	{
        while(fgets(pBuffer, sizeof(pBuffer) - 1, pFP) != NULL)
		{
			if (sscanf(pBuffer, "%s %s", pName, pRate) == 2)
			{
				strncpy(pInfo->xPartitions[pInfo->ulCount].pName, pName, sizeof(pInfo->xPartitions[pInfo->ulCount].pName) - 1);
				strncpy(pInfo->xPartitions[pInfo->ulCount].pRate, pRate, sizeof(pInfo->xPartitions[pInfo->ulCount].pRate) - 1);

				pInfo->ulCount++;
			}
		}
    	pclose(pFP);
    }

	return	FTM_RET_OK;
}
FTM_RET	FTM_getStatistics
(
	FTM_STATISTICS_PTR	pStatistics
)
{
	ASSERT(pStatistics != NULL);

	FTM_TIME_getCurrent(&pStatistics->xTime);
	FTM_getCPUUtilization(&pStatistics->fCPU);
	FTM_getMemoryUtilization(&pStatistics->xMemory.ulTotal, &pStatistics->xMemory.ulFree);
//	FTM_getDiskUtilization(&pStatistics->xDisk);
	FTM_getNetStatistics(&pStatistics->xNet);

	return	FTM_RET_OK;
}

FTM_RET	FTM_getBootTime
(
	FTM_TIME_PTR	pTime
)
{
	ASSERT(pTime != NULL);

	FTM_CHAR	pBuffer[128];
	FTM_TIME	xTime;
	FTM_UINT32	ulUPTime = 0;
    FILE *pFP;

	FTM_TIME_getCurrent(&xTime);
	pFP = popen("cat /proc/uptime | awk '{print $1}'", "r");
    if(pFP != NULL) 
	{
        if (fgets(pBuffer, sizeof(pBuffer) - 1, pFP) != NULL)
		{
			
			ulUPTime = (FTM_UINT32)strtod(pBuffer, NULL);
		}
    	pclose(pFP);
    }


	return	FTM_TIME_subSecs(&xTime, ulUPTime, pTime);
}

FTM_RET	FTM_setTime
(
	time_t	xTime
)
{
	FTM_CHAR	pBuffer[128];
	FTM_CHAR	pTimeString[32];
	struct tm	xTM;
    FILE *pFP;

	gmtime_r(&xTime, &xTM);

	strftime(pTimeString, sizeof(pTimeString), "%Y%m%d%H%M%S", &xTM);
	sprintf(pBuffer, "date -s %s;hwclock -w", pTimeString);

	pFP = popen(pBuffer, "r");
    if(pFP == NULL) 
	{
		return	FTM_RET_ERROR;
	}
	pclose(pFP);

	return	FTM_RET_OK;
}

FTM_RET	FTM_getTime
(
	time_t	*pTime
)
{
	ASSERT(pTime != NULL);

    *pTime = time(NULL);

	return	FTM_RET_OK;
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

FTM_UINT8	pEncryptKey[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,0x0d, 0x0f};
FTM_UINT8	pPasswdIV[16] 	= { 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd'};
FTM_UINT8	pUserIDIV[16] 	= { 'u', 's', 'e', 'r', 'i', 'd', '0', '0','u', 's', 'e', 'r', 'i', 'd', '0', '0' };

FTM_RET	FTM_encryptPasswd
(
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
)
{
	return	FTM_encrypt(pPasswdIV, pPasswd, ulPasswdLen, pBuffer, ulBufferLen);
}

FTM_RET	FTM_decryptPasswd
(
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen,
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen
)
{
	return	FTM_decrypt(pPasswdIV, pBuffer, ulBufferLen, pPasswd, ulPasswdLen);
}
FTM_RET	FTM_encryptUserID
(
	FTM_CHAR_PTR	pUserID,
	FTM_UINT32		ulUserIDLen,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
)
{
	return	FTM_encrypt(pUserIDIV, pUserID, ulUserIDLen, pBuffer, ulBufferLen);
}

FTM_RET	FTM_decryptUserID
(
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen,
	FTM_CHAR_PTR	pUserID,
	FTM_UINT32		ulUserIDLen
)
{
	return	FTM_decrypt(pUserIDIV, pBuffer, ulBufferLen, pUserID, ulUserIDLen);
}

FTM_RET	FTM_encrypt
(
	FTM_UINT8		pIV[16],
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
)
{
	FTM_UINT32	ulBlockLen = (ulPasswdLen + 15) / 16 * 16;
	FTM_UINT8	pBlock[128];
	FTM_UINT8	pEncryptedBlock[128];

	if (ulBufferLen <= ulBlockLen * 2)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	memset(pBlock, 0, ulBlockLen);
	memset(pEncryptedBlock, 0, ulBlockLen);
	memcpy(pBlock, pPasswd, ulPasswdLen);

	AES_CBC_encrypt_buffer(pEncryptedBlock, pBlock, ulBlockLen, pEncryptKey, pIV);

	for(FTM_UINT32	i = 0 ; i < ulBlockLen ; i++)
	{
		sprintf(&pBuffer[i*2], "%02x", pEncryptedBlock[i]);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_decrypt
(
	FTM_UINT8		pIV[16],
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen,
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen
)
{
	FTM_UINT32	ulBlockLen = ulBufferLen / 2;
	FTM_UINT8	pBlock[128];
	FTM_UINT8	pDecryptedBlock[128];
	
	memset(pBlock, 0, ulBlockLen);
	memset(pDecryptedBlock, 0, ulBlockLen);
	if (ulBufferLen % 32 != 0)
	{
		printf("Invalid block[%d]!\n", ulBufferLen);
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	for(FTM_UINT32	i = 0 ; i < ulBlockLen ; i++)
	{
		if ('0' <= pBuffer[i*2] && pBuffer[i*2] <= '9')
		{
			pBlock[i] = (pBuffer[i*2] - '0') << 4;
		}
		else if ('a' <= pBuffer[i*2] && pBuffer[i*2] <= 'f')
		{
			pBlock[i] = (pBuffer[i*2] - 'a' + 10) << 4;
		}
		else if ('A' <= pBuffer[i*2] && pBuffer[i*2] <= 'F')
		{
			pBlock[i] = (pBuffer[i*2] - 'A' + 10) << 4;
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}

		if ('0' <= pBuffer[i*2+1] && pBuffer[i*2+1] <= '9')
		{
			pBlock[i] |= (pBuffer[i*2+1] - '0');
		}
		else if ('a' <= pBuffer[i*2+1] && pBuffer[i*2+1] <= 'f')
		{
			pBlock[i] |= (pBuffer[i*2+1] - 'a' + 10);
		}
		else if ('A' <= pBuffer[i*2+1] && pBuffer[i*2+1] <= 'F')
		{
			pBlock[i] |= (pBuffer[i*2+1] - 'A' + 10);
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}


	AES_CBC_decrypt_buffer(pDecryptedBlock, pBlock, ulBlockLen, pEncryptKey, pIV);
	
	strncpy(pPasswd, (FTM_CHAR_PTR)pDecryptedBlock, ulPasswdLen);

	return	FTM_RET_OK;
}

