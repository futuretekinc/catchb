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
#include "ftm_telnet.h"
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
	FTM_CHAR	pLocalIP[FTM_IP_LEN+1];
	FTM_CHAR	pTelnetCommand[64];
	FTM_UINT32	ulIndex;
    FILE 		* pFile;
    FTM_CHAR	pCommandBuffers[NST_MAX_COMD][FTM_COMMAND_LEN];
	FTM_UINT32	ulCommandLines = 0;
	FTM_INT		hFile;
	FTM_INT		i;
	FTM_SSH_PTR	pSSH;

	FTM_getLocalIP(pLocalIP, sizeof(pLocalIP));
	ulIndex = ntohl(inet_addr(pTargetIP)) & 0xFFFFFF;
	
	xRet = FTM_SSH_create(&pSSH);
	xRet = FTM_SSH_connect(pSSH, pSwitch->xConfig.pIP, pSwitch->xConfig.pUserID, pSwitch->xConfig.pPasswd);

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


