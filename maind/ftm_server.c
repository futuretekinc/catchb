#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <common.h>
#include "ftm_trace.h"
#include "ftm_server.h"

FTM_RET	FTM_SERVER_create
(
	FTM_SERVER_PTR _PTR_ ppServer
)
{
	ASSERT(ppServer != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_SERVER_PTR	pServer;
	
	pServer = (FTM_SERVER_PTR)malloc(sizeof(FTM_SERVER));
	if (pServer == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create server!\n");
	}
	else
	{
		pServer->ulBacklog = 5;

		*ppServer = pServer;	
	}

	return	xRet;
}

FTM_RET	FTM_SERVER_destroy
(
	FTM_SERVER_PTR _PTR_ ppServer
)
{
	ASSERT(ppServer != NULL);
	ASSERT(*ppServer != NULL);

	free(*ppServer);
	*ppServer = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SERVER_open
(
	FTM_SERVER_PTR	pServer,
	FTM_CHAR_PTR	pFileName
)
{
	ASSERT(pServer != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_SOCKET	xSocket;
    struct sockaddr_un xAddr;

    xSocket = socket(PF_FILE, SOCK_STREAM, 0);
    if (xSocket == -1)
	{
		xRet = FTM_RET_SOCKET_CREATION_FAILED;
        ERROR(xRet, "Failed to create socket.");
        return xRet;
	}

    if (access(pFileName, F_OK) == 0)
	{
        unlink(pFileName);
	}


    if( fcntl(xSocket, F_SETFL, O_NONBLOCK) < 0)
	{ /* Stratus VOS¿¡¼­´Â O_NDELAY */
		xRet = FTM_RET_SOCKET_ATTRIBUTE_SET_FAILED;
        ERROR(xRet, "Failed to set nonblocking to socket.");
        return xRet;
    }

    memset(&xAddr, 0, sizeof(struct sockaddr_un));
    xAddr.sun_family = AF_UNIX;
    strncpy(xAddr.sun_path, pFileName, sizeof(xAddr.sun_path) - 1);

    if (bind(xSocket, (struct sockaddr *) &xAddr, sizeof(struct sockaddr_un)) == -1)
	{
		xRet = FTM_RET_SOCKET_BIND_FAILED;
		ERROR(xRet, "Failed to bind socket!");
        return xRet;
	}

    if (listen(xSocket, pServer->ulBacklog) == -1)
	{
		xRet = FTM_RET_SOCKET_LISTEN_FAILED;
		ERROR(xRet, "Failed to listen socket!");
        return xRet;
	}

	pServer->xSocket = xSocket;
	strncpy(pServer->pFileName, pFileName, sizeof(pServer->pFileName) - 1);

    return xRet;

}

FTM_RET	FTM_SERVER_close
(
	FTM_SERVER_PTR	pServer
)
{
	if (pServer->xSocket != 0)
	{
		close(pServer->xSocket);
		pServer->xSocket = 0;
	}

	return	FTM_RET_OK;
}
