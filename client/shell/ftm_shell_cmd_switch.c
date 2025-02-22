#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_client.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"


FTM_RET	FTM_SHELL_CMD_showSwitchACList
(	
	FTM_SWITCH_PTR	pSwitch
);

FTM_RET	FTM_SHELL_CMD_switch
(
	FTM_SHELL_PTR pShell, 
	FTM_INT nArgc, 
	FTM_CHAR_PTR pArgv[], 
	FTM_VOID_PTR pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);
	ASSERT(pData != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	FTM_CLIENT_PTR	pClient = (FTM_CLIENT_PTR)pData;

	if (nArgc == 1)
	{
		FTM_ID_PTR	pIDList;
		FTM_UINT32	i, ulCount = 0;

		pIDList = FTM_MEM_malloc(sizeof(FTM_ID) * 10);

		xRet = FTM_CLIENT_getSwitchIDList(pClient, 10, pIDList, &ulCount);
		if (xRet == FTM_RET_OK)
		{
			for(i = 0 ; i < ulCount ; i++)
			{
				printf("%4d : %s\n", i+1, pIDList[i]);
			}
		}

		FTM_MEM_free(pIDList);
	}
	else if (strcasecmp(pArgv[1], "add") == 0)
	{
		if ((nArgc < 4) || (nArgc % 2 == 1))
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;
		}

		FTM_SWITCH_CONFIG	xConfig;

		memset(&xConfig, 0, sizeof(xConfig));

		strncpy(xConfig.pID, pArgv[2], sizeof(xConfig.pID) - 1);

		xConfig.xModel = FTM_getSwitchModelID(pArgv[3]);
		if (xConfig.xModel == FTM_SWITCH_MODEL_UNKNOWN)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			printf("The switch model %s is not supported!\n", pArgv[3]);
			goto finished;
		}

		FTM_UINT32	i;
		for(i = 4 ; i < nArgc ; i+=2)
		{
			if (strcasecmp(pArgv[i], "--ip") == 0)
			{
				strncpy(xConfig.pIP, pArgv[i+1], sizeof(xConfig.pIP) - 1);
			}
			else if (strcasecmp(pArgv[i], "--userid") == 0)
			{
				strncpy(xConfig.pUserID, pArgv[i+1], sizeof(xConfig.pUserID) - 1);
			}
			else if (strcasecmp(pArgv[i], "--passwd") == 0)
			{
				strncpy(xConfig.pPasswd, pArgv[i+1], sizeof(xConfig.pPasswd) - 1);
			}
			else if (strcasecmp(pArgv[i], "--comment") == 0)
			{
				strncpy(xConfig.pComment, pArgv[i+1], sizeof(xConfig.pComment) - 1);
			}
			else
			{
				xRet = FTM_RET_INVALID_ARGUMENTS;
				goto finished;
			}
		}

		xRet = FTM_CLIENT_addSwitch(pClient, &xConfig);
		if (xRet != FTM_RET_OK)
		{
			printf("Error : Failed to creat switch!\n");	
		}

	}
	else if( strcasecmp(pArgv[1], "del") == 0)
	{
		if (nArgc == 3)
		{
			xRet = FTM_CLIENT_delSwitch(pClient, pArgv[2]);
			if (xRet != FTM_RET_OK)
			{
				printf("Failed to destroy switch[%s]!\n", pArgv[2]);	
			}
			else
			{
				printf("The switch[%s] has been deleted!\n", pArgv[2]);
			}
		}
		else 
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else if (strcasecmp(pArgv[1], "get") == 0)
	{
		if (nArgc < 3)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;	
		}

		FTM_SWITCH_CONFIG	xConfig;

		memset(&xConfig, 0, sizeof(xConfig));

		xRet = FTM_CLIENT_getSwitchProperties(pClient, pArgv[2], &xConfig);
		if (xRet != FTM_RET_OK)
		{
			printf("Error : Failed to get switch properties!\n");
			goto finished;
		}
		
		printf("%8s : %s\n", "ID", 		xConfig.pID);
		printf("%8s : %s\n", "Model", 	FTM_getSwitchModelName(xConfig.xModel));
		printf("%8s : %s\n", "IP", 		xConfig.pIP);
		printf("%8s : %s\n", "UserID",  xConfig.pUserID);
		printf("%8s : %s\n", "Passwd", 	xConfig.pPasswd);
		printf("%8s : %s\n", "Comment", xConfig.pComment);
		
	}
	else if (strcasecmp(pArgv[1], "set") == 0)
	{
		if ((nArgc < 5) || (nArgc % 2 == 0))
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;	
		}

		FTM_SWITCH_CONFIG	xConfig;
		FTM_UINT32			xFieldFlags = 0;

		memset(&xConfig, 0, sizeof(xConfig));

		FTM_UINT32	i;
		for(i = 3 ; i < nArgc ; i+=2)
		{
			if (strcasecmp(pArgv[i], "--ip") == 0)
			{
				strncpy(xConfig.pIP, pArgv[i+1], sizeof(xConfig.pIP) - 1);
				xFieldFlags |= FTM_SWITCH_FIELD_IP;
			}
			else if (strcasecmp(pArgv[i], "--userid") == 0)
			{
				strncpy(xConfig.pUserID, pArgv[i+1], sizeof(xConfig.pUserID) - 1);
				xFieldFlags |= FTM_SWITCH_FIELD_USER_ID;
			}
			else if (strcasecmp(pArgv[i], "--passwd") == 0)
			{
				strncpy(xConfig.pPasswd, pArgv[i+1], sizeof(xConfig.pPasswd) - 1);
				xFieldFlags |= FTM_SWITCH_FIELD_PASSWD;
			}
			else if (strcasecmp(pArgv[i], "--comment") == 0)
			{
				strncpy(xConfig.pComment, pArgv[i+1], sizeof(xConfig.pComment) - 1);
				xFieldFlags |= FTM_SWITCH_FIELD_COMMENT;
			}
			else
			{
				xRet = FTM_RET_INVALID_ARGUMENTS;
				goto finished;
			}
		}

		xRet = FTM_CLIENT_setSwitchProperties(pClient, pArgv[2], &xConfig, xFieldFlags);
		if (xRet != FTM_RET_OK)
		{
			printf("Error : Failed to set switch properties!\n");	
		}
		
	}
	else
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
	}

finished:
	return	xRet;
}

