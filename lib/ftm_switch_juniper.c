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

#undef	__MODULE__ 
#define	__MODULE__ "switch"

#define	JUNIPER_MAX_COMD	32

static
FTM_SWITCH_SCRIPT	xScript =
{
	.xDeny =
	{
		.ulCount = 16,
		.pLines = 
		{
			[0]	= {	.pPrompt= "login:",			.pInput = "", 			.ulSubIndex = 1,	.pNext  = {1, 0}},
			[1]	= {	.pPrompt= "Password:",		.pInput = "",			.ulSubIndex = 2,	.pNext 	= {2, 0} },
			[2]	= {	.pPrompt= "Jun>",			.pInput = "configure",	.ulSubIndex = 0,	.pNext 	= {3, 0} },
			[3]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 3, 	.pNext 	= {4, 0} },
			[4]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 4,	.pNext 	= {5, 0} },
			[5]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 5,	.pNext 	= {6, 0} },
			[6]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 6,	.pNext 	= {7, 0} },
			[7]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 7,	.pNext 	= {9, 0} },
			[8]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 8,	.pNext 	= {9, 0} },
			[9]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 9,	.pNext 	= {10, 0} },
			[10]= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 10,	.pNext 	= {11, 0} },
			[11]= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 11,	.pNext 	= {12, 0} },
			[12]= {	.pPrompt= "Jun#",			.pInput = "set vlans default filter input catchb_filter",.ulSubIndex = 0,	.pNext 	= {13, 0} },
			[13]= {	.pPrompt= "Jun#",			.pInput = "commit",		.ulSubIndex = 0,	.pNext 	= {14, 0} },
			[14]= {	.pPrompt= "Jun#",			.pInput = "exit",		.ulSubIndex = 0,	.pNext 	= {15, 0} },
			[15]= {	.pPrompt= "Jun>",			.pInput = "exit",		.ulSubIndex = 0,	.pNext 	= {16, 0} },
		}
	},

	.xAllow =
	{
		.ulCount = 9,
		.pLines = 
		{
			[0]	= {	.pPrompt= "login:",			.pInput = "", 			.ulSubIndex = 1,	.pNext  = {1, 0}},
			[1]	= {	.pPrompt= "Password:",		.pInput = "",			.ulSubIndex = 2,	.pNext 	= {2, 0} },
			[2]	= {	.pPrompt= "Jun>",			.pInput = "configure",	.ulSubIndex = 0,	.pNext 	= {3, 0} },
			[3]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 0, 	.pNext 	= {4, 0} },
			[4]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 12,	.pNext 	= {5, 0} },
			[5]	= {	.pPrompt= "Jun#",			.pInput = "",			.ulSubIndex = 13,	.pNext 	= {6, 0} },
			[6]= {	.pPrompt= "Jun#",			.pInput = "commit",		.ulSubIndex = 0,	.pNext 	= {7, 0} },
			[7]= {	.pPrompt= "Jun#",			.pInput = "exit",		.ulSubIndex = 0,	.pNext 	= {8, 0} },
			[8]= {	.pPrompt= "Jun>",			.pInput = "exit",		.ulSubIndex = 0,	.pNext 	= {9, 0} },
		}
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

	FTM_CHAR		pLocalIP[FTM_IP_LEN+1];
	FTM_UINT32		ulLineCount = 0;
	FTM_SWS_CMD_PTR	pLines;

	FTM_getLocalIP(pLocalIP, sizeof(pLocalIP));

	if (xPolicy == FTM_SWITCH_AC_POLICY_DENY)
	{
		ulLineCount = xScript.xDeny.ulCount;
		pLines		= xScript.xDeny.pLines;
	}
	else
	{
		ulLineCount = xScript.xAllow.ulCount;
		pLines		= xScript.xAllow.pLines;
	}

	FTM_UINT32	ulLine = 0;
	while(ulLine < ulLineCount)
	{
		switch(pLines[ulLine].ulSubIndex)
		{
		case	1:	sprintf(pLines[ulLine].pInput, 	"%s", pSwitch->xConfig.pUserID);	break;
		case	2:	sprintf(pLines[ulLine].pInput, 	"%s", pSwitch->xConfig.pPasswd);	break;
		case	3:	sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 10 from source-address %s", pLocalIP);	break;
		case	4:	sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 10 from destination-address %s", pTargetIP);	break;
		case	5:	sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 10 then accept");	break;
		case	6:	sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 20 from source-address %s", pTargetIP);	break;
		case	7:	sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 20 from destination-address %s", pLocalIP);	break;
		case	8:	sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 20 then accept");	break;
		case	9:	sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 30 from source-address %s", pTargetIP);	break;
		case	10: sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 30 then discard");	break;
		case	11: sprintf(pLines[ulLine].pInput,	"set firewall family ethernet-switching filter catchb_filter term 40 then accept");	break;
		case	12: sprintf(pLines[ulLine].pInput,	"delete firewall family ethernet-switching filter catchb_filter term 10 from destination-address %s", pTargetIP);	break;
		case	13: sprintf(pLines[ulLine].pInput,	"delete firewall family ethernet-switching filter catchb_filter term 20 from source-address %s", pTargetIP);	break;
		case	14: sprintf(pLines[ulLine].pInput,	"delete firewall family ethernet-switching filter catchb_filter term 30 from source-address %s", pTargetIP);	break;
		default: break;
		}

		ulLine++;
	}

	if (pSwitch->xConfig.bSecure)
	{
		return	FTM_SWITCH_SSH_setAC(pSwitch, pTargetIP, pLines, ulLineCount);
	}
	else
	{
		return	FTM_SWITCH_TELNET_setAC(pSwitch, pTargetIP, pLines, ulLineCount);
	}
}
