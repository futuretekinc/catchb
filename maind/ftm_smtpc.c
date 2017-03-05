/*
 * smtpc.c - SMTP stuff for libsmtpc.
 *
 * (C) 2006 Carlos Eduardo Medaglia Dyonisio <cadu(a)nerdfeliz.com>
 *          http://nerdfeliz.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version
 * 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

#include "ftm_smtpc.h"
#include "ftm_base64.h"
#include "ftm_trace.h"
#include "ftm_mem.h"

FTM_RET	FTM_SMTPC_CONNECTION_open
(	
	FTM_CHAR_PTR	pHostName, 
	FTM_UINT16		usPort,
	FTM_INT_PTR		pSock
);

FTM_RET	FTM_SMTPC_CONNECTION_read
(
	FTM_INT			nSock,
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_UINT32_PTR	pulDataLen
);

FTM_RET	FTM_SMTPC_CONNECTION_write
(
	FTM_INT			nSock,
	FTM_CHAR_PTR	pData,
	FTM_UINT32		ulDataLen
);

FTM_RET	FTM_SMTPC_create
(
	FTM_CHAR_PTR	pHostName,
	FTM_UINT16		usPort,
	FTM_SMTPC_PTR _PTR_ ppSMTPC
)
{
	ASSERT(pHostName != NULL);
	ASSERT(ppSMTPC != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_SMTPC_PTR	pSMTPC;

	pSMTPC = (FTM_SMTPC_PTR)FTM_MEM_malloc(sizeof(FTM_SMTPC));
	if (pSMTPC == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create SMTPC!");
		return	xRet;
	}

	strncpy(pSMTPC->pHostName, pHostName, FTM_HOST_NAME_LEN);
	pSMTPC->usPort = (usPort)?usPort:25;

	*ppSMTPC = pSMTPC;

	return FTM_RET_OK;
}

FTM_RET	FTM_SMTPC_destroy
(
	FTM_SMTPC_PTR _PTR_ ppSMTPC
)
{
	ASSERT(ppSMTPC != NULL);

	FTM_MEM_free(*ppSMTPC);

	*ppSMTPC = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SMTPC_connect
(
	FTM_SMTPC_PTR	pSMTPC
)
{
	FTM_RET		xRet;
	FTM_CHAR	pBuff[256];
	FTM_UINT32	ulDataLen;

	xRet = FTM_SMTPC_CONNECTION_open(pSMTPC->pHostName, pSMTPC->usPort, &pSMTPC->xSock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to connect port!");
		return	xRet;	
	}

	xRet = FTM_SMTPC_CONNECTION_read(pSMTPC->xSock, pBuff, sizeof(pBuff), &ulDataLen);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to read data from socket!");
		return	xRet;	
	}

	TRACE("S : %s", pBuff);

	pSMTPC->nCode = atoi(pBuff);

	return FTM_RET_OK;
}

FTM_RET	FTM_SMTPC_sendAndReceive
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pData
)
{
	ASSERT(pSMTPC != NULL);
	ASSERT(pData != NULL);
	FTM_CHAR	pBuff[256];
	FTM_UINT32	ulReadDataLen;

	FTM_RET	xRet = FTM_RET_OK;

	TRACE("C : %s", pData);
	xRet = FTM_SMTPC_CONNECTION_write(pSMTPC->xSock, pData, strlen(pData));
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to write data to socket!");
		return	xRet;	
	}
	
	xRet = FTM_SMTPC_CONNECTION_read(pSMTPC->xSock, pBuff, sizeof(pBuff), &ulReadDataLen);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to read data from socket!");
		return	xRet;	
	}

	TRACE("S : %s", pBuff);
	pSMTPC->nCode = atoi(pBuff);

	return FTM_RET_OK;
}

FTM_RET	FTM_SMTPC_sendGreeting
(
	FTM_SMTPC_PTR	pSMTPC
)
{
	ASSERT(pSMTPC != NULL);
	FTM_RET		xRet;
	FTM_CHAR	pBuff[256];

	sprintf(pBuff, "EHLO %s\r\n", pSMTPC->pHostName);

	xRet = FTM_SMTPC_sendAndReceive(pSMTPC, pBuff);
	if (xRet == FTM_RET_OK)
	{
		if (pSMTPC->nCode != FTM_SMTP_RET_COMPLETED)
		{
			xRet = FTM_RET_SMTPC_ERROR;
		}
	}

	return	xRet;
}

FTM_RET	FTM_SMTPC_sendAuth
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pUserID,
	FTM_CHAR_PTR	pPasswd
)
{
	ASSERT(pSMTPC != NULL);
	ASSERT(pUserID != NULL);
	ASSERT(pPasswd != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pBuff[1024];
	FTM_UINT32	ulEncodedIDLen = 0;
	FTM_CHAR	pEncodedID[1024];
	FTM_UINT32	ulEncodedPWLen = 0;
	FTM_CHAR	pEncodedPW[1024];

	memset(pEncodedID, 0, sizeof(pEncodedID));
	memset(pEncodedPW, 0, sizeof(pEncodedPW));

	xRet = FTM_BASE64_encode((FTM_UINT8_PTR)pUserID, strlen(pUserID), pEncodedID, sizeof(pEncodedID), &ulEncodedIDLen);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to encode ID!");	
		return	xRet;
	}

	xRet = FTM_BASE64_encode((FTM_UINT8_PTR)pPasswd, strlen(pPasswd), pEncodedPW, sizeof(pEncodedPW), &ulEncodedPWLen);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to encode PW!");	
		return	xRet;
	}

	sprintf(pBuff, "AUTH LOGIN %s\r\n", pEncodedID);
	xRet = FTM_SMTPC_sendAndReceive(pSMTPC, pBuff);
	if (xRet == FTM_RET_OK)
	{
		if (pSMTPC->nCode != FTM_SMTP_RET_BASE64_ENCODED_STRING)
		{
			xRet = FTM_RET_SMTPC_ERROR;
			ERROR(xRet, "Failed to encoding string");
		}
	}

	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to send ID and receive!");
		return	xRet;
	}
	
	sprintf(pBuff, "%s\r\n", pEncodedPW);
	xRet = FTM_SMTPC_sendAndReceive(pSMTPC, pBuff);
	if (xRet == FTM_RET_OK)
	{
		if (pSMTPC->nCode != FTM_SMTP_RET_AUTHENTICATION_SUCCEEDED)
		{
			ERROR(xRet, "Failed to send PW and receive!");
		}
	}

	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to send ID and receive!");
	}

	return	xRet;
}

FTM_RET	FTM_SMTPC_sendCmdAndArg
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pCmd,
	FTM_CHAR_PTR	pArg
)
{
	ASSERT(pSMTPC != NULL);
	ASSERT(pCmd != NULL);
	ASSERT(pArg != NULL);

	FTM_CHAR	pBuff[1024];

	snprintf(pBuff, sizeof(pBuff), "%s %s\r\n", pCmd, pArg);

	return	FTM_SMTPC_sendAndReceive(pSMTPC, pBuff);
}

/* TODO: inline */
FTM_RET	FTM_SMTPC_sendTo
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pTo
)
{
	return	FTM_SMTPC_sendCmdAndArg(pSMTPC, "RCPT TO:", pTo);
}

FTM_RET	FTM_SMTPC_sendFrom
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pFrom
)
{
	return	FTM_SMTPC_sendCmdAndArg(pSMTPC, "MAIL FROM:", pFrom);
}

FTM_RET	FTM_SMTPC_startBody
(
	FTM_SMTPC_PTR	pSMTPC
)
{
	FTM_CHAR_PTR	pData = "DATA\r\n";
	return	FTM_SMTPC_sendAndReceive(pSMTPC, pData);
}

FTM_RET	FTM_SMTPC_sendRawBody
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pData
)
{
	return	FTM_SMTPC_CONNECTION_write(pSMTPC->xSock, pData, strlen(pData));
}

FTM_RET	FTM_SMTPC_endBody
(
	FTM_SMTPC_PTR	pSMTPC
)
{
	FTM_CHAR_PTR	pData = "\r\n.\r\n";
	return	FTM_SMTPC_sendAndReceive(pSMTPC, pData);
}

FTM_RET	FTM_SMTPC_sendQuit
(
	FTM_SMTPC_PTR	pSMTPC
)
{
	FTM_CHAR_PTR	pData = "QUIT\r\n";
	return	FTM_SMTPC_sendAndReceive(pSMTPC, pData);
}

FTM_RET	FTM_SMTPC_disconnect
(
	FTM_SMTPC_PTR	pSMTPC
)
{
	ASSERT(pSMTPC != NULL);

	close(pSMTPC->xSock);

	pSMTPC->xSock = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SMTPC_CONNECTION_open
(
	FTM_CHAR_PTR	pHostName, 
	FTM_UINT16		usPort,
	FTM_INT_PTR		pSock
)
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	xSock;
	struct hostent *pHost;
	struct sockaddr_in xAddr;

	xSock = socket(PF_INET, SOCK_STREAM, 0);
	if(xSock < 0)
	{
		xRet = FTM_RET_SOCKET_CREATION_FAILED;
		return	xRet;
	}

	pHost = gethostbyname(pHostName);

	xAddr.sin_family = AF_INET;
	xAddr.sin_addr = *(struct in_addr *)pHost->h_addr;
	xAddr.sin_port = htons(usPort);

	connect(xSock, (struct sockaddr *)&xAddr, sizeof(xAddr));

	*pSock = xSock;

	return xRet;
}

FTM_RET	FTM_SMTPC_CONNECTION_read
(
	FTM_INT			xSock,
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_UINT32_PTR	pDataLen
)
{
	FTM_RET	xRet = FTM_RET_OK;
	struct pollfd xFDS;
	FTM_INT	nRet;

	xFDS.fd = xSock;
	xFDS.events = POLLIN | POLLPRI;
	
	nRet = poll(&xFDS, 1, 120000);
	if(nRet == 0)
	{
		*pDataLen = 0;
		xRet = FTM_RET_TIMEOUT;
		return xRet;
	}

	if(xFDS.revents & POLLIN || xFDS.revents & POLLPRI) 
	{
		FTM_INT	nRcvdLen = 0;
		FTM_CHAR	pTempBuff[2048];

		// XXX: Should still be changed... (2048 bytes)
		nRcvdLen = recv(xSock, pTempBuff, sizeof(pTempBuff), 0);
		if (ulBuffLen < nRcvdLen)
		{
			memcpy(pBuff, pTempBuff, ulBuffLen);
			*pDataLen = ulBuffLen;
		}
		else
		{
			memcpy(pBuff, pTempBuff, nRcvdLen);
			*pDataLen = nRcvdLen;
		}

		return FTM_RET_OK;
	}

	return FTM_RET_ERROR;
}

FTM_RET	FTM_SMTPC_CONNECTION_write
(
	FTM_INT			xSock,
	FTM_CHAR_PTR	pData,
	FTM_UINT32		ulDataLen
)
{
	ASSERT(pData != NULL);

	FTM_UINT32	ulSentLen = 0;

	while(ulSentLen < ulDataLen)
	{
		FTM_INT32	nWriteLen;

		nWriteLen = write(xSock, &pData[ulSentLen], ulDataLen - ulSentLen);
		if(nWriteLen < 0)
		{
			return	FTM_RET_ERROR;	
		}

		ulSentLen += nWriteLen;
	}

	return	FTM_RET_OK;
}

