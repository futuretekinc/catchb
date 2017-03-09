#include <ctype.h>
#include <common.h>
#include <syslog.h>
#include <signal.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <fcntl.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <termios.h>
#include <unistd.h>
#include <pty.h>

#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_utils.h"
#include "ftm_switch.h"
#include "ftm_telnet.h"

typedef	enum	
{
	FTM_SWS_CMD_TYPE_USER_ID = 0,
	FTM_SWS_CMD_TYPE_PASSWD,
	FTM_SWS_CMD_TYPE_GENERAL,
	FTM_SWS_CMD_TYPE_PERMIT_L2T,
	FTM_SWS_CMD_TYPE_PERMIT_T2L,
	FTM_SWS_CMD_TYPE_DENY_TARGET,
	FTM_SWS_CMD_TYPE_PERMIT_ALL,
	FTM_SWS_CMD_TYPE_FINISHED
}	FTM_SWS_CMD_TYPE, _PTR_ FTM_SWS_CMD_TYPE_PTR;

typedef	struct
{
	FTM_SWS_CMD_TYPE	xType;
	FTM_CHAR_PTR		pPrompt;
	FTM_CHAR_PTR		pInput;
	FTM_UINT32			ulDelay;
	FTM_UINT32			pNext[8];
	FTM_UINT32			ulNextCount;
}	FTM_SWS_CMD, _PTR_ FTM_SWS_CMD_PTR;

typedef	struct
{
	FTM_SWS_CMD	pCommands[32];
}	FTM_SWITCH_SCRIPT, _PTR_ FTM_SWITCH_SCRIPT_PTR;

FTM_SWITCH_SCRIPT	xAllowScript =
{
	.pCommands = 
	{
		[0]	= {	.xType	= FTM_SWS_CMD_TYPE_USER_ID, 	.pPrompt= "Username:",			 		.ulDelay = 1, .pInput = NULL, .pNext  = {1, 0}},
		[1]	= {	.xType	= FTM_SWS_CMD_TYPE_PASSWD,  	.pPrompt= "Password:",					.ulDelay = 1, .pInput = NULL,	.pNext 	= {2, 0} },
		[2]	= {	.xType	= FTM_SWS_CMD_TYPE_GENERAL,		.pPrompt= "Switch#", 					.ulDelay = 1, .pInput = "configure", .pNext = {3, 0}},
		[3]	= {	.xType	= FTM_SWS_CMD_TYPE_GENERAL,		.pPrompt= "Switch(config)#", 			.ulDelay = 1, .pInput = "interface range GigabitEthernet 1-28", .pNext = {4, }},
		[4]	= {	.xType	= FTM_SWS_CMD_TYPE_GENERAL,		.pPrompt= "Switch(config-if-range)#", 	.ulDelay = 1, .pInput = "no ip acl",	.pNext = {5, 0}},
		[5]	= {	.xType	= FTM_SWS_CMD_TYPE_GENERAL,		.pPrompt= "Switch(config-if-range)#", 	.ulDelay = 1, .pInput = "exit",	.pNext = {6, 0}},
		[6]	= {	.xType	= FTM_SWS_CMD_TYPE_GENERAL,		.pPrompt= "Switch(config)#", 			.ulDelay = 1, .pInput = "ip acl catchb_filter", .pNext = {7, 0}},
		[7]	= {	.xType	= FTM_SWS_CMD_TYPE_PERMIT_L2T,	.pPrompt= "Switch(config-ip-acl)#",		.ulDelay = 2, .pInput = NULL,	.pNext = {8, 0}},
		[8]	= {	.xType	= FTM_SWS_CMD_TYPE_PERMIT_T2L,	.pPrompt= "Switch(config-ip-acl)#",		.ulDelay = 2, .pInput = NULL, .pNext = {9, 0}},
		[9]	= {	.xType	= FTM_SWS_CMD_TYPE_DENY_TARGET,	.pPrompt= "Switch(config-ip-acl)#",		.ulDelay = 2, .pInput = NULL, .pNext = {10, 0}},
		[10]= {	.xType	= FTM_SWS_CMD_TYPE_PERMIT_ALL,	.pPrompt= "Switch(config-ip-acl)#",		.ulDelay = 2, .pInput = NULL, .pNext = {11, 0}},
		[11]= {	.xType	= FTM_SWS_CMD_TYPE_GENERAL,		.pPrompt= "Switch(config-ip-acl)#",		.ulDelay = 2, .pInput = "exit", .pNext = {12, 0}},
		[12]= {	.xType	= FTM_SWS_CMD_TYPE_GENERAL,		.pPrompt= "Switch(config)#", 			.ulDelay = 1, .pInput = "exit", .pNext = {13, 0}},
		[13]= {	.xType	= FTM_SWS_CMD_TYPE_GENERAL,		.pPrompt= "Switch#", 					.ulDelay = 1, .pInput = "exit", .pNext = {14, 0}},
		[14]= {	.xType	= FTM_SWS_CMD_TYPE_FINISHED,	.pPrompt= NULL, 						.ulDelay = 1, .pInput = NULL},
	}
};

#define	NST_MAX_COMD	32

FTM_RET	FTM_SWITCH_NST_setAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWITCH_AC_POLICY	xPolicy
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pLocalIP[FTM_IP_LEN+1];
	FTM_CHAR	pTelnetCommand[64];
	FTM_UINT32	ulIndex;
    FILE 		* pFile;
    FTM_CHAR	pCommandBuffers[NST_MAX_COMD][FTM_COMMAND_LEN];
	FTM_UINT32	ulCommandLines = 0;
	FTM_INT		hFile;
	FTM_INT		i;
	
	FTM_getLocalIP(pLocalIP, sizeof(pLocalIP));
	ulIndex = ntohl(inet_addr(pTargetIP)) & 0xFFFFFF;

	switch(xPolicy)
	{
	case	FTM_SWITCH_AC_POLICY_DENY:
		{
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", pSwitch->xConfig.pUserID);
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", pSwitch->xConfig.pPasswd);
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "configure");

			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "interface range GigabitEthernet 1-28");
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "no ip acl");
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "exit");

			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "ip acl catchb_filter");
			sprintf(pCommandBuffers[ulCommandLines++],"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255\n", ulIndex*3, pLocalIP, pTargetIP);
			sprintf(pCommandBuffers[ulCommandLines++],"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255\n", ulIndex*3+1,pTargetIP, pLocalIP);
			sprintf(pCommandBuffers[ulCommandLines++],"sequence %d deny   ip %s/255.255.255.255 any\n",ulIndex*3+2, pTargetIP);
			sprintf(pCommandBuffers[ulCommandLines++],"sequence %d permit ip any any\n", 2147483640);
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "exit");

			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "interface range GigabitEthernet 1-28");
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "ip acl catchb_filter");
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "exit");

			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "exit");
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "exit");
		}
		break;

	case	FTM_SWITCH_AC_POLICY_ALLOW:	
		{
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n",pSwitch->xConfig.pUserID);
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n",pSwitch->xConfig.pPasswd);
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "configure");

			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "interface range GigabitEthernet 1-28");
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "no ip acl");
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "exit");

			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "ip acl catchb_filter");
			sprintf(pCommandBuffers[ulCommandLines++],"no sequence %d\n",ulIndex*3);
			sprintf(pCommandBuffers[ulCommandLines++],"no sequence %d\n",ulIndex*3+1);
			sprintf(pCommandBuffers[ulCommandLines++],"no sequence %d\n",ulIndex*3+2);
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "exit");

			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "interface range GigabitEthernet 1-28");
			sprintf(pCommandBuffers[ulCommandLines++],"%s\n", "ip acl catchb_filter");

			sprintf(pCommandBuffers[ulCommandLines++],"end\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");
		}
		break;
	}



    sprintf(pTelnetCommand, "telnet %s >> /tmp/switch_nst.log", pSwitch->xConfig.pIP);

    pFile = popen(pTelnetCommand, "w");
	if (pFile == NULL)
	{
		xRet = FTM_RET_ERROR;
		goto error;
	}

	hFile = fileno(pFile);
	for(i = 0 ; i < 16 ; i++)
	{
		write(hFile, pCommandBuffers[i], strlen(pCommandBuffers[i]));
		usleep( 1000 * 700 );
	}

	pclose(pFile);

error:
	return	xRet;
}


FTM_RET	FTM_SWITCH_NST_accessControl2
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_BOOL		bAllow
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_RET	xRet;
	FTM_CHAR	pLocalIP[32];
	FTM_TELNET_CLIENT_PTR	pClient;
	FTM_UINT32	ulCommandLine = 0;
	FTM_UINT32	ulIndex;

	FTM_getLocalIP(pLocalIP, 32);
	
	ulIndex = inet_addr(pTargetIP) & 0xFFFFFF;

	xRet = FTM_TELNET_CLIENT_create(&pClient);
	if (xRet != FTM_RET_OK)
	{
		printf("Failed to create client!\n");	
		goto finished;
	}

	xRet = FTM_TELNET_CLIENT_open(pClient, pSwitch->xConfig.pIP, 23);
	if (xRet == FTM_RET_OK)
	{
		printf("telnet open success!\n");	

		while(1)
		{
			FTM_CHAR	pCommandBuffer[512];
			FTM_CHAR	pBuffer[512];
			FTM_UINT32	ulCommandBufferLen;
			FTM_UINT32	ulReadLen;
			FTM_BOOL	bFinished = FTM_FALSE;
			sleep(xAllowScript.pCommands[ulCommandLine].ulDelay);

			memset(pBuffer, 0, sizeof(pBuffer));

			xRet = FTM_TELNET_CLIENT_readline(pClient, pBuffer, sizeof(pBuffer), &ulReadLen);
			if ((!bFinished) && (xRet == FTM_RET_OK) && (ulReadLen != 0) && isprint(pBuffer[0]))
			{
				INFO("READ : %s", pBuffer);
				if (strncasecmp(pBuffer, xAllowScript.pCommands[ulCommandLine].pPrompt, strlen(xAllowScript.pCommands[ulCommandLine].pPrompt)) == 0)
				{
					switch(xAllowScript.pCommands[ulCommandLine].xType)
					{
					case	FTM_SWS_CMD_TYPE_USER_ID:
						{
							FTM_TELNET_CLIENT_write(pClient, pSwitch->xConfig.pUserID, strlen(pSwitch->xConfig.pUserID));
							FTM_TELNET_CLIENT_write(pClient, "\n", 1);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_PASSWD:
						{
							FTM_TELNET_CLIENT_write(pClient, pSwitch->xConfig.pPasswd, strlen(pSwitch->xConfig.pPasswd));
							FTM_TELNET_CLIENT_write(pClient, "\n", 1);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_GENERAL:
						{
							FTM_TELNET_CLIENT_write(pClient, xAllowScript.pCommands[ulCommandLine].pInput, strlen(xAllowScript.pCommands[ulCommandLine].pInput));
							FTM_TELNET_CLIENT_write(pClient, "\n", 1);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_PERMIT_L2T:
						{
							ulCommandBufferLen = sprintf(pCommandBuffer,"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255\n", ulIndex*3, pLocalIP, pTargetIP);
							FTM_TELNET_CLIENT_write(pClient, pCommandBuffer, ulCommandBufferLen);
							FTM_TELNET_CLIENT_write(pClient, "\n", 1);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_PERMIT_T2L:
						{
							ulCommandBufferLen = sprintf(pCommandBuffer,"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255\n", ulIndex*3+1,pTargetIP, pLocalIP);
							FTM_TELNET_CLIENT_write(pClient, pCommandBuffer, ulCommandBufferLen);
							FTM_TELNET_CLIENT_write(pClient, "\n", 1);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_DENY_TARGET:
						{
							ulCommandBufferLen = sprintf(pCommandBuffer,"sequence %d deny   ip %s/255.255.255.255 any\n",ulIndex*3+2, pTargetIP);
							FTM_TELNET_CLIENT_write(pClient, pCommandBuffer, ulCommandBufferLen);
							FTM_TELNET_CLIENT_write(pClient, "\n", 1);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_PERMIT_ALL:
						{
							ulCommandBufferLen = sprintf(pCommandBuffer,"sequence %d permit ip any any\n", 2147483640);
							FTM_TELNET_CLIENT_write(pClient, pCommandBuffer, ulCommandBufferLen);
							FTM_TELNET_CLIENT_write(pClient, "\n", 1);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_FINISHED:
						{
							bFinished = FTM_TRUE;
						}
						break;
					}
				}
			}
		}

		xRet = FTM_TELNET_CLIENT_close(pClient);
		if (xRet != FTM_RET_OK)
		{
			printf("Failed to close telnet!\n");	
		}
	}
	else
	{
		printf("telnet open failed!\n");	
	}

	xRet = FTM_TELNET_CLIENT_destroy(&pClient);
	if (xRet != FTM_RET_OK)
	{
		printf("Failed to destroy telnet clinet!\n");
	}

finished:

	return	xRet;
}
