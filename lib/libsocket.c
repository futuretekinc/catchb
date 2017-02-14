#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <common.h>
#include "catchb_trace.h"
#include "libsocket.h"

int server_socket_create(int *sock ,char *file_path)
{
    struct sockaddr_un addr;

    *sock = socket(PF_FILE, SOCK_STREAM, 0);
    if (*sock == -1)
	{
        return -1;
	}

    if (access(file_path, F_OK) == 0)
	{
        unlink(file_path);
	}


    if( fcntl(*sock, F_SETFL, O_NONBLOCK) == -1 )
	{ /* Stratus VOS¿¡¼­´Â O_NDELAY */
        ERROR(-1, "listen socket nonblocking");
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, file_path, sizeof(addr.sun_path) - 1);
    if (bind(*sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        return -1;

    if (listen(*sock, BACKLOG) == -1)
        return -1;

    return 0;

}

