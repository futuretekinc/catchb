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

#define NST_MAX_COMD 20
FTM_RET	FTM_SWITCH_NST_process
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_INT			nIndex,
	FTM_SWITCH_AC_POLICY	xPolicy
)  //0 : switch ip deny, 1 : switch ip allow
{
	ASSERT(pSwitch != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_RET	xRet = FTM_RET_OK;
    FILE * pFile;
    FTM_CHAR	pTelnetCommand[64];
    FTM_CHAR	pLocalIP[FTM_IP_LEN+1];
    FTM_CHAR	pCommandBuffers[NST_MAX_COMD][FTM_COMMAND_LEN];

	INFO("Request NST[%s] to %s IP[%s]!", pSwitch->xConfig.pIP, (xPolicy)?"allow":"block", pTargetIP);

	return	0;

	FTM_getLocalIP(pLocalIP, sizeof(pLocalIP)-1);

    sprintf(pTelnetCommand, "telnet %s", pSwitch->xConfig.pIP);
    pFile = popen(pTelnetCommand, "w");
	if (pFile == NULL)
	{
		xRet = FTM_RET_ERROR;
		goto error;
	}

    switch(xPolicy)
    {
	case	FTM_SWITCH_AC_POLICY_DENY:
		{
			sprintf(pCommandBuffers[ 0],"%s\n",pSwitch->xConfig.pUserID);
			sprintf(pCommandBuffers[ 1],"%s\n",pSwitch->xConfig.pPasswd);
			memcpy(pCommandBuffers[ 2],"configure\n",sizeof(pCommandBuffers[2]));

			memcpy(pCommandBuffers[ 3],"interface range GigabitEthernet 1-28\n",sizeof(pCommandBuffers[3]));
			memcpy(pCommandBuffers[ 4],"no ip acl\n",sizeof(pCommandBuffers[4]));

			memcpy(pCommandBuffers[ 5],"exit\n",sizeof(pCommandBuffers[5]));

			memcpy(pCommandBuffers[ 6],"ip acl catchb_filter\n",sizeof(pCommandBuffers[6]));

			sprintf(pCommandBuffers[ 7],"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255\n", (nIndex+1)*3, pLocalIP, pTargetIP);
			sprintf(pCommandBuffers[ 8],"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255\n", (nIndex+1)*3+1,pTargetIP, pLocalIP);
			sprintf(pCommandBuffers[ 9],"sequence %d deny   ip %s/255.255.255.255 any\n",(nIndex+1)*3+2, pTargetIP);
			sprintf(pCommandBuffers[10],"sequence %d permit ip any any\n", 2147483640);
			memcpy(pCommandBuffers[11],"exit\n",sizeof(pCommandBuffers[11]));
			memcpy(pCommandBuffers[12],"interface range GigabitEthernet 1-28\n",sizeof(pCommandBuffers[12]));
			memcpy(pCommandBuffers[13],"ip acl catchb_filter\n",sizeof(pCommandBuffers[13]));
			memcpy(pCommandBuffers[14],"exit\n",sizeof(pCommandBuffers[14]));
			memcpy(pCommandBuffers[15],"exit\n",sizeof(pCommandBuffers[15]));
			memcpy(pCommandBuffers[16],"exit\n",sizeof(pCommandBuffers[16]));
			memcpy(pCommandBuffers[17],"exit\n",sizeof(pCommandBuffers[17]));
		}
		break;

	case	FTM_SWITCH_AC_POLICY_ALLOW:
		{
			sprintf(pCommandBuffers[0],"%s\n",pSwitch->xConfig.pUserID);
			sprintf(pCommandBuffers[1],"%s\n",pSwitch->xConfig.pPasswd);
			memcpy(pCommandBuffers[2],"configure\n",sizeof(pCommandBuffers[2]));


			memcpy(pCommandBuffers[3],"interface range GigabitEthernet 1-28\n",sizeof(pCommandBuffers[3]));
			memcpy(pCommandBuffers[4],"no ip acl\n",sizeof(pCommandBuffers[4]));

			memcpy(pCommandBuffers[5],"exit\n",sizeof(pCommandBuffers[5]));
			memcpy(pCommandBuffers[6],"ip acl catchb_filter\n",sizeof(pCommandBuffers[6]));

			sprintf(pCommandBuffers[7],"no sequence %d\n",(nIndex+1)*3);
			sprintf(pCommandBuffers[8],"no sequence %d\n",(nIndex+1)*3+1);
			sprintf(pCommandBuffers[9],"no sequence %d\n",(nIndex+1)*3+2);

			memcpy(pCommandBuffers[10],"exit\n",sizeof(pCommandBuffers[10]));

			memcpy(pCommandBuffers[11],"interface range GigabitEthernet 1-28\n",sizeof(pCommandBuffers[11]));
			memcpy(pCommandBuffers[12],"ip acl catchb_filter\n",sizeof(pCommandBuffers[12]));

			memcpy(pCommandBuffers[13],"end\n",sizeof(pCommandBuffers[13]));
			memcpy(pCommandBuffers[14],"exit\n",sizeof(pCommandBuffers[14]));
			memcpy(pCommandBuffers[15],"exit\n",sizeof(pCommandBuffers[15]));
		}
		break;
	}

	{
		FTM_INT		hFile = fileno(pFile);
		FTM_INT		i;

		for(i = 0 ; i < 16 ; i++)
		{
			write(hFile, pCommandBuffers[i], strlen(pCommandBuffers[i]));
			usleep( 1000 * 700 );
		}
	} 

	pclose(pFile);

error:
    return xRet;
}




























