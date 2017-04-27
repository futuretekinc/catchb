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
	FTM_SWS_CMD_TYPE_LOGIN= 0,
	FTM_SWS_CMD_TYPE_PASSWD,
	FTM_SWS_CMD_TYPE_PROMPT,
	FTM_SWS_CMD_TYPE_CONFIG,
	FTM_SWS_CMD_TYPE_CONFIG_IF_RANGE,
	FTM_SWS_CMD_TYPE_CONFIG_IP_ACL,
	FTM_SWS_CMD_TYPE_FINISHED
}	FTM_SWS_CMD_TYPE, _PTR_ FTM_SWS_CMD_TYPE_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pPrompt;
	FTM_CHAR_PTR		pInput;
	FTM_UINT32			ulSubIndex;
	FTM_UINT32			pNext[8];
	FTM_UINT32			ulNextCount;
}	FTM_SWS_CMD, _PTR_ FTM_SWS_CMD_PTR;

typedef	struct
{
	FTM_SWS_CMD	pCommands[32];
}	FTM_SWITCH_SCRIPT, _PTR_ FTM_SWITCH_SCRIPT_PTR;

FTM_SWITCH_SCRIPT	xDenyScript =
{
	.pCommands = 
	{
		[0]	= {	.pPrompt= "Username:",			 		.pInput = NULL, 	.ulSubIndex = 0,	.pNext = {1, 0}},
		[1]	= {	.pPrompt= "Password:",					.pInput = NULL,		.ulSubIndex = 1,	.pNext = {2, 0} },
		[2]	= {	.pPrompt= "Switch#", 					.pInput = "configure", 					.pNext = {3, 0}},
		[3]	= {	.pPrompt= "Switch(config)#", 			.pInput = "interface range GigabitEthernet 1-28", 	.pNext = {4, }},
		[4]	= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "no ip acl",					.pNext = {5, 0}},
		[5]	= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "exit",						.pNext = {6, 0}},
		[6]	= {	.pPrompt= "Switch(config)#", 			.pInput = "ip acl catchb_filter", 		.pNext = {7, 0}},
		[7]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = NULL,		.ulSubIndex	= 2,	.pNext = {8, 0}},
		[8]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = NULL, 	.ulSubIndex = 3,	.pNext = {9, 0}},
		[9]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = NULL, 	.ulSubIndex = 4,	.pNext = {10, 0}},
		[10]= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = NULL, 	.ulSubIndex = 5,	.pNext = {11, 0}},
		[11]= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "exit", 						.pNext = {12, 0}},
		[12]= {	.pPrompt= "Switch(config)#", 			.pInput = "exit", 						.pNext = {13, 0}},
		[13]= {	.pPrompt= "Switch#", 					.pInput = "exit",						.pNext = {14, 0}},
		[14]= {	.pPrompt= NULL, 						.pInput = NULL},
	}
};

FTM_SWITCH_SCRIPT	xAllowScript =
{
	.pCommands = 
	{
		[0]	= {	.pPrompt= "Username:",			 		.pInput = NULL, 	.ulSubIndex = 0,	.pNext = {1, 0}},
		[1]	= {	.pPrompt= "Password:",					.pInput = NULL,		.ulSubIndex = 1,	.pNext = {2, 0} },
		[2]	= {	.pPrompt= "Switch#", 					.pInput = "configure", 					.pNext = {3, 0}},
		[3]	= {	.pPrompt= "Switch(config)#", 			.pInput = "interface range GigabitEthernet 1-28", 	.pNext = {4, }},
		[4]	= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "no ip acl",					.pNext = {5, 0}},
		[5]	= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "exit",						.pNext = {6, 0}},
		[6]	= {	.pPrompt= "Switch(config)#", 			.pInput = "ip acl catchb_filter", 		.pNext = {7, 0}},
		[7]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = NULL,		.ulSubIndex	= 6,	.pNext = {8, 0}},
		[8]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = NULL, 	.ulSubIndex = 7,	.pNext = {9, 0}},
		[9]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = NULL, 	.ulSubIndex = 8,	.pNext = {10, 0}},
		[10]= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "exit", 						.pNext = {11, 0}},
		[11]= {	.pPrompt= "Switch(config)#", 			.pInput = "exit", 						.pNext = {12, 0}},
		[12]= {	.pPrompt= "Switch#", 					.pInput = "exit",						.pNext = {13, 0}},
		[13]= {	.pPrompt= NULL, 						.pInput = NULL},
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
	FTM_CHAR	pLocalIP[32];
	FTM_TELNET_CLIENT_PTR	pClient;
	FTM_UINT32	ulIndex;
	FTM_SWITCH_SCRIPT_PTR	pScript = NULL;

	FTM_getLocalIP(pLocalIP, 32);
	
	ulIndex = inet_addr(pTargetIP) & 0xFFFFFF;

	if (xPolicy == FTM_SWITCH_AC_POLICY_DENY)
	{
		pScript = &xDenyScript;
	}
	else
	{
		pScript = &xAllowScript;
	}

	xRet = FTM_TELNET_CLIENT_create(&pClient);
	if (xRet != FTM_RET_OK)
	{
		printf("Failed to create client!\n");	
		goto finished;
	}

	xRet = FTM_TELNET_CLIENT_open(pClient, pSwitch->xConfig.pIP, 23);
	if (xRet == FTM_RET_OK)
	{
		FTM_UINT32	ulCommandLine = 0;
		printf("telnet open success!\n");	

		while(pScript->pCommands[ulCommandLine].pPrompt != NULL)
		{
			FTM_CHAR	pCommandBuffer[512];
			FTM_CHAR	pBuffer[512];
			FTM_UINT32	ulCommandBufferLen;
			FTM_UINT32	ulReadLen;
			sleep(1000);

			memset(pBuffer, 0, sizeof(pBuffer));

			xRet = FTM_TELNET_CLIENT_readline(pClient, pBuffer, sizeof(pBuffer), &ulReadLen);
			if ((ulReadLen != 0) && isprint(pBuffer[0]))
			{
				INFO("READ : %s", pBuffer);
				if (strncasecmp(pBuffer, pScript->pCommands[ulCommandLine].pPrompt, strlen(pScript->pCommands[ulCommandLine].pPrompt)) == 0)
				{
					if (pScript->pCommands[ulCommandLine].pInput != 0)
					{
						FTM_TELNET_CLIENT_writel(pClient, pScript->pCommands[ulCommandLine].pInput, strlen(pScript->pCommands[ulCommandLine].pInput));
						ulCommandLine++;
					}
					else
					{
						switch(pScript->pCommands[ulCommandLine].ulSubIndex)
						{
						case	0:
							ulCommandBufferLen = sprintf(pCommandBuffer, "%s", pSwitch->xConfig.pUserID);
							break;

						case	1:
							ulCommandBufferLen = sprintf(pCommandBuffer, "%s", pSwitch->xConfig.pPasswd);
							break;

						case	2:
							ulCommandBufferLen = sprintf(pCommandBuffer,"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255", ulIndex*3, pLocalIP, pTargetIP);
							break;

						case	3:
							ulCommandBufferLen = sprintf(pCommandBuffer,"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255", ulIndex*3+1,pTargetIP, pLocalIP);
							break;

						case	4:
							ulCommandBufferLen = sprintf(pCommandBuffer,"sequence %d deny   ip %s/255.255.255.255 any",ulIndex*3+2, pTargetIP);
							break;

						case	5:
							ulCommandBufferLen = sprintf(pCommandBuffer,"sequence %d permit ip any any", 2147483640);
							break;

						case	6:
							ulCommandBufferLen = sprintf(pCommandBuffer,"no sequence %d", ulIndex*3);
							break;

						case	7:
							ulCommandBufferLen = sprintf(pCommandBuffer,"no sequence %d", ulIndex*3+1);
							break;

						case	8:
							ulCommandBufferLen = sprintf(pCommandBuffer,"no sequence %d", ulIndex*3+2);
							break;
						}

						FTM_TELNET_CLIENT_writel(pClient, pCommandBuffer, ulCommandBufferLen);
						ulCommandLine++;
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


