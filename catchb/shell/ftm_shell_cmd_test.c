#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"


FTM_RET	FTM_SHELL_CMD_test
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

	switch(nArgc)
	{
	case	3:
		{
			if (strcasecmp(pArgv[1], "crypt") == 0)
			{
				FTM_CHAR	pCipherText[128];
				FTM_CHAR	pPlainText[128];

				memset(pCipherText, 0, sizeof(pCipherText));
				memset(pPlainText, 	0, sizeof(pPlainText));

				FTM_encryptUserID(pArgv[2], strlen(pArgv[2]), pCipherText, sizeof(pCipherText));
				FTM_decryptUserID(pCipherText, strlen(pCipherText), pPlainText, sizeof(pPlainText));

				printf("%16s : %s\n", "Original", pArgv[2]);
				printf("%16s : %s\n", "Cipher Text", pCipherText);
				printf("%16s : %s\n", "Plain Text", pPlainText);
			}
		}
		break;

	}

	return	xRet;
}


