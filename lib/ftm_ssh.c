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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "ftm_ssh.h"
#include "ftm_mem.h"
#include "ftm_timer.h"
#include "ftm_trace.h"
#include <libssh/callbacks.h>

#undef	__MODULE__
#define	__MODULE__	"ssh"


static FTM_VOID_PTR	FTM_SSH_CHANNEL_threadMain
(
	FTM_VOID_PTR	pData
);

void FTM_SSH_LOG_callback
(
	int 	priority, 
	const char *function, 
	const char *buffer, 
	void *userdata
)
{
	INFO("%s - %s", function, buffer);
}

FTM_RET	FTM_SSH_create
(
	FTM_SSH_PTR	_PTR_ ppSSH
)
{
	ASSERT(ppSSH != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_SSH_PTR	pSSH;


//	ssh_set_log_level(100);
//	ssh_set_log_callback(FTM_SSH_LOG_callback);

	pSSH = (FTM_SSH_PTR)FTM_MEM_malloc(sizeof(FTM_SSH));
	if (pSSH == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create ssh!");
		goto finished;
	}

	pSSH->pSession = ssh_new();
	if (pSSH->pSession == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create ssh!");
		goto finished;
	}

	*ppSSH = pSSH;
	INFO("SSH created successfully.");

finished:
	
	if (xRet != FTM_RET_OK)
	{
		if (pSSH != NULL)
		{
			FTM_MEM_free(pSSH);	
		}
	}
	return	xRet;
}

FTM_RET	FTM_SSH_destroy
(
	FTM_SSH_PTR _PTR_ ppSSH
)
{
	ASSERT(ppSSH != NULL);

	ssh_free((*ppSSH)->pSession);

	FTM_MEM_free(*ppSSH);

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
	ASSERT(pSSH != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nRet;

	INFO("FTM_SSH_connect(%s, %s, %s)", ((pHost)?pHost:""), ((pUser)?pUser:""), ((pPasswd)?pPasswd:""));
	if(pUser != NULL)
	{
		INFO("SST OPTION USER : %s", pUser);
		nRet = ssh_options_set(pSSH->pSession, SSH_OPTIONS_USER, pUser);
		if (nRet < 0)
		{
			xRet = FTM_RET_SSH_FAILED_TO_SET_OPTION;
			ERROR(xRet, "Failed to set option!");
			return xRet;
		}
	}

	nRet = ssh_options_set(pSSH->pSession, SSH_OPTIONS_HOST, pHost);
	if (nRet < 0)
	{
		INFO("SST OPTION HOST : %s", pHost);
		xRet = FTM_RET_SSH_FAILED_TO_SET_OPTION;
		ERROR(xRet, "Failed to set option!");
		return xRet;
	}

	nRet = ssh_connect(pSSH->pSession);
	if (nRet < 0)
	{
		xRet = FTM_RET_SSH_CONNECTION_FAILED;
		ERROR(xRet, "Failed to connect[%s]",ssh_get_error(pSSH->pSession));
		return xRet;
	}

	xRet = FTM_SSH_verifyKnownhost(pSSH);
	if(xRet != FTM_RET_OK)
	{
		FTM_SSH_disconnect(pSSH);

		ERROR(xRet, "Failed to verify known host!");
		return xRet;
	}

	if (pPasswd != NULL)
	{
		INFO("SST AuthenticateConsole : %s", pPasswd);
		xRet = FTM_SSH_authenticateConsole(pSSH, pPasswd);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to authenticate console!");
			FTM_SSH_disconnect(pSSH);
			return	xRet;
		}
	}

	return	xRet;
}

FTM_RET	FTM_SSH_disconnect
(
	FTM_SSH_PTR	pSSH
)
{
	ASSERT(pSSH != NULL);

	ssh_disconnect(pSSH->pSession);

	return	FTM_RET_OK;
}

#if 1
FTM_RET	FTM_SSH_authenticateKbdint
(
	FTM_SSH_PTR	pSSH, 
	const FTM_CHAR_PTR	pPasswd	
) 
{
	FTM_RET	xRet;
	FTM_INT	nRet;

    nRet = ssh_userauth_kbdint(pSSH->pSession, NULL, NULL);
    while (nRet == SSH_AUTH_INFO) 
	{
        const FTM_CHAR _PTR_	pInstruction;
        const FTM_CHAR _PTR_	pName;
        FTM_CHAR	pBuffer[128];
        FTM_INT		i, n;

        pName = ssh_userauth_kbdint_getname(pSSH->pSession);
        if (pName && strlen(pName) > 0) 
		{
            INFO("SSH : Name - %s\n", pName);
        }

        pInstruction = ssh_userauth_kbdint_getinstruction(pSSH->pSession);
        if (pInstruction && strlen(pInstruction) > 0) 
		{
            INFO("SSH : Instruction - %s\n", pInstruction);
        }

        n = ssh_userauth_kbdint_getnprompts(pSSH->pSession);

        for (i = 0; i < n; i++) 
		{
            const FTM_CHAR _PTR_	pAnswer;
            const FTM_CHAR _PTR_	pPrompt;
            char echo;

            pPrompt = ssh_userauth_kbdint_getprompt(pSSH->pSession, i, &echo);
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

                if (ssh_userauth_kbdint_setanswer(pSSH->pSession, i, pBuffer) < 0) {
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

                nRet = ssh_userauth_kbdint_setanswer(pSSH->pSession, i, pAnswer);
                memset(pBuffer, 0, sizeof(pBuffer));
                if (nRet < 0) 
				{
					xRet = FTM_RET_SSH_AUTH_ERROR;
					ERROR(xRet, "Failed to auth!");
					return	xRet;
                }
            }
        }
        nRet = ssh_userauth_kbdint(pSSH->pSession,NULL,NULL);
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

	nRet = ssh_userauth_none(pSSH->pSession, NULL);
	if (nRet == SSH_AUTH_ERROR) 
	{
		xRet = FTM_RET_SSH_AUTH_ERROR;
		ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH->pSession));
		return xRet;
	}

	nMethod = ssh_userauth_list(pSSH->pSession, NULL);
	while (nRet != SSH_AUTH_SUCCESS) 
	{
		if (nMethod & SSH_AUTH_METHOD_GSSAPI_MIC)
		{
			INFO("User auth GSSAPI MIC!");
			nRet = ssh_userauth_gssapi(pSSH->pSession);
			if(nRet == SSH_AUTH_ERROR) 
			{
				xRet = FTM_RET_SSH_AUTH_ERROR;
				ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH->pSession));
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
			INFO("User auth publi key!");
			nRet = ssh_userauth_publickey_auto(pSSH->pSession, NULL, NULL);
			if (nRet == SSH_AUTH_ERROR) 
			{
				xRet = FTM_RET_SSH_AUTH_ERROR;
				ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH->pSession));
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
				ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH->pSession));
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
			INFO("User Auth method passwd!");
			nRet = ssh_userauth_password(pSSH->pSession, NULL, pPasswd);
			if (nRet == SSH_AUTH_ERROR) 
			{
				xRet = FTM_RET_SSH_AUTH_ERROR;
				ERROR(xRet, "Failed to authentication[%s]", ssh_get_error(pSSH->pSession));
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

	nRet = ssh_get_publickey(pSSH->pSession, &pServerPublicKey);
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

	nState = ssh_is_server_known(pSSH->pSession);
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

		nRet = ssh_write_knownhost(pSSH->pSession);
		if (nRet < 0) 
		{
			xRet = FTM_RET_SSH_FAILED_TO_SAVE_KEY;
			ERROR(xRet, "Failed to save key[%s]", strerror(errno));
		}
		break;

	case SSH_SERVER_ERROR:
		{
			ERROR(xRet, "Server error[%s]",ssh_get_error(pSSH->pSession));
			xRet = FTM_RET_SSH_SERVER_ERROR;
		}
		break;
	}

	ssh_clean_pubkey_hash(&pHash);

	return xRet;
}

FTM_RET	FTM_SSH_CHANNEL_create
(
	FTM_SSH_PTR	pSSH,
	FTM_SSH_CHANNEL_PTR _PTR_ ppChannel
)
{
	ASSERT(pSSH != NULL);
	ASSERT(ppChannel != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_SSH_CHANNEL_PTR	pChannel;

	pChannel = (FTM_SSH_CHANNEL_PTR)FTM_MEM_malloc(sizeof(FTM_SSH_CHANNEL));
	if (pChannel == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create ssh channel!");
		goto finished;
	}

	xRet = FTM_BUFFER_create(4096, &pChannel->pBuffer);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create buffer!");
		goto finished;
	}

	xRet = FTM_LOCK_create(&pChannel->pLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create lock!");
		goto finished;
	}

	pChannel->pChannel = ssh_channel_new(pSSH->pSession);
	if (pChannel->pChannel == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create ssh channel!");
		goto finished;
	}

	pChannel->pSSH = pSSH;

	*ppChannel = pChannel;

	INFO("SSH channel[%08x] created successfully.", pChannel->pChannel);
	
finished:
	if (xRet != FTM_RET_OK)
	{
		if (pChannel != NULL)
		{
			if (pChannel->pBuffer != NULL)
			{
				FTM_BUFFER_destroy(&pChannel->pBuffer);	
			}

			if (pChannel->pLock != NULL)
			{
				FTM_LOCK_destroy(&pChannel->pLock);	
			}

			FTM_MEM_free(pChannel);	
		}
	}

	return	xRet;

}


FTM_RET	FTM_SSH_CHANNEL_destroy
(
	FTM_SSH_CHANNEL_PTR _PTR_ ppChannel
)
{
	ASSERT(ppChannel != NULL);

	INFO("SSH channel[%08x] free.", (*ppChannel)->pChannel);
	ssh_channel_free((*ppChannel)->pChannel);
	INFO("SSH channel[%08x] free.", (*ppChannel)->pChannel);

	if ((*ppChannel)->pBuffer != NULL)
	{
		FTM_BUFFER_destroy(&(*ppChannel)->pBuffer);	
	}

	if ((*ppChannel)->pLock != NULL)
	{
		FTM_LOCK_destroy(&(*ppChannel)->pLock);	
	}


	FTM_MEM_free(*ppChannel);
	*ppChannel = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SSH_CHANNEL_open
(	
	FTM_SSH_CHANNEL_PTR	pChannel
)
{
	ASSERT(pChannel != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nRet;

	nRet = ssh_channel_open_session(pChannel->pChannel);
	if (nRet != SSH_OK)
	{
		xRet = FTM_RET_SSH_FAILED_TO_OPEN_CHANNEL;
		ERROR(xRet, "Failed to open channel!");
		return	xRet;
	}

	nRet = ssh_channel_request_exec(pChannel->pChannel, "vtysh");
	if (nRet != SSH_OK)
	{
		ssh_channel_close(pChannel->pChannel);
		pChannel->pChannel = 0;
		xRet = FTM_RET_SSH_FAILED_TO_OPEN_CHANNEL;
		ERROR(xRet, "Failed to open channel!");
		return	xRet;
	}

	if (pthread_create(&pChannel->xThread, NULL, FTM_SSH_CHANNEL_threadMain, pChannel) < 0)
	{
		ssh_channel_close(pChannel->pChannel);
		pChannel->pChannel = 0;

		xRet = FTM_RET_THREAD_CREATION_FAILED;
		ERROR(xRet, "Failed to create thread!");

		return	xRet;
	}


	INFO("SSH channel opened.");
	return	xRet;
}

FTM_RET	FTM_SSH_CHANNEL_close
(	
	FTM_SSH_CHANNEL_PTR	pChannel
)
{
	ASSERT(pChannel != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	pChannel->bStop = FTM_TRUE;
	if (pChannel->xThread != 0)
	{
		pthread_join(pChannel->xThread, NULL);
		pChannel->xThread = 0;
	}

	if (ssh_channel_is_open(pChannel->pChannel))
	{
		ssh_channel_send_eof(pChannel->pChannel);
		ssh_channel_close(pChannel->pChannel);
	}

	INFO("SSH channel closed.");
	return	xRet;
}

FTM_BOOL	FTM_SSH_CHANNEL_isOpen
(	
	FTM_SSH_CHANNEL_PTR	pChannel
)
{
	ASSERT(pChannel != NULL);

	return	ssh_channel_is_open(pChannel->pChannel);
}

FTM_BOOL	FTM_SSH_CHANNEL_isEOF
(
	FTM_SSH_CHANNEL_PTR	pChannel
)
{
 	return	 ssh_channel_is_eof(pChannel->pChannel);
}


FTM_RET	FTM_SSH_CHANNEL_read2
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_UINT32			ulTimeout,
	FTM_UINT8_PTR		pBuffer,
	FTM_UINT32			ulBufferLen,
	FTM_UINT32_PTR		pReadLen,
	FTM_UINT8_PTR		pErrorBuffer,
	FTM_UINT32			ulErrorBufferLen,
	FTM_UINT32_PTR		pErrorReadLen
)
{
	ASSERT(pChannel != NULL);
	ASSERT(pBuffer != NULL);
	ASSERT(pReadLen != NULL);

	FTM_RET	xRet = FTM_RET_TIMEOUT;
	FTM_INT	nReadLen =  0;
	FTM_TIMER	xTimer;

	FTM_TIMER_initMS(&xTimer, ulTimeout);

	*pReadLen = 0;
	*pErrorReadLen = 0;

	while(!FTM_TIMER_isExpired(&xTimer))
	{
		if (pBuffer != NULL)
		{
			nReadLen = ssh_channel_read_nonblocking(pChannel->pChannel, pBuffer, ulBufferLen, 0);
			if (nReadLen < 0)
			{
				xRet = FTM_RET_SSH_READ_FAILED;	
				ERROR(xRet, "Failed to read data!");
				break;
			}
			else if (nReadLen > 0)
			{
				*pReadLen = nReadLen;	
				xRet = FTM_RET_OK;
				break;
			}
		}

		if (pErrorBuffer != NULL)
		{
			nReadLen = ssh_channel_read_nonblocking(pChannel->pChannel, pErrorBuffer, ulErrorBufferLen, 1);
			if (nReadLen < 0)
			{
				xRet = FTM_RET_SSH_READ_FAILED;	
				ERROR(xRet, "Failed to read data!");
				break;
			}
			else if (nReadLen > 0)
			{
				*pErrorReadLen = nReadLen;	
				xRet = FTM_RET_OK;
				break;
			}
		}

		usleep(1000);
	}

	return	xRet;
}


FTM_RET	FTM_SSH_CHANNEL_write
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_UINT8_PTR		pBuffer,
	FTM_UINT32			ulBufferLen
)
{
	ASSERT(pChannel != NULL);
	ASSERT(pBuffer != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT32	nWrittenLen;

	INFO("SSH channel write (%s, %d)", pBuffer, ulBufferLen);

  	if (FTM_SSH_CHANNEL_isOpen(pChannel) && !FTM_SSH_CHANNEL_isEOF(pChannel))
	{
		fd_set	fds;
		FTM_INT	nFD = 0; 

		FD_ZERO(&fds);
		FD_SET(0,&fds);

		nFD = ssh_get_fd(pChannel->pSSH->pSession);
		if (nFD > 0) 
		{
			FD_SET(nFD, &fds);

			if(FD_ISSET(nFD,&fds))
			{
				INFO("ssh_channel_write(%x, %s, %d)", pChannel->pChannel, pBuffer, ulBufferLen);
				nWrittenLen = ssh_channel_write(pChannel->pChannel, pBuffer, ulBufferLen);
				if (nWrittenLen != ulBufferLen)
				{
					xRet = FTM_RET_SSH_WRITE_FAILED;	
					ERROR(xRet, "Failed to write data!");
				}
				INFO("ssh_channel_write done.");
			}
		}
		else
		{ 
			xRet = FTM_RET_ERROR;
			ERROR(xRet, "Failed to get FD.");
		}    
	}

	return	xRet;
}

FTM_RET	FTM_SSH_CHANNEL_writeLine
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_CHAR_PTR		pBuffer
)
{
	ASSERT(pChannel != NULL);
	ASSERT(pBuffer != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT32	nWrittenLen;

  	if (FTM_SSH_CHANNEL_isOpen(pChannel) && !FTM_SSH_CHANNEL_isEOF(pChannel))
	{
		fd_set	fds;
		FTM_INT	nFD = 0; 

		FD_ZERO(&fds);
		FD_SET(0,&fds);

		nFD = ssh_get_fd(pChannel->pSSH->pSession);
		if (nFD > 0) 
		{
			FD_SET(nFD, &fds);

			if(FD_ISSET(nFD,&fds))
			{
				FTM_CHAR	pInternalBuffer[1024];

				INFO("SSH Write : %s", pBuffer);
				sprintf(pInternalBuffer, "%s\n", pBuffer);
				nWrittenLen = ssh_channel_write(pChannel->pChannel, pInternalBuffer, strlen(pInternalBuffer));
				if (nWrittenLen != strlen(pInternalBuffer))
				{
					xRet = FTM_RET_SSH_WRITE_FAILED;	
					ERROR(xRet, "Failed to write data!");
				}

				INFO("ssh_channel_write done.");
			}
			else
			{
				ERROR(xRet, "Failed to write line");	
			}
		}
		else
		{ 
			xRet = FTM_RET_ERROR;
			ERROR(xRet, "Failed to get FD.");
		}    
	}
	else
	{
		ERROR(xRet, "Failed to write line");	
	}

	return	xRet;
}

FTM_RET	FTM_SSH_CHANNEL_readLine
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_CHAR_PTR		pBuffer,
	FTM_UINT32			ulBufferSize,
	FTM_UINT32_PTR		pReadLen
)
{
	ASSERT(pChannel != NULL);
	ASSERT(pBuffer != NULL);
	ASSERT(pReadLen != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	ulReadLen = 0;
	FTM_UINT32	ulRcvdLen = 0;
	FTM_UINT32	ulLen = 0;
	FTM_UINT8	nData;
	FTM_UINT32	i;
	FTM_BOOL	bFound = FTM_FALSE;

	if (pChannel->pChannel == NULL)
	{
		xRet = FTM_RET_SOCKET_CLOSED;
		goto finished;
	}

	FTM_BUFFER_getSize(pChannel->pBuffer, &ulRcvdLen);
	if (ulRcvdLen == 0)
	{
		goto finished;
	}

	for(i = 0 ; i < ulRcvdLen  && i < ulBufferSize ; i++)
	{
		xRet = FTM_BUFFER_getAt(pChannel->pBuffer, i, &nData);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get first data from buffer!");
			goto finished;	
		}

		if(nData == '\n')
		{
			bFound = FTM_TRUE;
			break;
		}
		else if(nData == 0x1b)
		{
			if ((ulRcvdLen - i) < 3)
			{
				break;
			}

			i+= 2;
		}
	}

	if (bFound )
	{
		FTM_LOCK_set(pChannel->pLock);

		for(i = 0 ; i < ulRcvdLen  && i < ulBufferSize ; i++)
		{
			xRet = FTM_BUFFER_getFirst(pChannel->pBuffer, &nData);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to get first data from buffer!");
				goto finished;	
			}

			if(nData == '\n')
			{
				FTM_BUFFER_popFront(pChannel->pBuffer, (FTM_UINT8_PTR)&nData, 1, &ulLen);
				break;	
			}
			else if (nData == '\r')
			{
				FTM_BUFFER_popFront(pChannel->pBuffer, (FTM_UINT8_PTR)&nData, 1, &ulLen);
			}
			else if(nData == 0x1b)
			{
				FTM_UINT8	pTemp[3];
				FTM_BUFFER_popFront(pChannel->pBuffer, pTemp, 3, &ulLen);
			}
			else
			{
				FTM_BUFFER_popFront(pChannel->pBuffer, (FTM_UINT8_PTR)&pBuffer[ulReadLen++], 1, &ulLen);
			}
		}

		FTM_LOCK_reset(pChannel->pLock);
	}
finished:

	*pReadLen = ulReadLen;

	if (ulReadLen != 0)
	{
		pBuffer[ulReadLen] = 0;
		INFO("SSL READ : %s", pBuffer);	
	}
	return	xRet;
}

FTM_RET	FTM_SSH_CHANNEL_read
(
	FTM_SSH_CHANNEL_PTR	pChannel,
	FTM_CHAR_PTR		pBuffer,
	FTM_UINT32			ulBufferSize,
	FTM_UINT32_PTR		pReadLen
)
{
	ASSERT(pChannel != NULL);
	ASSERT(pBuffer != NULL);
	ASSERT(pReadLen != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	ulReadLen = 0;
	FTM_UINT32	ulRcvdLen = 0;
	FTM_UINT32	ulLen = 0;
	FTM_UINT8	nData;
	FTM_UINT32	i;

	if (pChannel->pChannel == NULL)
	{
		xRet = FTM_RET_SOCKET_CLOSED;
		goto finished;
	}

	FTM_BUFFER_getSize(pChannel->pBuffer, &ulRcvdLen);
	if (ulRcvdLen == 0)
	{
		goto finished;
	}

	FTM_LOCK_set(pChannel->pLock);

	for(i = 0 ; i < ulRcvdLen  && i < ulBufferSize ; i++)
	{
		xRet = FTM_BUFFER_getFirst(pChannel->pBuffer, &nData);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get first data from buffer!");
			goto finished;	
		}

		if(nData == '\n')
		{
			FTM_BUFFER_popFront(pChannel->pBuffer, (FTM_UINT8_PTR)&nData, 1, &ulLen);
			break;	
		}
		else if (nData == '\r')
		{
			FTM_BUFFER_popFront(pChannel->pBuffer, (FTM_UINT8_PTR)&nData, 1, &ulLen);
		}
		else
		{
			FTM_BUFFER_popFront(pChannel->pBuffer, (FTM_UINT8_PTR)&pBuffer[ulReadLen++], 1, &ulLen);
		}
	}

	FTM_LOCK_reset(pChannel->pLock);
finished:

	*pReadLen = ulReadLen;

	if (ulReadLen != 0)
	{
		pBuffer[ulReadLen] = 0;
//		INFO("SSH READ : %s", pBuffer);	
	}
	return	xRet;
}

FTM_VOID_PTR	FTM_SSH_CHANNEL_threadMain
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_CHAR	pBuffer[1024];
	FTM_INT	nReadLen =  0;
	FTM_SSH_CHANNEL_PTR	pChannel = (FTM_SSH_CHANNEL_PTR)pData;

	pChannel->bStop = FTM_FALSE;

	/* loop while both connections are open */
	while (!pChannel->bStop)
	{
		nReadLen = ssh_channel_read_nonblocking(pChannel->pChannel, pBuffer, sizeof(pBuffer), 0);
#if 0
		if (nReadLen < 0)
		{
			ERROR(FTM_RET_SSH_READ_FAILED, "Failed to read data!");
			break;
		}
		else 
#endif
		if (nReadLen > 0)
		{
			pBuffer[nReadLen] = 0;
//			INFO("SSH RECEIVED[%d] : %s", nReadLen, pBuffer);
			FTM_LOCK_set(pChannel->pLock);
			for(FTM_INT i = 0 ; i < nReadLen ; i++)
			{
				if ((pBuffer[i] == 0x1B) && (i + 3 <= nReadLen))
				{
					i+= 2;
				}
				else
				{
					FTM_BUFFER_pushBack(pChannel->pBuffer, (FTM_UINT8_PTR)&pBuffer[i], 1);
				}
			}
			FTM_LOCK_reset(pChannel->pLock);
		}

		usleep(1000);
	}

	pChannel->bStop = FTM_TRUE;

	return 0;
}
