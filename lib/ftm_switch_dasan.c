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

#define	DASAN_MAX_COMD	32

static
FTM_SWITCH_SCRIPT	xDenyScript =
{
	.pCommands = 
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
		[9]= {	.pPrompt= "SWITCH(config-flow",.pInput = "exit"},
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
		[24]= {	.pPrompt= "SWITCH#",			.pInput = "exit"},
		[25]= {	.pPrompt= NULL,				.pInput = ""}
	}
};

static
FTM_SWITCH_SCRIPT	xAllowScript =
{
	.pCommands = 
	{
		[0]	= {	.pPrompt= "SWITCH>",		.pInput = "enable"},
		[1]	= {	.pPrompt= "SWITCH#",		.pInput = "config terminal"},
		[2]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 9},
		[3]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 10},
		[4]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 11},
		[5]	= {	.pPrompt= "SWITCH(config)#",.ulSubIndex = 12},
		[6]= {	.pPrompt= "SWITCH(config)#",.pInput = "end"},
		[7]= {	.pPrompt= "SWITCH#",		.pInput = "exit"},
		[8]= {	.pPrompt= NULL,				.pInput = ""}
	}
};

FTM_RET	FTM_SWITCH_DASAN_setAC
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
	FTM_SWITCH_SCRIPT_PTR pScript;

	if (xPolicy == FTM_SWITCH_AC_POLICY_DENY)
	{
		pScript = &xDenyScript;
	}
	else
	{
		pScript = &xAllowScript;
	}

	FTM_getLocalIP(pLocalIP, sizeof(pLocalIP));
	ulIndex = ntohl(inet_addr(pTargetIP)) & 0xFFFFFF;

	FTM_SWS_CMD_PTR	pCommand = pScript->pCommands;
	while(pCommand->pPrompt != NULL)
	{
		switch(pCommand->ulSubIndex)
		{
		case	1: 	sprintf(pCommand->pInput,	"flow catchb_main_%d create\n", ulIndex); break;
		case	2: 	sprintf(pCommand->pInput,	"ip %s/32 %s/32\n", pTargetIP, pLocalIP); break;
		case	3: 	sprintf(pCommand->pInput,	"flow catchb_%d create\n", ulIndex); break;
		case	4: 	sprintf(pCommand->pInput,	"ip %s/32 any\n", pTargetIP); break;
		case	5: 	sprintf(pCommand->pInput,	"policy catchb_main_%d create\n", ulIndex); break;
		case	6: 	sprintf(pCommand->pInput,	"include-flow catchb_main_%d\n", ulIndex); break;
		case	7: 	sprintf(pCommand->pInput,	"policy catchb_%d create\n", ulIndex); break;
		case	8: 	sprintf(pCommand->pInput,	"include-flow catchb_%d\n", ulIndex); break;
		case	9:	sprintf(pCommand->pInput,	"no policy catchb_%d\n", ulIndex); break;
		case	10:	sprintf(pCommand->pInput,	"no flow catchb_%d\n", ulIndex); break;
		case	11:	sprintf(pCommand->pInput,	"no policy catchb_main_%d\n", ulIndex); break;
		case	12:	sprintf(pCommand->pInput, 	"no flow catchb_main_%d\n", ulIndex); break;
		default: break;
		}

		pCommand++;
	}

#if 0
	if (pSwitch->xConfig.bSecure)
	{
		return	FTM_SWITCH_SSH_setAC(pSwitch, pTargetIP, pScript);
	}
	else
	{
		return	FTM_SWITCH_TELNET_setAC(pSwitch, pTargetIP, pScript);
	}
#else
	return	FTM_SWITCH_SSH_setAC(pSwitch, pTargetIP, pScript);
#endif
}
