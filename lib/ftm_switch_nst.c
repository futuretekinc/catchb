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
#include "ftm_timer.h"
#include "ftm_switch.h"
#include "ftm_ssh.h"
#include "ftm_telnet.h"

#undef	__MODULE__ 
#define	__MODULE__ "switch"

FTM_SWITCH_SCRIPT	xScript =
{
	.xDeny	= 
	{
		.ulCount = 17,
		.pLines = 
		{
			[0]	= {	.pPrompt= "Username:",			 		.pInput = "", 			.ulSubIndex = 1},
			[1]	= {	.pPrompt= "Password:",					.pInput = "",			.ulSubIndex = 2},
			[2]	= {	.pPrompt= "Switch#", 					.pInput = "configure", 	.ulSubIndex = 0},
			[3]	= {	.pPrompt= "Switch(config)#", 			.pInput = "interface range GigabitEthernet 1-28",	.ulSubIndex = 0},
			[4]	= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "no ip acl",	.ulSubIndex = 0},
			[5]	= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "exit",		.ulSubIndex = 0},
			[6]	= {	.pPrompt= "Switch(config)#", 			.pInput = "ip acl catchb_filter"},
			[7]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "",			.ulSubIndex	= 3},
			[8]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "", 			.ulSubIndex = 4},
			[9]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "", 			.ulSubIndex = 5},
			[10]= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "", 			.ulSubIndex = 6},
			[11]= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "exit", 		.ulSubIndex = 0},
			[12]= {	.pPrompt= "Switch(config)#", 			.pInput = "interface range GigabitEthernet 1-28",	.ulSubIndex = 0},
			[13]= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "ip acl catchb_filter",	.ulSubIndex = 0},
			[14]= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "exit",		.ulSubIndex = 0},
			[15]= {	.pPrompt= "Switch(config)#", 			.pInput = "exit", 		.ulSubIndex = 0},
			[16]= {	.pPrompt= "Switch#", 					.pInput = "exit",		.ulSubIndex = 0},
		}
	},

	.xAllow =
	{
		.ulCount = 16,
		.pLines = 
		{
			[0]	= {	.pPrompt= "Username:",			 		.pInput = "", 			.ulSubIndex = 1},
			[1]	= {	.pPrompt= "Password:",					.pInput = "",			.ulSubIndex = 2},
			[2]	= {	.pPrompt= "Switch#", 					.pInput = "configure", 	.ulSubIndex = 0},
			[3]	= {	.pPrompt= "Switch(config)#", 			.pInput = "interface range GigabitEthernet 1-28",	.ulSubIndex = 0},
			[4]	= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "no ip acl",	.ulSubIndex = 0},
			[5]	= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "exit",		.ulSubIndex = 0},
			[6]	= {	.pPrompt= "Switch(config)#", 			.pInput = "ip acl catchb_filter", .ulSubIndex = 0},
			[7]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "",			.ulSubIndex	= 7},
			[8]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "", 			.ulSubIndex = 8},
			[9]	= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "", 			.ulSubIndex = 9},
			[10]= {	.pPrompt= "Switch(config-ip-acl)#",		.pInput = "exit", 		.ulSubIndex = 0},
			[11]= {	.pPrompt= "Switch(config)#", 			.pInput = "interface range GigabitEthernet 1-28",	.ulSubIndex = 0},
			[12]= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "ip acl catchb_filter",	.ulSubIndex = 0},
			[13]= {	.pPrompt= "Switch(config-if-range)#", 	.pInput = "exit",		.ulSubIndex = 0},
			[14]= {	.pPrompt= "Switch(config)#", 			.pInput = "exit", 		.ulSubIndex = 0},
			[15]= {	.pPrompt= "Switch#", 					.pInput = "exit",		.ulSubIndex = 0}
		}
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
	FTM_UINT32	ulIndex;
	FTM_CHAR	pLocalIP[32];
	FTM_UINT32	ulLineCount = 0;
	FTM_SWS_CMD_PTR	pLines;

	FTM_getLocalIP(pLocalIP, 32);
	ulIndex = inet_addr(pTargetIP) & 0xFFFFFF;

	if (xPolicy == FTM_SWITCH_AC_POLICY_DENY)
	{
		ulLineCount = xScript.xDeny.ulCount;
		pLines 		= xScript.xDeny.pLines;
	}
	else
	{
		ulLineCount = xScript.xAllow.ulCount;
		pLines 		= xScript.xAllow.pLines;
	}

	for(FTM_UINT32 ulLine = 0; ulLine < ulLineCount ; ulLine++)
	{
		switch(pLines[ulLine].ulSubIndex)
		{
		case	1: sprintf(pLines[ulLine].pInput, "%s", pSwitch->xConfig.pUserID); break;
		case	2: sprintf(pLines[ulLine].pInput, "%s", pSwitch->xConfig.pPasswd); break;
		case	3: sprintf(pLines[ulLine].pInput, "sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255", ulIndex*3, pLocalIP, pTargetIP); break;
		case	4: sprintf(pLines[ulLine].pInput, "sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255", ulIndex*3+1,pTargetIP, pLocalIP); break;
		case	5: sprintf(pLines[ulLine].pInput, "sequence %d deny   ip %s/255.255.255.255 any",ulIndex*3+2, pTargetIP); break;
		case	6: sprintf(pLines[ulLine].pInput, "sequence %d permit ip any any", 2147483640); break;
		case	7: sprintf(pLines[ulLine].pInput, "no sequence %d", ulIndex*3); break;
		case	8: sprintf(pLines[ulLine].pInput, "no sequence %d", ulIndex*3+1); break;
		case	9: sprintf(pLines[ulLine].pInput, "no sequence %d", ulIndex*3+2); break;
		default: break;
		}
	}

#if 1
	if (pSwitch->xConfig.bSecure)
	{
		return	FTM_SWITCH_SSH_setAC(pSwitch, pTargetIP, pLines, ulLineCount);
	}
	else
	{
		return	FTM_SWITCH_TELNET_setAC(pSwitch, pTargetIP, pLines, ulLineCount);
	}
#else
	return	FTM_SWITCH_TELNET_setAC(pSwitch, pTargetIP, pScript);
#endif
}

