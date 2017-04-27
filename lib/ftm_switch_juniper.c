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
#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <libssh/connect_ssh.h>

#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_utils.h"
#include "ftm_switch.h"
#include "ftm_timer.h"
#include "ftm_ssh.h"
#include "ftm_telnet.h"

#define	JUNIPER_MAX_COMD	32

typedef	enum	
{
	FTM_SWS_CMD_TYPE_LOGIN,
	FTM_SWS_CMD_TYPE_PASSWD,
	FTM_SWS_CMD_TYPE_CONFIGURE,
	FTM_SWS_CMD_TYPE_SET_FIREWALL,
	FTM_SWS_CMD_TYPE_DEL_FIREWALL,
	FTM_SWS_CMD_TYPE_SET_VLAN,
	FTM_SWS_CMD_TYPE_COMMIT,
	FTM_SWS_CMD_TYPE_EXIT,
	FTM_SWS_CMD_TYPE_FINISHED
}	FTM_SWS_CMD_TYPE, _PTR_ FTM_SWS_CMD_TYPE_PTR;

typedef	struct
{
	FTM_SWS_CMD_TYPE	xType;
	FTM_CHAR_PTR		pPrompt;
	FTM_CHAR_PTR		pInput;
	FTM_UINT32			ulDelay;
	FTM_UINT32			ulSubIndex;
	FTM_UINT32			pNext[8];
	FTM_UINT32			ulNextCount;
}	FTM_SWS_CMD, _PTR_ FTM_SWS_CMD_PTR;

typedef	struct
{
	FTM_SWS_CMD	pCommands[32];
}	FTM_SWITCH_SCRIPT, _PTR_ FTM_SWITCH_SCRIPT_PTR;

static
FTM_SWITCH_SCRIPT	xDenyScript =
{
	.pCommands = 
	{
		[0]	= {	.xType	= FTM_SWS_CMD_TYPE_LOGIN, 			.pPrompt= "login:",			.ulDelay = 10000, .pInput = NULL, 			.pNext  = {1, 0}},
		[1]	= {	.xType	= FTM_SWS_CMD_TYPE_PASSWD, 			.pPrompt= "Password:",		.ulDelay = 10000, .pInput = NULL,			.pNext 	= {2, 0} },
		[2]	= {	.xType	= FTM_SWS_CMD_TYPE_CONFIGURE, 		.pPrompt= "Jun>",			.ulDelay = 10000, .pInput = "configure",	.pNext 	= {3, 0} },
		[3]	= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL,	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 0, 	.pNext 	= {4, 0} },
		[4]	= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 1,	.pNext 	= {5, 0} },
		[5]	= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 2,	.pNext 	= {6, 0} },
		[6]	= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 3,	.pNext 	= {7, 0} },
		[7]	= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 4,	.pNext 	= {9, 0} },
		[8]	= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 5,	.pNext 	= {9, 0} },
		[9]	= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 6,	.pNext 	= {10, 0} },
		[10]= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 7,	.pNext 	= {11, 0} },
		[11]= {	.xType	= FTM_SWS_CMD_TYPE_SET_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 8,	.pNext 	= {12, 0} },
		[12]= {	.xType	= FTM_SWS_CMD_TYPE_SET_VLAN, 		.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = "set vlans default filter input catchb_filter",	.pNext 	= {13, 0} },
		[13]= {	.xType	= FTM_SWS_CMD_TYPE_COMMIT, 			.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = "commit",		.ulSubIndex = 0,	.pNext 	= {14, 0} },
		[14]= {	.xType	= FTM_SWS_CMD_TYPE_EXIT, 			.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = "exit",			.ulSubIndex = 0,	.pNext 	= {15, 0} },
		[15]= {	.xType	= FTM_SWS_CMD_TYPE_EXIT, 			.pPrompt= "Jun>",			.ulDelay = 10000, .pInput = "exit",			.ulSubIndex = 0,	.pNext 	= {16, 0} },
		[16]= {	.xType	= FTM_SWS_CMD_TYPE_FINISHED, 		.pPrompt= "",				.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 0,	.pNext 	= {0, 0} },
	}
};

static
FTM_SWITCH_SCRIPT	xAllowScript =
{
	.pCommands = 
	{
		[0]	= {	.xType	= FTM_SWS_CMD_TYPE_LOGIN, 			.pPrompt= "login:",			.ulDelay = 10000, .pInput = NULL, 			.pNext  = {1, 0}},
		[1]	= {	.xType	= FTM_SWS_CMD_TYPE_PASSWD, 			.pPrompt= "Password:",		.ulDelay = 10000, .pInput = NULL,			.pNext 	= {2, 0} },
		[2]	= {	.xType	= FTM_SWS_CMD_TYPE_CONFIGURE, 		.pPrompt= "Jun>",			.ulDelay = 10000, .pInput = "configure",	.pNext 	= {3, 0} },
		[3]	= {	.xType	= FTM_SWS_CMD_TYPE_DEL_FIREWALL,	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 0, 	.pNext 	= {4, 0} },
		[4]	= {	.xType	= FTM_SWS_CMD_TYPE_DEL_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 1,	.pNext 	= {5, 0} },
		[5]	= {	.xType	= FTM_SWS_CMD_TYPE_DEL_FIREWALL, 	.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = NULL,			.ulSubIndex = 2,	.pNext 	= {6, 0} },
		[6]= {	.xType	= FTM_SWS_CMD_TYPE_COMMIT, 			.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = "commit",		.pNext 	= {7, 0} },
		[7]= {	.xType	= FTM_SWS_CMD_TYPE_EXIT, 			.pPrompt= "Jun#",			.ulDelay = 10000, .pInput = "exit",			.pNext 	= {8, 0} },
		[8]= {	.xType	= FTM_SWS_CMD_TYPE_EXIT, 			.pPrompt= "Jun>",			.ulDelay = 10000, .pInput = "exit",			.pNext 	= {9, 0} },
		[9]= {	.xType	= FTM_SWS_CMD_TYPE_FINISHED, 		.pPrompt= "",				.ulDelay = 10000, .pInput = NULL,			.pNext 	= {0, 0} },
	}
};


FTM_RET	FTM_SWITCH_JUNIPER_setAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWITCH_AC_POLICY	xPolicy
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_RET	xRet;
	FTM_CHAR	pLocalIP[32];
	FTM_TELNET_CLIENT_PTR	pClient;
	FTM_UINT32	ulCommandLine = 0;
	FTM_UINT32	ulSubIndex = 0;

	FTM_getLocalIP(pLocalIP, 32);
	
	xRet = FTM_TELNET_CLIENT_create(&pClient);
	if (xRet != FTM_RET_OK)
	{
		printf("Failed to create client!\n");	
		goto finished;
	}

	FTM_SWITCH_SCRIPT_PTR	pScript;
	
	if (xPolicy == FTM_SWITCH_AC_POLICY_DENY)
	{
		pScript = &xDenyScript;
	}
	else
	{
		pScript = &xAllowScript;
	}

	xRet = FTM_TELNET_CLIENT_open(pClient, pSwitch->xConfig.pIP, 23);
	if (xRet == FTM_RET_OK)
	{
		FTM_BOOL	bFinished = FTM_FALSE;
		printf("telnet open success!\n");	

		while(!bFinished)
		{
			FTM_CHAR	pCommandBuffer[512];
			FTM_CHAR	pBuffer[512];
			FTM_UINT32	ulCommandBufferLen;
			FTM_UINT32	ulReadLen;
			usleep(pScript->pCommands[ulCommandLine].ulDelay);

			memset(pBuffer, 0, sizeof(pBuffer));

			xRet = FTM_TELNET_CLIENT_readline(pClient, pBuffer, sizeof(pBuffer), &ulReadLen);
			if ((!bFinished) && (xRet == FTM_RET_OK) && (ulReadLen != 0) && isprint(pBuffer[0]))
			{
				INFO("READ : %s", pBuffer);
				if (strncasecmp(pBuffer, pScript->pCommands[ulCommandLine].pPrompt, strlen(pScript->pCommands[ulCommandLine].pPrompt)) == 0)
				{
					switch(pScript->pCommands[ulCommandLine].xType)
					{
					case	FTM_SWS_CMD_TYPE_LOGIN:
						{
							FTM_TELNET_CLIENT_writel(pClient, pSwitch->xConfig.pUserID, strlen(pSwitch->xConfig.pUserID));
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_PASSWD:
						{
							FTM_TELNET_CLIENT_writel(pClient, pSwitch->xConfig.pPasswd, strlen(pSwitch->xConfig.pPasswd));
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_CONFIGURE:
						{
							FTM_TELNET_CLIENT_writel(pClient, pScript->pCommands[ulCommandLine].pInput, strlen(pScript->pCommands[ulCommandLine].pInput));
							ulSubIndex = 0;
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_SET_FIREWALL:
						{
							switch(ulSubIndex)
							{
							case	0:
								ulCommandBufferLen = sprintf(pCommandBuffer, "set firewall family ethernet-switching filter catchb_filter term 10 from source-address %s", pLocalIP);
								break;

							case	1:
								ulCommandBufferLen = sprintf(pCommandBuffer,"set firewall family ethernet-switching filter catchb_filter term 10 from destination-address %s", pTargetIP);
								break;

							case	2:
								ulCommandBufferLen = sprintf(pCommandBuffer,"set firewall family ethernet-switching filter catchb_filter term 10 then accept");
								break;

							case	3:
								ulCommandBufferLen = sprintf(pCommandBuffer,"set firewall family ethernet-switching filter catchb_filter term 20 from source-address %s", pTargetIP);
								break;

							case	4:
								ulCommandBufferLen = sprintf(pCommandBuffer,"set firewall family ethernet-switching filter catchb_filter term 20 from destination-address %s", pLocalIP);
								break;

							case	5:
								ulCommandBufferLen = sprintf(pCommandBuffer,"set firewall family ethernet-switching filter catchb_filter term 20 then accept");
								break;

							case	6:
								ulCommandBufferLen = sprintf(pCommandBuffer,"set firewall family ethernet-switching filter catchb_filter term 30 from source-address %s", pTargetIP);
								break;

							case	7:
								ulCommandBufferLen = sprintf(pCommandBuffer,"set firewall family ethernet-switching filter catchb_filter term 30 then discard");
								break;

							case	8:
								ulCommandBufferLen = sprintf(pCommandBuffer,"set firewall family ethernet-switching filter catchb_filter term 40 then accept");
							}
							ulSubIndex++;

							FTM_TELNET_CLIENT_writel(pClient, pCommandBuffer, ulCommandBufferLen);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_DEL_FIREWALL:
						{
							switch(ulSubIndex)
							{
							case	0:
								ulCommandBufferLen = sprintf(pCommandBuffer,"delete firewall family ethernet-switching filter catchb_filter term 10 from destination-address %s", pTargetIP);
								break;

							case	1:
								ulCommandBufferLen = sprintf(pCommandBuffer,"delete firewall family ethernet-switching filter catchb_filter term 20 from source-address %s", pTargetIP);
								break;

							case	2:
								ulCommandBufferLen = sprintf(pCommandBuffer,"delete firewall family ethernet-switching filter catchb_filter term 30 from source-address %s", pTargetIP);
								break;
							}
							ulSubIndex++;

							FTM_TELNET_CLIENT_writel(pClient, pCommandBuffer, ulCommandBufferLen);
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_SET_VLAN:
						{
							FTM_TELNET_CLIENT_writel(pClient, pScript->pCommands[ulCommandLine].pInput, strlen(pScript->pCommands[ulCommandLine].pInput));
							ulSubIndex = 0;
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_COMMIT:
						{
							FTM_TELNET_CLIENT_writel(pClient, pScript->pCommands[ulCommandLine].pInput, strlen(pScript->pCommands[ulCommandLine].pInput));
							ulSubIndex = 0;
							ulCommandLine++;
						}
						break;

					case	FTM_SWS_CMD_TYPE_EXIT:
						{
							FTM_TELNET_CLIENT_writel(pClient, pScript->pCommands[ulCommandLine].pInput, strlen(pScript->pCommands[ulCommandLine].pInput));
							ulSubIndex = 0;
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


