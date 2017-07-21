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
#include "ftm_mem.h"
#include "ftm_trace.h"
#include "ftm_utils.h"
#include "ftm_switch.h"
#include "ftm_timer.h"
#include "ftm_ssh.h"
#include "ftm_telnet.h"

#undef	__MODULE__ 
#define	__MODULE__ "switch"

#define	DASAN_MAX_COMD	32

static
FTM_SWITCH_SCRIPT	xScript =
{
	.xDeny = 
	{
		.ulCount = 25,
		.pLines = 
		{
			[0]	= {	.pPrompt= "SWITCH>",		.pInput = "enable"},
			[1]	= {	.pPrompt= "SWITCH#",		.pInput = "config terminal"},
			[2]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 1},
			[3]	= {	.pPrompt= "SWITCH(config-flow",.ulSubIndex = 2},
			[4]	= {	.pPrompt= "SWITCH(config-flow",.pInput = "apply"},
			[5]	= {	.pPrompt= "SWITCH(config-flow",.pInput = "exit"},
			[6]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 3},
			[7]	= {	.pPrompt= "SWITCH(config-flow",.ulSubIndex = 4},
			[8]	= {	.pPrompt= "SWITCH(config-flow",.pInput = "apply"},
			[9] = {	.pPrompt= "SWITCH(config-flow",.pInput = "exit"},
			[10]= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 5},
			[11]= {	.pPrompt= "SWITCH(config-policy",.ulSubIndex = 6},
			[12]= {	.pPrompt= "SWITCH(config-policy",.pInput = "priority highest"},
			[13]= {	.pPrompt= "SWITCH(config-policy",.pInput = "interface-binding vlan any"},
			[14]= {	.pPrompt= "SWITCH(config-policy",.pInput = "action match permit"},
			[15]= {	.pPrompt= "SWITCH(config-policy",.pInput = "apply"},
			[16]= {	.pPrompt= "SWITCH(config-policy",.pInput = "exit"},
			[17]= {	.pPrompt= "SWITCH(config)#",	.ulSubIndex = 7},
			[18]= {	.pPrompt= "SWITCH(config-policy",.ulSubIndex = 8},
			[19]= {	.pPrompt= "SWITCH(config-policy",.pInput = "priority medium"},
			[20]= {	.pPrompt= "SWITCH(config-policy",.pInput = "interface-binding vlan any"},
			[21]= {	.pPrompt= "SWITCH(config-policy",.pInput = "action match deny"},
			[22]= {	.pPrompt= "SWITCH(config-policy",.pInput = "apply"},
			[23]= {	.pPrompt= "SWITCH(config-policy",.pInput = "end"},
			[24]= {	.pPrompt= "SWITCH#",			.pInput = "exit"}
		}
	},

	.xAllow =
	{
		.ulCount = 8,
		.pLines = 
		{
			[0]	= {	.pPrompt= "SWITCH>",		.pInput = "enable"},
			[1]	= {	.pPrompt= "SWITCH#",		.pInput = "config terminal"},
			[2]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 9},
			[3]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 10},
			[4]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 11},
			[5]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 12},
			[6] = {	.pPrompt= "SWITCH(config)#",.pInput = "end"},
			[7] = {	.pPrompt= "SWITCH#",		.pInput = "exit"},
			[8] = {	.pPrompt= "",				.pInput = ""}
		}
	}
};

FTM_RET	FTM_SWITCH_GEN_setAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWITCH_AC_POLICY	xPolicy
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_CHAR	pLocalIP[FTM_IP_LEN+1];
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 0;
	FTM_SWS_CMD_PTR	pLines;

	if (xPolicy == FTM_SWITCH_AC_POLICY_DENY)
	{
		ulCount		= xScript.xDeny.ulCount;
		pLines 	= xScript.xDeny.pLines;
	}
	else
	{
		ulCount		= xScript.xAllow.ulCount;
		pLines 	= xScript.xAllow.pLines;
	}

	FTM_getLocalIP(pLocalIP, sizeof(pLocalIP));
	ulIndex = ntohl(inet_addr(pTargetIP)) & 0xFFFFFF;

	for(FTM_UINT32	ulLine = 0; ulLine < ulCount ; ulLine++)
	{
		switch(pLines[ulLine].ulSubIndex)
		{
		case	1: 	sprintf(pLines[ulLine].pInput,	"flow catchb_main_%d create", ulIndex); break;
		case	2: 	sprintf(pLines[ulLine].pInput,	"ip %s/32 %s/32", pTargetIP, pLocalIP); break;
		case	3: 	sprintf(pLines[ulLine].pInput,	"flow catchb_%d create", ulIndex); break;
		case	4: 	sprintf(pLines[ulLine].pInput,	"ip %s/32 any", pTargetIP); break;
		case	5: 	sprintf(pLines[ulLine].pInput,	"policy catchb_main_%d create", ulIndex); break;
		case	6: 	sprintf(pLines[ulLine].pInput,	"include-flow catchb_main_%d", ulIndex); break;
		case	7: 	sprintf(pLines[ulLine].pInput,	"policy catchb_%d create", ulIndex); break;
		case	8: 	sprintf(pLines[ulLine].pInput,	"include-flow catchb_%d", ulIndex); break;
		case	9:	sprintf(pLines[ulLine].pInput,	"no policy catchb_%d", ulIndex); break;
		case	10:	sprintf(pLines[ulLine].pInput,	"no flow catchb_%d", ulIndex); break;
		case	11:	sprintf(pLines[ulLine].pInput,	"no policy catchb_main_%d", ulIndex); break;
		case	12:	sprintf(pLines[ulLine].pInput, 	"no flow catchb_main_%d", ulIndex); break;
		default: break;
		}
	}

#if 1
	if (pSwitch->xConfig.bSecure)
	{
		return	FTM_SWITCH_SSH_setAC(pSwitch, pTargetIP, pLines, ulCount);
	}
	else
	{
		return	FTM_SWITCH_TELNET_setAC(pSwitch, pTargetIP, pLines, ulCount);
	}
#else
	return	FTM_SWITCH_SSH_setAC(pSwitch, pTargetIP, pScript);
#endif
}

