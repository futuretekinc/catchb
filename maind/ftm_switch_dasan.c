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

FTM_RET	FTM_SWITCH_DASAN_setAC
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
	FTM_UINT32	ulIndex;
	FTM_UINT32	ulCommandLines = 0;
	FTM_SSH_PTR	pSSH = NULL;
	FTM_SSH_CHANNEL_PTR	pChannel = NULL;
	FTM_TIMER	xTimer;

	FTM_getLocalIP(pLocalIP, sizeof(pLocalIP));
	ulIndex = ntohl(inet_addr(pTargetIP)) & 0xFFFFFF;
	
	switch(xPolicy)
	{
	case	FTM_SWITCH_AC_POLICY_DENY:
		{
			sprintf(pCommandBuffers[ulCommandLines++],"enable\n");
			sprintf(pCommandBuffers[ulCommandLines++],"configure terminal\n");

			sprintf(pCommandBuffers[ulCommandLines++],"flow catchb_main_%d create\n", ulIndex);
			sprintf(pCommandBuffers[ulCommandLines++],"ip %s/32 %s/32\n", pTargetIP, pLocalIP);
			sprintf(pCommandBuffers[ulCommandLines++],"apply\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");

			sprintf(pCommandBuffers[ulCommandLines++],"flow catchb_%d create\n", ulIndex);
			sprintf(pCommandBuffers[ulCommandLines++],"ip %s/32 any\n", pTargetIP);
			sprintf(pCommandBuffers[ulCommandLines++],"apply\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");

			sprintf(pCommandBuffers[ulCommandLines++],"policy catchb_main_%d create\n", ulIndex);
			sprintf(pCommandBuffers[ulCommandLines++],"include-flow catchb_main_%d\n", ulIndex);
			sprintf(pCommandBuffers[ulCommandLines++],"priority highest\n");
			sprintf(pCommandBuffers[ulCommandLines++],"interface-binding vlan any\n");
			sprintf(pCommandBuffers[ulCommandLines++],"action match permit\n");
			sprintf(pCommandBuffers[ulCommandLines++],"apply\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");

			sprintf(pCommandBuffers[ulCommandLines++],"policy catchb_%d create\n", ulIndex);
			sprintf(pCommandBuffers[ulCommandLines++],"include-flow catchb_%d\n", ulIndex);
			sprintf(pCommandBuffers[ulCommandLines++],"priority medium\n");
			sprintf(pCommandBuffers[ulCommandLines++],"interface-binding vlan any\n");
			sprintf(pCommandBuffers[ulCommandLines++],"action match deny\n");
			sprintf(pCommandBuffers[ulCommandLines++],"apply\n");
			sprintf(pCommandBuffers[ulCommandLines++],"end\n");
			sprintf(pCommandBuffers[ulCommandLines++],"exit\n");

		}
		break;

	case	FTM_SWITCH_AC_POLICY_ALLOW:	
		{
			sprintf(pCommandBuffers[ulCommandLines++],"enable\n");
			sprintf(pCommandBuffers[ulCommandLines++],"configure terminal\n");

			sprintf(pCommandBuffers[ulCommandLines++],"no policy catchb_%d\n", ulIndex);
			sprintf(pCommandBuffers[ulCommandLines++],"no flow catchb_%d\n", ulIndex);

			sprintf(pCommandBuffers[ulCommandLines++],"no policy catchb_main_%d\n", ulIndex);
			sprintf(pCommandBuffers[ulCommandLines++],"no flow catchb_main_%d\n", ulIndex);

			sprintf(pCommandBuffers[ulCommandLines++],"end\n");
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

	FTM_TIMER_initS(&xTimer, 1);
	for(i = 0 ; i < ulCommandLines ; i++)
	{
  		if (FTM_SSH_CHANNEL_isOpen(pChannel) && !FTM_SSH_CHANNEL_isEOF(pChannel))
		{
			FTM_CHAR	pBuffer[1024];
			FTM_UINT32	ulReadLen;

			FTM_SSH_CHANNEL_write(pChannel, (FTM_UINT8_PTR)pCommandBuffers[i], strlen(pCommandBuffers[i]));

			while(!FTM_TIMER_isExpired(&xTimer))
			{
				memset(pBuffer, 0, sizeof(pBuffer));
				xRet = FTM_SSH_CHANNEL_read(pChannel, (FTM_UINT8_PTR)pBuffer, sizeof(pBuffer), &ulReadLen);
				if (xRet != FTM_RET_OK)
				{
					ERROR(xRet, "Failed to read channel!");
					goto finished;
				}

				if (ulReadLen == 0)
				{
					usleep(10 * 1000);
				}
				else
				{
					INFO("SSH : %s", pBuffer);	
				}
			}
			FTM_TIMER_initS(&xTimer, 1);
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


