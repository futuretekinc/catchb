#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>

#include "ftm_utils.h"
#include "ftm_trace.h"
#include "libsha1.h"

FTM_RET	FTM_getLocalIP
(
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffSize
) 
{
	ASSERT(pBuff != NULL);

	FTM_RET	xRet = FTM_RET_OK;
    FILE *pFP;
	
	pFP = popen("ifconfig eth0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'", "r");
    if(pFP != NULL) 
	{
        if (fgets(pBuff, ulBuffSize, pFP) == NULL)
		{
			xRet = FTM_RET_NET_INTERFACE_ERROR;
		}
    	pclose(pFP);
    }
	else
	{
		xRet = FTM_RET_NET_INTERFACE_ERROR;	
	}

	if (xRet == FTM_RET_OK)
	{
		FTM_INT32	i;

		for(i = 0 ; i < strlen(pBuff) ; i++)
		{
			if (!isprint(pBuff[i]))
			{
				pBuff[i] = '\0';	
			}
		}
	}

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
        for(i = 0; i < 5 ; i++)
        {
            ulLen += snprintf(&pValue[ulLen], ulBufferSize - ulLen, "%x", xSHA.Message_Digest[i]);
        }

        for(i =0 ; i < ulLen; i++)
		{
            if(!isdigit(pValue[i]))
			{
                if(pValue[i] >= 'a')
				{
                    pValue[i] = pValue[i]-32;
				}
                else 
				{
                    pValue[i] = pValue[i]+32;
				}

            }
        }

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
#if 0
	FTM_RET		xRet;
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
	snprintf(pCmd, sizeof(pCmd) - 1, "ps -ax | egrep %d | awk '{if($1 ~ %d) print \"found\"}'", ulPID, ulPID);

	*pDuplicated = (strcasecmp(pCmd, "found") == 0);
	
	return	FTM_RET_OK;
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

	pFP = popen("mpstat | tail -1 | awk '{print 100-$13}'", "r");
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

