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

#define	NST_MAX_COMD	32

FTM_RET	FTM_SWITCH_JUNIPER_setAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWITCH_AC_POLICY	xPolicy
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_RET		xRet;
	FTM_INT		i;
	FTM_CHAR	pLocalIP[FTM_IP_LEN+1];
    FTM_CHAR	pCommandBuffers[NST_MAX_COMD][FTM_COMMAND_LEN];
	//FTM_UINT32	ulIndex;
	FTM_UINT32	ulCommandLines = 0;
	FTM_SSH_PTR	pSSH = NULL;
	FTM_SSH_CHANNEL_PTR	pChannel = NULL;

	FTM_getLocalIP(pLocalIP, sizeof(pLocalIP));
	//ulIndex = ntohl(inet_addr(pTargetIP)) & 0xFFFFFF;
	
	switch(xPolicy)
	{
	case	FTM_SWITCH_AC_POLICY_DENY:
		{
			sprintf(pCommandBuffers[ulCommandLines++],"configure\n");

			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 10 from source-address %s\n", pLocalIP);
			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 10 from destination-address %s\n", pTargetIP);
			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 10 then accept\n");

			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 20 from source-address %s\n", pTargetIP);
			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 20 from destination-address %s\n", pLocalIP);
			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 20 then accept\n");

			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 30 from source-address %s\n", pTargetIP);
			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 30 then discard\n");

			sprintf(pCommandBuffers[ulCommandLines++],"set firewall family ethernet-switching filter catchb_filter term 40 then accept\n");

			sprintf(pCommandBuffers[ulCommandLines++],"set vlans default filter input catchb_filter\n");
			sprintf(pCommandBuffers[ulCommandLines++],"commit\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");

		}
		break;

	case	FTM_SWITCH_AC_POLICY_ALLOW:	
		{
			sprintf(pCommandBuffers[ulCommandLines++],"configure\n");
			sprintf(pCommandBuffers[ulCommandLines++],"show firewall family ethernet-switching filter catchb_filter\n");

			sprintf(pCommandBuffers[ulCommandLines++],"\n");
			sprintf(pCommandBuffers[ulCommandLines++],"\n");

			sprintf(pCommandBuffers[ulCommandLines++],"delete firewall family ethernet-switching filter catchb_filter term 10 from destination-address %s\n", pTargetIP);

			sprintf(pCommandBuffers[ulCommandLines++],"delete firewall family ethernet-switching filter catchb_filter term 20 from source-address %s\n",pTargetIP);

			sprintf(pCommandBuffers[ulCommandLines++],"delete firewall family ethernet-switching filter catchb_filter term 30 from source-address %s\n",pTargetIP);

			sprintf(pCommandBuffers[ulCommandLines++],"commit\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");

		}
		break;
	}


	xRet = FTM_SSH_create(&pSSH);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create SSH!");	
		goto finished;
	}

	xRet = FTM_SSH_connect(pSSH, pSwitch->xConfig.pIP, pSwitch->xConfig.pUserID, pSwitch->xConfig.pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create channel!");	
		goto finished;
	}

	xRet = FTM_SSH_CHANNEL_create(pSSH, &pChannel);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create channel!");	
		goto finished;
	}

	xRet = FTM_SSH_CHANNEL_open(pChannel);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create channel!");	
		goto finished;
	}

	for(i = 0 ; i < ulCommandLines ; i++)
	{
  		if (FTM_SSH_CHANNEL_isOpen(pChannel) && !FTM_SSH_CHANNEL_isEOF(pChannel))
		{
			FTM_CHAR	pBuffer[1024];
			FTM_UINT32	ulReadLen;
			FTM_CHAR	pErrorBuffer[1024];
			FTM_UINT32	ulErrorReadLen;

			xRet = FTM_SSH_CHANNEL_read(pChannel, 1000, (FTM_UINT8_PTR)pBuffer, sizeof(pBuffer), &ulReadLen, (FTM_UINT8_PTR)pErrorBuffer, sizeof(pErrorBuffer), &ulErrorReadLen);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to read channel!");
				goto finished;
			}

			FTM_SSH_CHANNEL_write(pChannel, (FTM_UINT8_PTR)pCommandBuffers[i], strlen(pCommandBuffers[i]));
		}
	}

	FTM_SSH_CHANNEL_close(pChannel);
	FTM_SSH_disconnect(pSSH);

finished:
	
	if (pChannel != NULL)
	{
		FTM_SSH_CHANNEL_destroy(&pChannel);	
	}

	if (pSSH != NULL)
	{
		FTM_SSH_destroy(&pSSH);	
	}

	return	xRet;
}


