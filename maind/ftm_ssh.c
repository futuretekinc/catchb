/*
 * connect_ssh.c
 * This file contains an example of how to connect to a
 * SSH server using libssh
 */

/*
Copyright 2009 Aris Adamantiadis

This file is part of the SSH Library

You are free to copy this file, modify it in any way, consider it being public
domain. This does not apply to the rest of the library though, but it is
allowed to cut-and-paste working code from this file to any license of
program.
The goal is to show the API in action. It's not a reference on how terminal
clients must be made or how a client should react.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ftm_ssh.h"
#include "ftm_trace.h"

FTM_RET	FTM_SSH_create
(
	FTM_SSH_PTR	_PTR_ ppSSH
)
{
	ASSERT(ppSSH != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_SSH_PTR	pSSH;

	pSSH = ssh_new();
	if (pSSH == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create ssh!");
		return	xRet;
	}

	*ppSSH = pSSH;

	return	xRet;
}

FTM_RET	FTM_SSH_destroy
(
	FTM_SSH_PTR _PTR_ ppSSH
)
{
	ASSERT(ppSSH != NULL);

	ssh_free(*ppSSH);
	*ppSSH = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SSH_connect
(
	FTM_SSH_PTR	pSSH,
	const FTM_CHAR_PTR	pHost, 
	const FTM_CHAR_PTR	pUser,
	const FTM_CHAR_PTR	pPasswd
)
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nRet;

	if(pUser != NULL)
	{
		nRet = ssh_options_set(pSSH, SSH_OPTIONS_USER, pUser);
		if (nRet < 0)
		{
			xRet = FTM_RET_SSH_FAILED_TO_SET_OPTION;
			ERROR(xRet, "Failed to set option!");
			return xRet;
		}
	}

	nRet = ssh_options_set(pSSH, SSH_OPTIONS_HOST, pHost);
	if (nRet < 0)
	{
		xRet = FTM_RET_SSH_FAILED_TO_SET_OPTION;
		ERROR(xRet, "Failed to set option!");
		return xRet;
	}

	nRet = ssh_connect(pSSH);
	if (nRet < 0)
	{
		xRet = FTM_RET_SSH_CONNECTION_FAILED;
		ERROR(xRet, "Failed to connect[%s]",ssh_get_error(pSSH));
		return xRet;
	}

	xRet = FTM_SSH_verifyKnownhost(pSSH);
	if(xRet != FTM_RET_OK)
	{
		FTM_SSH_disconnect(pSSH);

		ERROR(xRet, "Failed to verify known host!");
		return xRet;
	}

	xRet = FTM_SSH_authenticateConsole(pSSH, pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to authenticate console!");
		FTM_SSH_disconnect(pSSH);
	}

	return	xRet;
}

FTM_RET	FTM_SSH_disconnect
(
	FTM_SSH_PTR	pSSH
)
{
	ASSERT(pSSH != NULL);

	ssh_disconnect(pSSH);

	return	FTM_RET_OK;
}

#if 0
FTM_RET	FTM_SSH_authenticateKbdint
(
	FTM_SSH_PTR	pSSH, 
	const FTM_CHAR_PTR	pPasswd	
) 
{
	FTM_RET	xRet;
	FTM_INT	nRet;

    nRet = ssh_userauth_kbdint(pSSH, NULL, NULL);
    while (nRet == SSH_AUTH_INFO) 
	{
        const FTM_CHAR _PTR_	pInstruction;
        const FTM_CHAR _PTR_	pName;
        FTM_CHAR	pBuffer[128];
        FTM_INT		i, n;

        pName = ssh_userauth_kbdint_getname(pSSH);
        if (pName && strlen(pName) > 0) 
		{
            INFO("SSH : Name - %s\n", pName);
        }

        pInstruction = ssh_userauth_kbdint_getinstruction(pSSH);
        if (pInstruction && strlen(pInstruction) > 0) 
		{
            INFO("SSH : Instruction - %s\n", pInstruction);
        }

        n = ssh_userauth_kbdint_getnprompts(pSSH);

        for (i = 0; i < n; i++) 
		{
            const FTM_CHAR _PTR_	pAnswer;
            const FTM_CHAR _PTR_	pPrompt;
            char echo;

            pPrompt = ssh_userauth_kbdint_getprompt(pSSH, i, &echo);
            if (pPrompt == NULL) 
			{
                break;
            }

            if (echo) 
			{
                char *p;

                printf("%s", pPrompt);

                if (fgets(pBuffer, sizeof(pBuffer), stdin) == NULL) 
				{
                    return SSH_AUTH_ERROR;
                }

                pBuffer[sizeof(pBuffer) - 1] = '\0';
                if ((p = strchr(pBuffer, '\n'))) 
				{
                    *p = '\0';
                }

                if (ssh_userauth_kbdint_setanswer(pSSH, i, pBuffer) < 0) {
                    return SSH_AUTH_ERROR;
                }

                memset(pBuffer, 0, strlen(pBuffer));
            } 
			else 
			{
                if (pPasswd && strstr(pPrompt, "Password:")) 
				{
                    pAnswer = pPasswd;
                } 
				else 
				{
                    pBuffer[0] = '\0';

                    if (ssh_getpass(pPrompt, pBuffer, sizeof(pBuffer), 0, 0) < 0) 
					{
                        return SSH_AUTH_ERROR;
                    }
                    pAnswer = pBuffer;
                }

                nRet = ssh_userauth_kbdint_setanswer(pSSH, i, pAnswer);
                memset(pBuffer, 0, sizeof(pBuffer));
                if (nRet < 0) 
				{
					xRet = FTM_RET_SSH_AUTH_ERROR;
					ERROR(xRet, "Failed to auth!");
					return	xRet;
                }
            }
        }
        nRet = ssh_userauth_kbdint(pSSH,NULL,NULL);
    }

	if (nRet < 0)
	{
		xRet = FTM_RET_SSH_AUTH_ERROR;
	}

    return xRet;
}
#endif

FTM_RET	FTM_SSH_authenticateConsole
(
	FTM_SSH_PTR	pSSH, 
	const FTM_CHAR_PTR	pPasswd
)	
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nRet;
	FTM_INT	nMethod;

	// Try to authenticate
	nRet = ssh_userauth_none(pSSH, NULL);
	if (nRet == SSH_AUTH_ERROR) 
	{
		xRet = FTM_RET_SSH_AUTH_ERROR;
		ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH));
		return xRet;
	}

	nMethod = ssh_userauth_list(pSSH, NULL);
	while (nRet != SSH_AUTH_SUCCESS) 
	{
		if (nMethod & SSH_AUTH_METHOD_GSSAPI_MIC)
		{
			nRet = ssh_userauth_gssapi(pSSH);
			if(nRet == SSH_AUTH_ERROR) 
			{
				xRet = FTM_RET_SSH_AUTH_ERROR;
				ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH));
				return xRet;
			} 
			else if (nRet == SSH_AUTH_SUCCESS) 
			{
				break;
			}
		}

		// Try to authenticate with public key first
		if (nMethod & SSH_AUTH_METHOD_PUBLICKEY) 
		{
			nRet = ssh_userauth_publickey_auto(pSSH, NULL, NULL);
			if (nRet == SSH_AUTH_ERROR) 
			{
				xRet = FTM_RET_SSH_AUTH_ERROR;
				ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH));
				return xRet;
			} 
			else if (nRet == SSH_AUTH_SUCCESS) 
			{
				break;
			}
		}

#if 0
		// Try to authenticate with keyboard interactive";
		if (nMethod & SSH_AUTH_METHOD_INTERACTIVE) 
		{
			nRet = FTM_SSH_authenticateKbdint(pSSH, NULL);
			if (nRet == SSH_AUTH_ERROR) 
			{
				xRet = FTM_RET_SSH_AUTH_ERROR;
				ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH));
				return xRet;
			} 
			else if (nRet == SSH_AUTH_SUCCESS) 
			{
				break;
			}
		}
#endif
		// Try to authenticate with password
		if (nMethod & SSH_AUTH_METHOD_PASSWORD) 
		{
			nRet = ssh_userauth_password(pSSH, NULL, pPasswd);
			if (nRet == SSH_AUTH_ERROR) 
			{
				xRet = FTM_RET_SSH_AUTH_ERROR;
				ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH));
				return xRet;
			} 
			else if (nRet == SSH_AUTH_SUCCESS) 
			{
				break;
			}
		}
	}

	return FTM_RET_OK;
}

FTM_RET	FTM_SSH_verifyKnownhost
(
	FTM_SSH_PTR pSSH
)
{
	ASSERT(pSSH != NULL);
	
	FTM_RET			xRet = FTM_RET_OK;
	FTM_INT			nRet;
	FTM_CHAR_PTR	pHexa;
	FTM_INT			nState;
	FTM_CHAR		pBuff[10];
	FTM_UINT8_PTR	pHash = NULL;
	size_t 			nHashLen;
	ssh_key 		pServerPublicKey;


	memset(pBuff, 0x00, sizeof(pBuff));

	nRet = ssh_get_publickey(pSSH, &pServerPublicKey);
	if (nRet < 0) 
	{
		xRet = FTM_RET_SSH_FAILED_TO_GET_PUBLICKEY;
		ERROR(xRet, "Failed to get publickey!");
		return xRet;
	}

	nRet = ssh_get_publickey_hash(pServerPublicKey, SSH_PUBLICKEY_HASH_SHA1, &pHash, &nHashLen);
	ssh_key_free(pServerPublicKey);
	if (nRet < 0) 
	{
		xRet = FTM_RET_SSH_FAILED_TO_GET_PUBLICKEY_HASH;
		ERROR(xRet, "Failed to get publickey hash!");
		return xRet;
	}

	nState = ssh_is_server_known(pSSH);
	if (nState == SSH_SERVER_KNOWN_CHANGED)
	{
		nState = SSH_SERVER_NOT_KNOWN;
	}

	switch(nState)
	{
	case SSH_SERVER_KNOWN_OK:
		break; /* ok */

	case SSH_SERVER_KNOWN_CHANGED:
		xRet = 	FTM_RET_SSH_SERVER_KNOWN_CHANGED;
		ERROR(xRet, "SSH : Host key for server changed");
		ERROR(xRet, "SSH : server's one is now :");
		ERROR(xRet, "SSH : For security reason, connection will be stopped");
		break;

	case SSH_SERVER_FOUND_OTHER:
		xRet = FTM_RET_SSH_SERVER_FOUND_OTHER;
		ERROR(xRet, "SSH : The host key for this server was not found but an other type of key exists.");
		ERROR(xRet, "SSH : An attacker might change the default server key to confuse your client");
		ERROR(xRet, "SSH : into thinking the key does not exist");
		ERROR(xRet, "SSH : We advise you to rerun the client with -d or -r for more safety.");
		break;

	case SSH_SERVER_FILE_NOT_FOUND:
		INFO("SSH : Could not find known host file. If you accept the host key here,");
		INFO("SSH : the file will be automatically created.");
		/* fallback to SSH_SERVER_NOT_KNOWN behavior */
	case SSH_SERVER_NOT_KNOWN:
		pHexa = ssh_get_hexa(pHash, nHashLen);
		INFO("The server is unknown. Do you trust the host key ?");
		INFO("Public key hash: %s", pHexa);
		ssh_string_free_char(pHexa);

		nRet = ssh_write_knownhost(pSSH);
		if (nRet < 0) 
		{
			xRet = FTM_RET_SSH_FAILED_TO_SAVE_KEY;
			ERROR(xRet, "Failed to save key[%s]", strerror(errno));
		}
		break;

	case SSH_SERVER_ERROR:
		{
			ERROR(xRet, "Server error[%s]",ssh_get_error(pSSH));
			xRet = FTM_RET_SSH_SERVER_ERROR;
		}
		break;
	}

	ssh_clean_pubkey_hash(&pHash);

	return xRet;
}
