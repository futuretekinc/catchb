#ifndef	FTM_SSH_H_
#define	FTM_SSH_H_

#include "ftm_types.h"
#include <libssh/libssh.h>

typedef	ssh_session	FTM_SSH_PTR;

FTM_RET	FTM_SSH_create
(
	FTM_SSH_PTR	_PTR_ ppSSH
);

FTM_RET	FTM_SSH_destroy
(
	FTM_SSH_PTR _PTR_ ppSSH
);

FTM_RET	FTM_SSH_connect
(
	FTM_SSH_PTR	pSSH,
	const FTM_CHAR_PTR	pHost, 
	const FTM_CHAR_PTR	pUser,
	const FTM_CHAR_PTR	pPasswd
);

FTM_RET	FTM_SSH_disconnect
(
	FTM_SSH_PTR	pSSH
);

FTM_RET	FTM_SSH_authenticateConsole
(
	FTM_SSH_PTR	pSSH, 
	const FTM_CHAR_PTR	pPasswd	
);

FTM_RET	FTM_SSH_verifyKnownhost
(
	FTM_SSH_PTR pSSH
);

#endif
